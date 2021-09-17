/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../herald/herald.h"

#include <zephyr.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <string.h>
// #include <usb/usb_device.h>
// #include <drivers/uart.h>

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <sys/byteorder.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/services/nus.h>

#ifdef CC3XX_BACKEND
// Cryptocell - nRF52840/nRF9160/nRF53x only. See prj.conf too to enable this Hardware
#include <nrf_cc3xx_platform.h>
#include <nrf_cc3xx_platform_entropy.h>
#endif

#include <utility>

#include <kernel_structs.h>
// #include <sys/thread_stack.h>
#include <drivers/gpio.h>
#include <drivers/hwinfo.h>

#include <inttypes.h>

#include <logging/log.h>
// namespace applogging {
  LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);
	#define APP_DBG(_msg,...) LOG_DBG(_msg,##__VA_ARGS__);
	#define APP_INF(_msg,...) LOG_INF(_msg,##__VA_ARGS__);
	#define APP_ERR(_msg,...) LOG_ERR(_msg,##__VA_ARGS__);
// }

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif

struct k_thread herald_thread;
constexpr int stackMaxSize = 
#ifdef CONFIG_BT_MAX_CONN
	2048 + (CONFIG_BT_MAX_CONN * 512)
	// Was 12288 + (CONFIG_BT_MAX_CONN * 512), but this starved newlibc of HEAP (used in handling BLE connections/devices)
#else
	9192
#endif
// Since v2.1 - MEMORY ARENA extra stack reservation - See herald/datatype/data.h
#ifdef HERALD_MEMORYARENA_MAX
  + HERALD_MEMORYARENA_MAX
#else
  + 8192
#endif
  + 4508 // Since v2.1 test for debug
  // + 5120 // Since v2.1 AllocatableArray and removal of vector and map
	// Note +0 crashes at Herald entry, +1024 crashes at PAST DATE, +2048 crashes at creating sensor array
	// +3072 crashes at herald entry with Illegal load of EXC_RETURN into PC
	// +4096 crashes at BEFORE DATE          with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20006304
	// +5120 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20008ae0
	// +6144 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20008ee0
	// +7168 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x200092e0
	// +8192 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x200096e0
	// +9216 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20009ae0
	// +10240 crashes zephyr in main thread loop - likely due to memory exhaustion for the heap (over 96% SRAM used at this level)
	// Changed heap for nRF52832 to 1024, max conns to 4 from 2
	// +0 crashes BEFORE DATE with Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20006504
	// Changed heap for nRF52832 to 2048. max conns back to 2
	// +4096 crashes BEFORE DATE with Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20006904
	// +5120 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x200090e0
;
K_THREAD_STACK_DEFINE(herald_stack, 
	stackMaxSize
); // Was 9192 for nRF5340 (10 conns), 2048 for nRF52832 (3 conns)

using namespace herald;
using namespace herald::data;
using namespace herald::payload;
using namespace herald::payload::fixed;

char* str(const TargetIdentifier& ti) {
  return log_strdup( ((std::string)ti).c_str());
}

class AppLoggingDelegate {
public:
	AppLoggingDelegate() = default;
	~AppLoggingDelegate() = default;

	void sensor(SensorType sensor, const TargetIdentifier& didDetect) {
		// LOG_DBG("sensor didDetect");
		APP_DBG("sensor didDetect: %s", str(didDetect) ); // May want to disable this - logs A LOT of info
	}

  /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
  void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) {
		// LOG_DBG("sensor didRead");
		APP_DBG("sensor didRead: %s with payload: %s", str(fromTarget), log_strdup(didRead.hexEncodedString().c_str()));
	}

  /// Receive written immediate send data from target, e.g. important timing signal.
  void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) {
		// LOG_DBG("sensor didReceive");
		APP_DBG("sensor didReceive: %s with immediate send data: %s", str(fromTarget), log_strdup(didReceive.hexEncodedString().c_str()));
	}

  /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
  void sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget) {
		APP_DBG("sensor didShare");
		// LOG_DBG("sensor didShare: %s", str(fromTarget) );
		// for (auto& p : didShare) {
		// 	LOG_DBG(" - %s", log_strdup(p.hexEncodedString().c_str()));
		// }
	}

  /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) {
		APP_DBG("sensor didMeasure");
		// LOG_DBG("sensor didMeasure: %s with proximity: %d", str(fromTarget), didMeasure.value); 
	}

  /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
	template <typename LocationT>
  void sensor(SensorType sensor, const Location<LocationT>& didVisit) {
		APP_DBG("sensor didVisit");
	}

  /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) {
		APP_DBG("sensor didMeasure withPayload");
	}

  /// Sensor state update
  void sensor(SensorType sensor, const SensorState& didUpdateState) {
		APP_DBG("sensor didUpdateState");
	}
};

