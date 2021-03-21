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

#ifdef CC3XX_BACKEND
// Cryptocell - nRF52840/nRF9160/nRF53x only. See prj.conf too to enable this Hardware
#include <nrf_cc3xx_platform.h>
#include <nrf_cc3xx_platform_entropy.h>
#endif

#include <utility>

#include <kernel_structs.h>
#include <sys/thread_stack.h>
#include <drivers/gpio.h>
#include <drivers/hwinfo.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

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
K_THREAD_STACK_DEFINE(herald_stack, 9192); // TODO reduce this down

using namespace herald;
using namespace herald::data;
using namespace herald::payload;
using namespace herald::payload::fixed;

std::shared_ptr<SensorArray> sa;

char* str(const TargetIdentifier& ti) {
  return log_strdup( ((std::string)ti).c_str());
}

class AppLoggingDelegate : public herald::SensorDelegate {
public:
	AppLoggingDelegate() = default;
	~AppLoggingDelegate() = default;

	void sensor(SensorType sensor, const TargetIdentifier& didDetect) override {
		// LOG_DBG("sensor didDetect");
		LOG_DBG("sensor didDetect: %s", str(didDetect) ); // May want to disable this - logs A LOT of info
	}

  /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
  void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) override {
		// LOG_DBG("sensor didRead");
		LOG_DBG("sensor didRead: %s with payload: %s", str(fromTarget), log_strdup(didRead.hexEncodedString().c_str()));
	}

  /// Receive written immediate send data from target, e.g. important timing signal.
  void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) override {
		// LOG_DBG("sensor didReceive");
		LOG_DBG("sensor didReceive: %s with immediate send data: %s", str(fromTarget), log_strdup(didReceive.hexEncodedString().c_str()));
	}

  /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
  void sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget) override {
		LOG_DBG("sensor didShare");
		// LOG_DBG("sensor didShare: %s", str(fromTarget) );
		// for (auto& p : didShare) {
		// 	LOG_DBG(" - %s", log_strdup(p.hexEncodedString().c_str()));
		// }
	}

  /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) override {
		LOG_DBG("sensor didMeasure");
		// LOG_DBG("sensor didMeasure: %s with proximity: %d", str(fromTarget), didMeasure.value); 
	}

  /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
	template <typename LocationT>
  void sensor(SensorType sensor, const Location<LocationT>& didVisit) {
		LOG_DBG("sensor didVisit");
	}

  /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) override {
		LOG_DBG("sensor didMeasure withPayload");
	}

  /// Sensor state update
  void sensor(SensorType sensor, const SensorState& didUpdateState) override {
		LOG_DBG("sensor didUpdateState");
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
	LOG_DBG("Herald entry");
	k_sleep(K_MSEC(10000)); // pause so we have time to see Herald initialisation log messages. Don't do this in production!
	LOG_DBG("Herald setup begins");

	// Test date/time based things on Zephyr - interesting issues with compliance!
	Date now;
	LOG_DBG("BEFORE DATE");
	std::string s = now.iso8601DateTime();
	LOG_DBG("DATE: %s", log_strdup(s.c_str()));
	LOG_DBG("PAST DATE");

	std::shared_ptr<AppLoggingDelegate> appDelegate = std::make_shared<AppLoggingDelegate>();
	
	// IMPLEMENTORS GUIDANCE - USING HERALD
	// First initialise the Zephyr Context - this links Herald to any Zephyr OS specific constructs or callbacks
	std::shared_ptr<ZephyrContext> ctx = std::make_shared<ZephyrContext>();

  // Now prepare your device's Herald identity payload - this is what gets sent to other devices when they request it
	//   SECURITY: Depending on the payload provider, this could be static and in the clear or varying over time. 
	//             If static, it **could** be used to track a device - so only use the Fixed payload in testing.
	//             Consider the SecuredPayload or SimplePayload in all other circumstances.
  std::uint16_t countryCode = 826; // UK ISO 3166-1 numeric
	std::uint16_t stateCode = 0; // National default

	// TESTING ONLY
	// IF IN TESTING / DEBUG, USE A FIXED PAYLOAD (SO YOU CAN TRACK IT OVER TIME)
	std::uint64_t clientId = 1234567890; // TODO generate unique device ID from device hardware info (for static, test only, payload)
	std::uint8_t uniqueId[8];	
  // 7. Implement a consistent post restart valid ID from a hardware identifier (E.g. nRF serial number)
	auto hwInfoAvailable = hwinfo_get_device_id(uniqueId,sizeof(uniqueId));
	if (hwInfoAvailable > 0) {
		LOG_DBG("Read %d bytes for a unique, persistent, device ID", hwInfoAvailable);
		clientId = *uniqueId;
	} else {
		LOG_DBG("Couldn't read hardware info for zephyr device. Error code: %d", hwInfoAvailable);
	}
	LOG_DBG("Final clientID: %d", clientId);

	std::shared_ptr<ConcreteFixedPayloadDataSupplierV1> pds = std::make_shared<ConcreteFixedPayloadDataSupplierV1>(
		countryCode,
		stateCode,
		clientId
	);
	// END TESTING ONLY

	// PRODUCTION ONLY
// 	LOG_DBG("Before simple");
// 	k_sleep(K_SECONDS(2));
// 	// Use the simple payload, or secured payload, that implements privacy features to prevent user tracking
// 	herald::payload::simple::K k;
// 	// NOTE: You should store a secret key for a period of days and pass the value for the correct epoch in to here instead of sk
	
// 	// Note: Using the CC310 to do this. You can use RandomnessSource.h random sources instead if you wish, but CC310 is more secure.
// 	herald::payload::simple::SecretKey sk(std::byte(0x00),2048); // fallback - you should do something different.
	
// 	size_t buflen = 2048;
// 	uint8_t* buf = new uint8_t[buflen];
// 	size_t olen = 0;
	
// #ifdef CC3XX_BACKEND
// 	int success = nrf_cc3xx_platform_entropy_get(buf,buflen,&olen); 
// #else
// 	int success = 1;
// #endif
// 	if (0 == success) {
// 		sk.clear();
// 		sk.append(buf, 0, buflen);
// 		LOG_DBG("Have applied CC3xx generated data to secret key");
// 	} else {
// 		LOG_DBG("Could not generate 2048 bytes of randomness required for SimplePayload Secret Key. Falling back to fixed generic secret key.");
// 	}

// 	// verify secret key
// 	for (int i = 0;i < 2048;i+=64) {
// 		Data t = sk.subdata(i,64);
// 		LOG_DBG("Got 64 bytes from secret key from %d",i);
// 	}

// 	LOG_DBG("About to create Payload data supplier");
// 	k_sleep(K_SECONDS(2));

// 	std::shared_ptr<herald::payload::simple::ConcreteSimplePayloadDataSupplierV1> pds = std::make_shared<herald::payload::simple::ConcreteSimplePayloadDataSupplierV1>(
// 		ctx,
// 		countryCode,
// 		stateCode,
// 		sk,
// 		k
// 	);
	// END PRODUCTION ONLY
	LOG_DBG("Have created Payload data supplier");
	k_sleep(K_SECONDS(2));


	auto sink = ctx->getLoggingSink("mySub","myCat");
	sink->log(SensorLoggerLevel::debug,"Here's some info for you");
	auto payload = pds->payload(PayloadTimestamp(),nullptr);
	sink->log(SensorLoggerLevel::debug,"I've got some payload data");
	sink->log(SensorLoggerLevel::debug,payload->hexEncodedString());
	
	auto sink2 = ctx->getLoggingSink("mySub","mySecondCat");
	sink2->log(SensorLoggerLevel::debug,"Here's some more info for you");

	// LOGGING LEVEL TESTING
	LOG_DBG("Zephyr debug message");
	LOG_INF("Zephyr info message");
	LOG_ERR("Zephyr error message");
	sink2->log(SensorLoggerLevel::debug,"Herald debug message");
	sink2->log(SensorLoggerLevel::info,"Herald info message");
	sink2->log(SensorLoggerLevel::fault,"Herald error message");
	
	// Enable transmitter (i.e. this is a Herald device)
	BLESensorConfiguration::advertisingEnabled = true;

	
	LOG_DBG("Creating sensor array");
	k_sleep(K_SECONDS(2));
	
	// Create Herald sensor array - this handles both advertising (Transmitter) and scanning/connecting (Receiver)
	sa = std::make_shared<SensorArray>(ctx,pds);

	// Add contacts.log delegate
	// CURRENTLY BREAKS ZEPHYR - DON'T KNOW WHY YET - LOGGING SUBSYSTEM ISSUE
	// std::shared_ptr<ConcretePayloadDataFormatter> pdf = std::make_shared<ConcretePayloadDataFormatter>();
	// std::shared_ptr<ErrorStreamContactLogger> contactLogger = std::make_shared<ErrorStreamContactLogger>(ctx, pdf);
	// sa->add(contactLogger);
	
	// Note: You will likely want to register a SensorDelegate implementation of your own to the sensor array to get callbacks on nearby devices
	sa->add(appDelegate);


	// 3. Create and add a Logging sensor delegate to enable testing of discovery


	// 4. Now create a live analysis pipeline and enable RSSI to be sent to it for distance estimation
	herald::analysis::algorithms::distance::FowlerBasicAnalyser distanceAnalyser(0, -50, -24); // 0 = run every time run() is called

	herald::analysis::LoggingAnalysisDelegate<herald::datatype::Distance> myDelegate(ctx);
	herald::analysis::AnalysisDelegateManager adm(std::move(myDelegate)); // NOTE: myDelegate MOVED FROM and no longer accessible
	herald::analysis::AnalysisProviderManager apm(std::move(distanceAnalyser)); // NOTE: distanceAnalyser MOVED FROM and no longer accessible

	herald::analysis::AnalysisRunner<
		herald::analysis::AnalysisDelegateManager<herald::analysis::LoggingAnalysisDelegate<herald::datatype::Distance>>,
		herald::analysis::AnalysisProviderManager<herald::analysis::algorithms::distance::FowlerBasicAnalyser>,
		RSSI,Distance
	> runner(adm, apm); // just for Sample<RSSI> types, and their produced output (Sample<Distance>)

	std::shared_ptr<herald::analysis::SensorDelegateRSSISource<decltype(runner)>> src = std::make_shared<herald::analysis::SensorDelegateRSSISource<decltype(runner)>>(runner);
	sa->add(src);

	
	LOG_DBG("Starting sensor array");
	k_sleep(K_SECONDS(2));

	// Start array (and thus start advertising)
	sa->start(); // There's a corresponding stop() call too

	int iter = 0;
	Date last;
	int delay = 250; // KEEP THIS SMALL!!! This is how often we check to see if anything needs to happen over a connection.
	while (1) {
		k_sleep(K_MSEC(delay)); 
		Date now;
		if (iter > 40 /* && iter < 44 */ ) { // some delay to allow us to see advertising output
			// You could only do first 3 iterations so we can see the older log messages without continually scrolling through log messages
			sa->iteration(now - last);
		}
		
		if (0 == iter % (5000 / delay)) {
			LOG_DBG("herald thread still running. Iteration: %d", iter);
			runner.run(Date()); // Note: You may want to do this less or more regularly depending on your requirements
		}

		last = now;
		iter++;
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

	LOG_DBG("Logging test");
	LOG_DBG("Const char* param test: %s","some string param");
	LOG_DBG("int param test: %d",1234);

#ifdef CC3XX_BACKEND
	cc3xx_init();
#endif

	// Start herald entry on a new thread in case of errors, or needing to do something on the main thread
	k_tid_t herald_pid = k_thread_create(&herald_thread, herald_stack, 4096,
			(k_thread_entry_t)herald_entry, NULL, NULL, NULL,
			-1, K_USER,
			K_NO_WAIT);

  // herald_entry();
  // NOTE Above only works if CONFIG_MAIN_STACK_SIZE=2048 is set in prj.conf

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(2));
		gpio_pin_set(dev, PIN, (int)led_is_on);
		led_is_on = !led_is_on;

		LOG_DBG("main thread still running");

		// TODO Add logic here to detect failure in Herald thread, and restart to resume as necessary
	}
}
