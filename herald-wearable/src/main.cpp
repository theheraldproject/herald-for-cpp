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


// Cryptocell - nRF52840/nRF9160/nRF53x only. See prj.conf too to enable this Hardware
#include <nrf_cc3xx_platform.h>
#include <nrf_cc3xx_platform_entropy.h>

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
K_THREAD_STACK_DEFINE(herald_stack, 4096); // TODO reduce this down

using namespace herald;
using namespace herald::payload;
using namespace herald::payload::fixed;

std::shared_ptr<SensorArray> sa;

class AppLoggingDelegate : public herald::SensorDelegate {
public:
	AppLoggingDelegate() = default;
	~AppLoggingDelegate() = default;

	void sensor(SensorType sensor, const TargetIdentifier& didDetect) override {
		LOG_INF("sensor didDetect"); // May want to disable this - logs A LOT of info
	}

  /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
  void sensor(SensorType sensor, PayloadData didRead, const TargetIdentifier& fromTarget) override {
		LOG_INF("sensor didRead");
	}

  /// Receive written immediate send data from target, e.g. important timing signal.
  void sensor(SensorType sensor, ImmediateSendData didReceive, const TargetIdentifier& fromTarget) override {
		LOG_INF("sensor didReceive");
	}

  /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
  void sensor(SensorType sensor, std::vector<PayloadData> didShare, const TargetIdentifier& fromTarget) override {
		LOG_INF("sensor didShare");
	}

  /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
  void sensor(SensorType sensor, Proximity didMeasure, const TargetIdentifier& fromTarget) override {
		LOG_INF("sensor didMeasure");
	}

  /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
  void sensor(SensorType sensor, Location didVisit) override {
		LOG_INF("sensor didVisit");
	}

  /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
  void sensor(SensorType sensor, Proximity didMeasure, const TargetIdentifier& fromTarget, PayloadData withPayload) override {
		LOG_INF("sensor didMeasure withPayload");
	}

  /// Sensor state update
  void sensor(SensorType sensor, SensorState didUpdateState) override {
		LOG_INF("sensor didUpdateState");
	}
};

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
		LOG_INF("Could not initialise CC3xx cryptocell - Check prj.conf to ensure hardware is enabled");
	} else {
		success = nrf_cc3xx_platform_entropy_get(buf,buflen,&olen); // blocks, doesn't have its own pool
		if (0 != success) {
			LOG_INF("Secure RNG failed");
		} else if (olen != buflen) {
			LOG_INF("Didn't generate enough randomness for output");
		} else {
			LOG_INF("nRF CC3xx cryptocell successfully initialised and tested");
			// Note: Your random number is in buf. Note the function will block until randomness is generated.
			//       For performance use an entropy pool and fill it out of sequence in another thread when it
			//       dips below your needs. (E.g. use 256 bit pool, and refill if its length is <= 32 bits).
		}
	}
	// END IMPLEMENTORS GUIDANCE
}

void herald_entry() {
	k_sleep(K_MSEC(5000)); // pause so we have time to see Herald initialisation log messages. Don't do this in production!

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
	std::uint64_t clientId = 1234567890; // TODO generate unique device ID from device hardware info (for static, test only, payload)

	std::uint8_t uniqueId[8];
	auto hwInfoAvailable = hwinfo_get_device_id(uniqueId,sizeof(uniqueId));
	if (hwInfoAvailable > 0) {
		LOG_DBG("Read %d bytes for a unique, persistent, device ID", hwInfoAvailable);
		clientId = *uniqueId;
	} else {
		LOG_DBG("Couldn't read hardware info for zephyr device. Error code: %s", hwInfoAvailable);
	}
	LOG_DBG("Final clientID: %d", clientId);


  // 7. Implement a consistent post restart valid ID from a hardware identifier (E.g. nRF serial number)


	std::shared_ptr<ConcreteFixedPayloadDataSupplierV1> pds = std::make_shared<ConcreteFixedPayloadDataSupplierV1>(
		countryCode,
		stateCode,
		clientId
	);
	auto sink = ctx->getLoggingSink("mySub","myCat");
	sink->log(SensorLoggerLevel::info,"Here's some info for you");
	auto payload = pds->payload(PayloadTimestamp(),nullptr);
	sink->log(SensorLoggerLevel::info,"I've got some payload data");
	sink->log(SensorLoggerLevel::info,payload->hexEncodedString());
	
	auto sink2 = ctx->getLoggingSink("mySub","mySecondCat");
	sink2->log(SensorLoggerLevel::info,"Here's some more info for you");

	// auto bles = std::make_shared<ConcreteBLESensor>(ctx, ctx->getBluetoothStateManager(),
  //     pds);

	// sink->log(SensorLoggerLevel::debug,"Got concrete ble sensor");
	
	// BUG transmitter currently interferes with wearable receiver
	BLESensorConfiguration::advertisingEnabled = false;
	
	// Create Herald sensor array - this handles both advertising (Transmitter) and scanning/connecting (Receiver)
	sa = std::make_shared<SensorArray>(ctx,pds);
	
	// Note: You will likely want to register a SensorDelegate implementation of your own to the sensor array to get callbacks on nearby devices
	sa->add(appDelegate);


	// 3. Create and add a Logging sensor delegate to enable testing of discovery

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
			LOG_INF("herald thread still running. Iteration: %d", iter);
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

	// const struct device *dev = device_get_binding(
	// 	CONFIG_UART_CONSOLE_ON_DEV_NAME);
	// uint32_t dtr = 0;

	// if (usb_enable(NULL)) {
	// 	//return;
	// }

	// /* Poll if the DTR flag was set, optional */
	// while (!dtr) {
	// 	uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
	// }

	// if (strlen(CONFIG_UART_CONSOLE_ON_DEV_NAME) !=
	//     strlen("CDC_ACM_0") ||
	//     strncmp(CONFIG_UART_CONSOLE_ON_DEV_NAME, "CDC_ACM_0",
	// 	    strlen(CONFIG_UART_CONSOLE_ON_DEV_NAME))) {
	// 	printk("Error: Console device name is not USB ACM\n");

	// 	return;
	// }

	LOG_INF("Logging test");
	LOG_INF("Const char* param test: %s","some string param");
	LOG_INF("int param test: %d",1234);

	cc3xx_init();

	// Start herald entry on a new thread in case of errors, or needing to do something on the main thread
	k_tid_t herald_pid = k_thread_create(&herald_thread, herald_stack, 2048,
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

		LOG_INF("main thread still running");
	}
}