#ifdef CC3XX_BACKEND
void cc3xx_init() {
  // START IMPLEMENTORS GUIDANCE - EXAMPLE CODE NOT NEEDED TO COPY IN TO IN YOUR DEMO APP
  // NOTE TO IMPLEMENTORS: Please remember to use a hardware security module where present.
	//   This is especially important for Herald Secured payload use as it requires a secure TRNG.
	//   Note also that the nRF5 BLe hardware DOES NOT use cryptocell RNG, but an RNG on chip
	//   instead. This means the BLe Mac Address generated for nRF5, although a TRNG, is not as
	//   secure as the cryptocell and does not follow NIST guidance fully.
	//   The Herald project team recommends using the CC3xx cryptocell on nRFx for RNG where possible.
	//   In future Herald will fail compilation if it is being compiled for a platform with a
	//   cryptocell but without that functionality enabled in prj.conf.
	//   The below code is to show you how to use the cryptocell for TRNG when required.
	// Generate an example secure RNG
	size_t buflen = 16;
	uint8_t* buf = new uint8_t[buflen];
	size_t olen = 0;
	int success = nrf_cc3xx_platform_init();
	if (0 != success) {
		LOG_DBG("Could not initialise CC3xx cryptocell - Check prj.conf to ensure hardware is enabled");
	} else {
		success = nrf_cc3xx_platform_entropy_get(buf,buflen,&olen); // blocks, doesn't have its own pool
		if (0 != success) {
			LOG_DBG("Secure RNG failed");
		} else if (olen != buflen) {
			LOG_DBG("Didn't generate enough randomness for output");
		} else {
			LOG_DBG("nRF CC3xx cryptocell successfully initialised and tested");
			// Note: Your random number is in buf. Note the function will block until randomness is generated.
			//       For performance use an entropy pool and fill it out of sequence in another thread when it
			//       dips below your needs. (E.g. use 256 bit pool, and refill if its length is <= 32 bits).
		}
	}
	// END IMPLEMENTORS GUIDANCE
}
#endif

