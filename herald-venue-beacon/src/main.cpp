/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../herald/herald.h"

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#include <kernel_structs.h>
// #include <sys/thread_stack.h>
#include <drivers/gpio.h>
#include <drivers/hwinfo.h>

#include <inttypes.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);
#define APP_DBG(_msg,...) LOG_DBG(_msg,##__VA_ARGS__);
#define APP_INF(_msg,...) LOG_INF(_msg,##__VA_ARGS__);
#define APP_ERR(_msg,...) LOG_ERR(_msg,##__VA_ARGS__);

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

void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *esf) {
	// LOG_PANIC();
	while (1) {
		// do nothing
	}
}

struct k_thread herald_thread;
constexpr int stackMaxSize = 
#ifdef CONFIG_BT_MAX_CONN
	2048 + (CONFIG_BT_MAX_CONN * 512)
	// Was 12288 + (CONFIG_BT_MAX_CONN * 512), but this starved newlibc of HEAP (used in handling BLE connections/devices)
#else
	9192
#endif
  // + 16000
	+ 9192 // nRF52840 Data memory arena size allocation reservation
;
K_THREAD_STACK_DEFINE(herald_stack, 
	stackMaxSize
); // Was 9192 for nRF5340 (10 conns), 2048 for nRF52832 (3 conns)


struct DummyDelegate {};

using MYUINT32 = unsigned long;

struct basic_venue {
	std::uint16_t country;
	std::uint16_t state;
	MYUINT32 code; // C++ linker may balk, confusing unsigned int with unsigned long
	std::string name;
};

static struct basic_venue joesPizza = {
	.country = 826,
	.state = 4,
	.code = 12345,
	.name = "Joe's Pizza"
};

static struct basic_venue adamsFishShop = {
	.country = 826,
	.state = 3,
	.code = 22334,
	.name = "Adam's Fish Shop"
};

static struct basic_venue maxsFineDining = {
	.country = 832,
	.state = 1,
	.code = 55566,
	.name = "Max's Fine Dining"
};

static struct basic_venue erinsStakehouse = {
	.country = 826,
	.state = 4,
	.code = 123123,
	.name = "Erin's Stakehouse"
};

// TODO replace the below with sub-venue extended data, with same venue code
static struct basic_venue adamsKitchen = {
  .country = 826,
	.state = 4,
	.code = 1234,
	.name = "Adam's Kitchen"
};
static struct basic_venue adamsOffice = {
  .country = 826,
	.state = 4,
	.code = 2345,
	.name = "Adam's Office"
};
static struct basic_venue adamsBedroom = {
  .country = 826,
	.state = 4,
	.code = 3456,
	.name = "Adam's Bedroom"
};
static struct basic_venue adamsLanding = {
  .country = 826,
	.state = 4,
	.code = 5678,
	.name = "Adam's Landing"
};
static struct basic_venue adamsPond = {
  .country = 826,
	.state = 4,
	.code = 6789,
	.name = "Adam's Pond"
};
static struct basic_venue adamsLounge = {
  .country = 826,
	.state = 4,
	.code = 7890,
	.name = "Adam's Lounge"
};

void herald_entry() {
	APP_DBG("Herald entry");
	k_sleep(K_MSEC(10000)); // pause so we have time to see Herald initialisation log messages. Don't do this in production!
	APP_DBG("Herald setup begins");

	using namespace herald;
	using namespace herald::payload;
	using namespace herald::payload::beacon;
	using namespace herald::payload::extended;

	// Create Herald sensor array
	ZephyrContextProvider zcp;
	Context ctx(zcp,zcp.getLoggingSink(),zcp.getBluetoothStateManager());
	// using CT = Context<ZephyrContextProvider,ZephyrLoggingSink,BluetoothStateManager>;
	
	// Disable receiver / scanning mode - we're just transmitting our value
	BLESensorConfiguration config = ctx.getSensorConfiguration(); // copy ctor
	config.scanningEnabled = false;
	// config.advertisingEnabled = true; // default
	ctx.setSensorConfiguration(config);

	ConcreteExtendedDataV1 extendedData;
	extendedData.addSection(ExtendedDataSegmentCodesV1::TextPremises, erinsStakehouse.name);

	payload::beacon::ConcreteBeaconPayloadDataSupplierV1 pds(
		erinsStakehouse.country,
		erinsStakehouse.state,
		erinsStakehouse.code,
		extendedData
	);

  // this is unusual, but required. Really we should log activity to serial BLE or similar
	DummyDelegate appDelegate;
	SensorDelegateSet sensorDelegates(appDelegate);
	
	ConcreteBLESensor ble(ctx, ctx.getBluetoothStateManager(), pds, sensorDelegates);
	SensorArray sa(ctx,pds,ble);

	// Start array (and thus start advertising)
	sa.start();

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

	APP_DBG("Logging test");

	// Start herald entry on a new thread in case of errors, or needing to do something on the main thread
	[[maybe_unused]]
	k_tid_t herald_pid = k_thread_create(&herald_thread, herald_stack, stackMaxSize,
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

		APP_DBG("main thread still running");

		// TODO Add logic here to detect failure in Herald thread, and restart to resume as necessary
	}
};
