#include "ble/BleDevice.h"

#if HERALD_NOT_FOUND_EXPONENTIAL > 1
static int prv_ipow(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}
#endif


/**
 * \brief Record a herald not found, must be called on payload read error
 * 
 * Updates state and next read time
 * 
 * \param self 
 */
void BleDevice_payload_not_read(BleDevice_t * self, int8_t err)
{
    assert(self);

    uint32_t increment_time;

    if(self->nextRead != BleDevice_CONNECTION_TIME_MAX)
    {
        LOG_WRN("State is not CONNECTING!");
    }

    #if CONFIG_HERALD_NOT_FOUND_RESET_COUNT > 0
    if(self->herald_not_found > CONFIG_HERALD_NOT_FOUND_RESET_COUNT)
    {
        self->herald_not_found = 0;
    }
    #endif

    /* Increment herald not found */
    self->herald_not_found++;

    /* Only need to do the power it greater then 1 because x^1 = x */
    #if HERALD_NOT_FOUND_EXPONENTIAL > 1
    increment_time = prv_ipow(CONFIG_HERALD_NOT_FOUND_RETRY_S, HERALD_NOT_FOUND_EXPONENTIAL);
    #else
    increment_time = CONFIG_HERALD_NOT_FOUND_RETRY_S;
    #endif

    #if CONFIG_HERALD_NOT_FOUND_MULTIPLIER > 0
    increment_time *= self->herald_not_found * CONFIG_HERALD_NOT_FOUND_MULTIPLIER;
    #endif

    LOG_DBG("Next read: %u", increment_time);

    /* Update the next time to read */
    self->nextRead = Timestamp_now_s() + increment_time;
}