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

void main(void)
{
	using namespace herald;
	using namespace herald::payload;
	using namespace herald::payload::beacon;
	using namespace herald::payload::extended;

	// Create Herald sensor array
	std::shared_ptr<ZephyrContext> ctx = std::make_shared<ZephyrContext>();
	ConcreteExtendedDataV1 extendedData;
	extendedData.addSection(ExtendedDataSegmentCodesV1::TextPremises, erinsStakehouse.name);

	std::shared_ptr<payload::beacon::ConcreteBeaconPayloadDataSupplierV1> pds = std::make_shared<payload::beacon::ConcreteBeaconPayloadDataSupplierV1>(
		erinsStakehouse.country,
		erinsStakehouse.state,
		erinsStakehouse.code,
		extendedData
	);
	SensorArray sa(ctx,pds);
	// Start array (and thus start advertising)
	sa.start();

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

    // TODO periodic Herald tidy up tasks here
	}
}
