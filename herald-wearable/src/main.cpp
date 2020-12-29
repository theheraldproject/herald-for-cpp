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

void main(void)
{
	using namespace herald;
	using namespace herald::payload;
	using namespace herald::payload::fixed;
	

  std::uint16_t countryCode = 826; // UK ISO 3166-1 numeric
	std::uint16_t stateCode = 0; // National default
	std::uint64_t clientId = 1234567890; // TODO generate unique device ID from device hardware info (for static, test only, payload)

	// Create Herald sensor array
	std::shared_ptr<ZephyrContext> ctx = std::make_shared<ZephyrContext>();

	std::shared_ptr<payload::fixed::ConcreteFixedPayloadDataSupplierV1> pds = std::make_shared<payload::fixed::ConcreteFixedPayloadDataSupplierV1>(
		countryCode,
		stateCode,
		clientId
	);
	SensorArray sa(ctx,pds);
	// Start array (and thus start advertising)
	sa.start();

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1));

    // Periodic Herald tasks here
		ctx->periodicActions();
	}
}
