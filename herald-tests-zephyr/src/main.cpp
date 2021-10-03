/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <kernel_structs.h>
// #include <sys/thread_stack.h> // Cannot be found in NCS v1.6.0
// #include <drivers/gpio.h>
// #include <drivers/hwinfo.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

// See https://github.com/catchorg/Catch2/blob/devel/docs/own-main.md#top
#define CATCH_CONFIG_RUNNER
#define SA_ONSTACK 0
#define CATCH_CONFIG_NO_POSIX_SIGNALS 
#define CATCH_CONFIG_NOSTDOUT
// #define CATCH_CONFIG_DISABLE_EXCEPTIONS
#include "../../herald-tests/catch.hpp"

#include <sstream>
#include <cstdio>

struct k_thread herald_thread;
constexpr int stackMaxSize = 
// #ifdef CONFIG_BT_MAX_CONN
// 	2048 + (CONFIG_BT_MAX_CONN * 512)
// 	// Was 12288 + (CONFIG_BT_MAX_CONN * 512), but this starved newlibc of HEAP (used in handling BLE connections/devices)
// #else
// 	9192
// #endif
// // Since v2.1 - MEMORY ARENA extra stack reservation - See herald/datatype/data.h
// #ifdef HERALD_MEMORYARENA_MAX
//   + HERALD_MEMORYARENA_MAX
// #else
//   + 8192
// #endif
//   + 4508 // Since v2.1 test for debug
//   // + 5120 // Since v2.1 AllocatableArray and removal of vector and map
// 	// Note +0 crashes at Herald entry, +1024 crashes at PAST DATE, +2048 crashes at creating sensor array
// 	// +3072 crashes at herald entry with Illegal load of EXC_RETURN into PC
// 	// +4096 crashes at BEFORE DATE          with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20006304
// 	// +5120 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20008ae0
// 	// +6144 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20008ee0
// 	// +7168 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x200092e0
// 	// +8192 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x200096e0
// 	// +9216 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20009ae0
// 	// +10240 crashes zephyr in main thread loop - likely due to memory exhaustion for the heap (over 96% SRAM used at this level)
// 	// Changed heap for nRF52832 to 1024, max conns to 4 from 2
// 	// +0 crashes BEFORE DATE with Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20006504
// 	// Changed heap for nRF52832 to 2048. max conns back to 2
// 	// +4096 crashes BEFORE DATE with Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x20006904
// 	// +5120 crashes at sensor array running with  Stacking error (context area might be not valid)... Data Access Violation... MMFAR Address: 0x200090e0

// // Additions for catch2
//   // + 16384

// ;
65536;
K_THREAD_STACK_DEFINE(herald_stack, stackMaxSize);

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
        LOG_ERR("Catch2: %s",log_strdup(str().c_str()));
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
    k_sleep(K_SECONDS(10));
  }
}

void herald_entry() {
  LOG_DBG("Catch thread entry");
  k_sleep(K_SECONDS(10));
  try {
    LOG_DBG("In try");
    k_sleep(K_SECONDS(10));
    Catch::Session session; 
    LOG_DBG("Session created. Calling run in 2 seconds");
    k_sleep(K_SECONDS(10));
    int result = session.run();
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
	int nameOk = k_thread_name_set(NULL,"main");
  // Wait for a few seconds for remote to connect
  LOG_DBG("Waiting for 10 seconds for tests to start (so we see thread analyser output)");
  k_sleep(K_SECONDS(10));

  // Now manually initialise catch
  LOG_DBG("Initialising catch");
  k_sleep(K_SECONDS(10));

  [[maybe_unused]]
	k_tid_t herald_pid = k_thread_create(&herald_thread, herald_stack, stackMaxSize,
			(k_thread_entry_t)herald_entry, NULL, NULL, NULL,
			-1, K_USER,
			K_SECONDS(2));
	nameOk = k_thread_name_set(herald_pid,"herald");
      
  LOG_DBG("Catch thread started");

	while (1) {
		k_sleep(K_SECONDS(2));

		LOG_DBG("main thread still running");
	}
}