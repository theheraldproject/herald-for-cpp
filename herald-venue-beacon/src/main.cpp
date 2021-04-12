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
	using CT = Context<ZephyrContextProvider,ZephyrLoggingSink,BluetoothStateManager>;
	
	// Disable receiver / scanning mode - we're just transmitting our value
	BLESensorConfiguration config = ctx.getSensorConfiguration(); // copy ctor
	config.scanningEnabled = false;
	ctx.setSensorConfiguration(config);

	ConcreteExtendedDataV1 extendedData;
	extendedData.addSection(ExtendedDataSegmentCodesV1::TextPremises, erinsStakehouse.name);

	std::shared_ptr<payload::beacon::ConcreteBeaconPayloadDataSupplierV1> pds = std::make_shared<payload::beacon::ConcreteBeaconPayloadDataSupplierV1>(
		erinsStakehouse.country,
		erinsStakehouse.state,
		erinsStakehouse.code,
		extendedData
	);
	
	SensorArray sa(ctx,pds);
	ConcreteBLESensor<CT> ble(ctx,ctx.getBluetoothStateManager(),pds);
	sa.add(ble);

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
	}
}
