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

#include <kernel_structs.h>
#include <sys/thread_stack.h>
#include <drivers/gpio.h>
#include <drivers/hwinfo.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

// See https://github.com/catchorg/Catch2/blob/devel/docs/own-main.md#top
#define CATCH_CONFIG_RUNNER
#define SA_ONSTACK 0
#define CATCH_CONFIG_NO_POSIX_SIGNALS 
#define CATCH_CONFIG_NOSTDOUT
#include "../../herald-tests/catch.hpp"

#include <sstream>
#include <cstdio>

struct k_thread herald_thread;
K_THREAD_STACK_DEFINE(herald_stack, 2*8192); // TODO reduce this down

// Catch normally uses std::cout but Zephyr doesn't use that - so we need this:-
// from https://github.com/catchorg/Catch2/issues/1290

class out_buff : public std::stringbuf {
    std::FILE* m_stream;
public:
    out_buff(std::FILE* stream):m_stream(stream) {}
    ~out_buff();
    int sync() override {
        int ret = 0;
        // for (unsigned char c : str()) {
        //     if (putc(c, m_stream) == EOF) {
        //         ret = -1;
        //         break;
        //     }
        // }
        LOG_ERR("Cacth2: %s",log_strdup(str().c_str()));
        // Reset the buffer to avoid printing it multiple times
        str("");
        return ret;
    }
};

out_buff::~out_buff() { pubsync(); }

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wexit-time-destructors" // static variables in cout/cerr/clog
#endif

namespace Catch {
    std::ostream& cout() {
        static std::ostream ret(new out_buff(stdout));
        return ret;
    }
    std::ostream& clog() {
        static std::ostream ret(new out_buff(stderr));
        return ret;
    }
    std::ostream& cerr() {
        return clog();
    }
}

// Fatal thread failure error handler in Zephyr
void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *esf) {
  LOG_DBG("Kernel fatal thread failure with reason: %d", reason);
  // Always return to allow main thread to proceed (Don't halt via k_fatal_halt like the default)
}

void _exit(int status)
{
  LOG_ERR("FAILURE: exit called with status %d", status);
  while (1) {
    ;
  }
}

void herald_entry() {
  LOG_DBG("Catch thread entry");
  try {
    int result = Catch::Session().run();
    LOG_DBG("Catch returned a value of: %d",result);
  } catch (const std::exception& e) {
    LOG_ERR("Tests reported problem: %s",log_strdup(e.what()));
  }

	while (1) {
		k_sleep(K_SECONDS(2));

		LOG_DBG("test thread still running");
	}
}

void main(void)
{
  // Wait for a few seconds for remote to connect
  LOG_DBG("Waiting for 5 seconds for tests to start");
  k_sleep(K_SECONDS(5));

  // Now manually initialise catch
  LOG_DBG("Initialising catch");
  k_sleep(K_SECONDS(2));

	k_tid_t herald_pid = k_thread_create(&herald_thread, herald_stack, 16384,
			(k_thread_entry_t)herald_entry, NULL, NULL, NULL,
			-1, K_USER,
			K_SECONDS(4));
      
  LOG_DBG("Catch thread started");

	while (1) {
		k_sleep(K_SECONDS(2));

		LOG_DBG("main thread still running");
	}
}