void herald_entry() {
	APP_DBG("Herald entry");
	k_sleep(K_MSEC(6000)); // pause so we have time to see Herald initialisation log messages. Don't do this in production!
	APP_DBG("Herald setup begins");

	// Test date/time based things on Zephyr - interesting issues with compliance!
	// Date now;
	// APP_DBG("BEFORE DATE");
	// std::string s = now.iso8601DateTime();
	// APP_DBG("DATE: %s", log_strdup(s.c_str()));
	// APP_DBG("PAST DATE");

	
	// IMPLEMENTORS GUIDANCE - USING HERALD
	// First initialise the Zephyr Context - this links Herald to any Zephyr OS specific constructs or callbacks
	ZephyrContextProvider zcp;
	Context ctx(zcp,zcp.getLoggingSink(),zcp.getBluetoothStateManager());
	// Note: The following shows the type of the above, but you don't need a reference to it in your app
	// using CT = Context<ZephyrContextProvider,ZephyrLoggingSink,BluetoothStateManager>;


	AppLoggingDelegate appDelegate;
	
	// 4. Now create a live analysis pipeline and enable RSSI to be sent to it for distance estimation
// #ifdef HERALD_ANALYSIS_ENABLED
	// herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(0, -50, -24); // 0 = run every time run() is called

	// herald::analysis::LoggingAnalysisDelegate<CT,herald::datatype::Distance> myDelegate(ctx);
	// herald::analysis::AnalysisDelegateManager adm(std::move(myDelegate)); // NOTE: myDelegate MOVED FROM and no longer accessible
	// herald::analysis::AnalysisProviderManager apm(std::move(distanceAnalyser)); // NOTE: distanceAnalyser MOVED FROM and no longer accessible

	// herald::analysis::AnalysisRunner<
	// 	herald::analysis::AnalysisDelegateManager<herald::analysis::LoggingAnalysisDelegate<CT,herald::datatype::Distance>>,
	// 	herald::analysis::AnalysisProviderManager<herald::analysis::algorithms::distance::FowlerBasicAnalyser>,
	// 	RSSI,Distance
	// > runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

	// herald::analysis::SensorDelegateRSSISource<decltype(runner)> src(runner);
	herald::ble::nordic_uart::NordicUartSensorDelegate nus(ctx);
	// sa.add(src);
// #endif

	SensorDelegateSet sensorDelegates(appDelegate/*, src*/ , nus); // just the one from the app, and one for the analysis API
	

  // Now prepare your device's Herald identity payload - this is what gets sent to other devices when they request it
	//   SECURITY: Depending on the payload provider, this could be static and in the clear or varying over time. 
	//             If static, it **could** be used to track a device - so only use the Fixed payload in testing.
	//             Consider the SecuredPayload or SimplePayload in all other circumstances.
  std::uint16_t countryCode = 826; // UK ISO 3166-1 numeric
	std::uint16_t stateCode = 0; // National default

	// TESTING ONLY
	// IF IN TESTING / DEBUG, USE A FIXED PAYLOAD (SO YOU CAN TRACK IT OVER TIME)
	std::uint64_t clientId = 1234567890; // TODO generate unique device ID from device hardware info (for static, test only, payload)
	// std::uint8_t uniqueId[8];	
  // // 7. Implement a consistent post restart valid ID from a hardware identifier (E.g. nRF serial number)
	// auto hwInfoAvailable = hwinfo_get_device_id(uniqueId,sizeof(uniqueId));
	// if (hwInfoAvailable > 0) {
	// 	APP_DBG("Read %d bytes for a unique, persistent, device ID", hwInfoAvailable);
	// 	clientId = *uniqueId;
	// } else {
	// 	APP_DBG("Couldn't read hardware info for zephyr device. Error code: %d", hwInfoAvailable);
	// }
	APP_DBG("Final clientID: %" PRIu64 "", clientId);

	ConcreteFixedPayloadDataSupplierV1 pds(
		countryCode,
		stateCode,
		clientId
	);
	// END TESTING ONLY

	// PRODUCTION ONLY
// 	APP_DBG("Before simple");
// 	k_sleep(K_SECONDS(2));
// 	// Use the simple payload, or secured payload, that implements privacy features to prevent user tracking
// 	herald::payload::simple::K k;
// 	// NOTE: You should store a secret key for a period of days and pass the value for the correct epoch in to here instead of sk
	
// 	APP_DBG("after simple key");
// 	k_sleep(K_SECONDS(2));
// 	// Note: Using the CC310 to do this. You can use RandomnessSource.h random sources instead if you wish, but CC310 is more secure.
// 	herald::payload::simple::SecretKey sk(std::byte(0x00),2048); // fallback - you should do something different.
	
// 	APP_DBG("after secret key");
// 	k_sleep(K_SECONDS(2));
// #ifdef CC3XX_BACKEND
// 	size_t buflen = 2048;
// 	uint8_t* buf = new uint8_t[buflen];
// 	size_t olen = 0;
// 	int success = nrf_cc3xx_platform_entropy_get(buf,buflen,&olen); 
// 	if (0 == success) {
// 		sk.clear();
// 		sk.append(buf, 0, buflen);
// 		APP_DBG("Have applied CC3xx generated data to secret key");
// 	} else {
// 		APP_DBG("Could not generate 2048 bytes of randomness required for SimplePayload Secret Key. Falling back to fixed generic secret key.");
// 	}
// 	delete buf;
// #endif

// 	APP_DBG("Before printing key");
// 	k_sleep(K_SECONDS(2));

// 	// verify secret key
// 	// for (int i = 0;i < 2048;i+=64) {
// 	// 	Data t = sk.subdata(i,64);
// 	// 	APP_DBG("Got 64 bytes from secret key from %d",i);
// 	// }

// 	APP_DBG("About to create Payload data supplier");
// 	k_sleep(K_SECONDS(2));

// 	herald::payload::simple::ConcreteSimplePayloadDataSupplierV1 pds(
// 		ctx,
// 		countryCode,
// 		stateCode,
// 		sk,
// 		k
// 	);
	// END PRODUCTION ONLY
	APP_DBG("Have created Payload data supplier");
	k_sleep(K_SECONDS(2));

	// auto& sink = ctx.getLoggingSink();
	// sink.log("subsys1","cat1",SensorLoggerLevel::debug,"Here's some info for you");
	// auto payload = pds.payload(PayloadTimestamp(),nullptr);
	// sink.log("subsys1","cat1",SensorLoggerLevel::debug,"I've got some payload data");
	// sink.log("subsys1","cat1",SensorLoggerLevel::debug,payload->hexEncodedString());

	// Basic logging type checks
	HLOGGERINLINE(ctx,"basesys1","cat2");
	HTDBG("A message with a number with {} value", 124);
	HTDBG("A message with a double with {} value", 456.789);
	HTDBG("A message with a double with {} and {} values", 124, 456.789);
	HTDBG("A message with a double with {} and '{}' and {} values", 124, "flibble", 456.789);
	// HTDBG("A message with a payload convertible to string with {1} value", payload); // TODO get this working automatically
	
	// auto& sink2 = ctx.getLoggingSink();
	// sink2.log("subsys2","cat2",SensorLoggerLevel::debug,"Here's some more info for you");

	// // LOGGING LEVEL TESTING
	// APP_DBG("Zephyr debug message");
	// APP_INF("Zephyr info message");
	// APP_ERR("Zephyr error message");
	// sink2.log("subsys2","cat2",SensorLoggerLevel::debug,"Herald debug message");
	// sink2.log("subsys2","cat2",SensorLoggerLevel::info,"Herald info message");
	// sink2.log("subsys2","cat2",SensorLoggerLevel::fault,"Herald error message");
	
	// Enable transmitter (i.e. this is a Herald device)
	BLESensorConfiguration config = ctx.getSensorConfiguration(); // copy ctor
	config.advertisingEnabled = true;
	config.scanningEnabled = true; // Enables scanning, payload fetching, etc. 
	// NOTE: ^ PROVIDES BLE COORD PROVIDER FOR THE HERALD PROTOCOL!!!
	ctx.setSensorConfiguration(config);

	
	APP_DBG("Creating sensor array");
	k_sleep(K_SECONDS(2));
	
	// Create Herald sensor array - this handles both advertising (Transmitter) and scanning/connecting (Receiver)
	ConcreteBLESensor ble(ctx, ctx.getBluetoothStateManager(), pds, sensorDelegates);
	SensorArray sa(ctx,pds,ble);
	// sa.add(ble);

	// Add contacts.log delegate
	// CURRENTLY BREAKS ZEPHYR - DON'T KNOW WHY YET - LOGGING SUBSYSTEM ISSUE
	// ConcretePayloadDataFormatter pdf;
	// ErrorStreamContactLogger contactLogger(ctx, pdf);
	// sa.add(contactLogger);
	
	// Note: You will likely want to register a SensorDelegate implementation of your own to the sensor array to get callbacks on nearby devices
	// sa.add(appDelegate);


	// 3. Create and add a Logging sensor delegate to enable testing of discovery

	// Enable Bluetooth explicitly now so we can initialise bt_nus_init
	zcp.enableBluetooth();

	APP_DBG("Initialising NUS service");
  k_sleep(K_SECONDS(5));
  bt_nus_init(NULL);
	APP_DBG("Post NUS init");

	
	APP_DBG("Starting sensor array");
	k_sleep(K_SECONDS(2));

	// Start array (and thus start advertising)
	sa.start(); // There's a corresponding stop() call too
	
	APP_DBG("Sensor array running!");
	k_sleep(K_SECONDS(2));


	int iter = 0;
	// APP_DBG("got iter!");
	// k_sleep(K_SECONDS(2));
	Date last;
	// APP_DBG("got last!");
	// k_sleep(K_SECONDS(2));
	int delay = 250; // KEEP THIS SMALL!!! This is how often we check to see if anything needs to happen over a connection.
	
	APP_DBG("Entering herald iteration loop");
	k_sleep(K_SECONDS(2));
	while (1) {
		k_sleep(K_MSEC(delay)); 
		Date now;
		if (iter > 40 /* && iter < 44 */ ) { // some delay to allow us to see advertising output
			// You could only do first 3 iterations so we can see the older log messages without continually scrolling through log messages
			APP_DBG("Calling Sensor Array iteration");
			// k_sleep(K_SECONDS(2));
			sa.iteration(now - last);
		}
		
		if (0 == iter % (5000 / delay)) {
			APP_DBG("herald thread still running. Iteration: %d", iter);
			// runner.run(Date()); // Note: You may want to do this less or more regularly depending on your requirements
			APP_ERR("Memory pages free in Data Arena: %d", herald::datatype::Data::getArena().pagesFree());
		}

		last = now;
		++iter;
	}
}

void main(void)
{
	const struct device *dev;
	bool led_is_on = true;
	int ret;

	dev = device_get_binding(LED0);
	if (dev == NULL) {
		return;
	}

	ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		return;
	}

	// APP_DBG("Logging test");
	// APP_DBG("Const char* param test: %s","some string param");
	// APP_DBG("int param test: %d",1234);

#ifdef CC3XX_BACKEND
	cc3xx_init();
#endif

	// Start herald entry on a new thread in case of errors, or needing to do something on the main thread
	[[maybe_unused]]
	k_tid_t herald_pid = k_thread_create(&herald_thread, herald_stack, stackMaxSize,
			(k_thread_entry_t)herald_entry, NULL, NULL, NULL,
			-1, K_USER,
			K_NO_WAIT);
	int nameOk = k_thread_name_set(herald_pid,"herald");
	nameOk = k_thread_name_set(NULL,"main");

  // herald_entry();
  // NOTE Above only works if CONFIG_MAIN_STACK_SIZE=2048 is set in prj.conf

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(2));
		gpio_pin_set(dev, PIN, (int)led_is_on);
		led_is_on = !led_is_on;

		APP_DBG("main thread still running");

		// TODO Add logic here to detect failure in Herald thread, and restart to resume as necessary
	}
}
