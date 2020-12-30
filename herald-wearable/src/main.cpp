/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../herald/herald.h"

#include <zephyr.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <string.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>

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

#include <logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

// Cryptocell - nRF52840/nRF9160/nRF53x only. See prj.conf too to enable this Hardware
#include <nrf_cc3xx_platform.h>
#include <nrf_cc3xx_platform_entropy.h>

void main(void)
{

	const struct device *dev = device_get_binding(
		CONFIG_UART_CONSOLE_ON_DEV_NAME);
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		return;
	}

	/* Poll if the DTR flag was set, optional */
	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
	}

	if (strlen(CONFIG_UART_CONSOLE_ON_DEV_NAME) !=
	    strlen("CDC_ACM_0") ||
	    strncmp(CONFIG_UART_CONSOLE_ON_DEV_NAME, "CDC_ACM_0",
		    strlen(CONFIG_UART_CONSOLE_ON_DEV_NAME))) {
		printk("Error: Console device name is not USB ACM\n");

		return;
	}

	LOG_INF("USB logging test");
	using namespace herald;
	using namespace herald::payload;
	using namespace herald::payload::fixed;

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
	std::shared_ptr<ConcreteFixedPayloadDataSupplierV1> pds = std::make_shared<ConcreteFixedPayloadDataSupplierV1>(
		countryCode,
		stateCode,
		clientId
	);
	
	// Create Herald sensor array - this handles both advertising (Transmitter) and scanning/connecting (Receiver)
	SensorArray sa(ctx,pds);

	// Note: You will likely want to register a SensorDelegate implementation of your own to the sensor array to get callbacks on nearby devices

	// Start array (and thus start advertising)
	sa.start(); // There's a corresponding stop() call too

	/* Implement notification. At the moment there is no suitable way
	 * of starting delayed work so we do it here
	 */
	while (1) {
		k_sleep(K_SECONDS(1)); // TODO reduce this, or use separate threads for certain activities

		// Periodic Herald tasks here
		// ctx->periodicActions();
	}
}
