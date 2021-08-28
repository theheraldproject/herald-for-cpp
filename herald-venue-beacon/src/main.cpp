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

// #include <logging/log.h>
// LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);
// #define APP_DBG(_msg,...) LOG_DBG(_msg,##__VA_ARGS__);
// #define APP_INF(_msg,...) LOG_INF(_msg,##__VA_ARGS__);
// 	#define APP_ERR(_msg,...) LOG_ERR(_msg,##__VA_ARGS__);


// using namespace herald;
// using namespace herald::data;
// using namespace herald::payload;

// char* str(const TargetIdentifier& ti) {
//   return log_strdup( ((std::string)ti).c_str());
// }

struct DummyDelegate {};
// class AppLoggingDelegate {
// public:
// 	AppLoggingDelegate() = default;
// 	~AppLoggingDelegate() = default;

// 	void sensor(SensorType sensor, const TargetIdentifier& didDetect) {
// 		// LOG_DBG("sensor didDetect");
// 		APP_DBG("sensor didDetect: %s", str(didDetect) ); // May want to disable this - logs A LOT of info
// 	}

//   /// Read payload data from target, e.g. encrypted device identifier from BLE peripheral after successful connection.
//   void sensor(SensorType sensor, const PayloadData& didRead, const TargetIdentifier& fromTarget) {
// 		// LOG_DBG("sensor didRead");
// 		APP_DBG("sensor didRead: %s with payload: %s", str(fromTarget), log_strdup(didRead.hexEncodedString().c_str()));
// 	}

//   /// Receive written immediate send data from target, e.g. important timing signal.
//   void sensor(SensorType sensor, const ImmediateSendData& didReceive, const TargetIdentifier& fromTarget) {
// 		// LOG_DBG("sensor didReceive");
// 		APP_DBG("sensor didReceive: %s with immediate send data: %s", str(fromTarget), log_strdup(didReceive.hexEncodedString().c_str()));
// 	}

//   /// Read payload data of other targets recently acquired by a target, e.g. Android peripheral sharing payload data acquired from nearby iOS peripherals.
//   void sensor(SensorType sensor, const std::vector<PayloadData>& didShare, const TargetIdentifier& fromTarget) {
// 		APP_DBG("sensor didShare");
// 		// LOG_DBG("sensor didShare: %s", str(fromTarget) );
// 		// for (auto& p : didShare) {
// 		// 	LOG_DBG(" - %s", log_strdup(p.hexEncodedString().c_str()));
// 		// }
// 	}

//   /// Measure proximity to target, e.g. a sample of RSSI values from BLE peripheral.
//   void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget) {
// 		APP_DBG("sensor didMeasure");
// 		// LOG_DBG("sensor didMeasure: %s with proximity: %d", str(fromTarget), didMeasure.value); 
// 	}

//   /// Detection of time spent at location, e.g. at specific restaurant between 02/06/2020 19:00 and 02/06/2020 21:00
// 	template <typename LocationT>
//   void sensor(SensorType sensor, const Location<LocationT>& didVisit) {
// 		APP_DBG("sensor didVisit");
// 	}

//   /// Measure proximity to target with payload data. Combines didMeasure and didRead into a single convenient delegate method
//   void sensor(SensorType sensor, const Proximity& didMeasure, const TargetIdentifier& fromTarget, const PayloadData& withPayload) {
// 		APP_DBG("sensor didMeasure withPayload");
// 	}

//   /// Sensor state update
//   void sensor(SensorType sensor, const SensorState& didUpdateState) {
// 		APP_DBG("sensor didUpdateState");
// 	}
// };

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

void main(void)
{
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

	Date last;
	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

    // Periodic Herald tidy up tasks here
		Date now;
		sa.iteration(now - last);
		last = now;
		
		// APP_ERR("Memory pages free in Data Arena: %d", herald::datatype::Data::getArena().pagesFree());
	}
}
