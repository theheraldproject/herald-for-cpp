#include "ble/BleDevice.h"
#include "ble/BleErrCodes.h"

static uint32_t prv_ipow(uint32_t base, uint32_t exp)
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

// Exponential backoff. Can be calculated here http://exponentialbackoffcalculator.com/

#define prvHRLD_RST_CNT  CONFIG_HERALD_NOT_FOUND_EXP_BACKOFF_RESET_COUNT
#define prvHRLD_INTERVAL CONFIG_HERALD_NOT_FOUND_EXP_BACKOFF_INTERVAL_S
#define prvHRLD_RATE     CONFIG_HERALD_NOT_FOUND_EXP_BACKOFF_RATE

#define prvCON_RST_CNT  CONFIG_HERALD_CON_ERR_EXP_RESET_COUNT
#define prvCON_INTERVAL CONFIG_HERALD_CON_ERR_EXP_BACKOFF_INTERVAL_S
#define prvCON_RATE     CONFIG_HERALD_CON_ERR_EXP_BACKOFF_RATE

static uint32_t prv_connection_error(BleDevice_t * self)
{
    uint32_t increment_time;

    #if prvCON_RST_CNT > 0
    if(self->err_connecting > prvCON_RST_CNT)
    {
        self->err_connecting = 0;
    }
    #endif

    /* Calculate exponentail backoff */
    increment_time = prvCON_INTERVAL * prv_ipow(prvCON_RATE, self->err_connecting);

    /* Increment conenction error */
    self->err_connecting++;

    return increment_time;
}

static uint32_t prv_herald_not_found(BleDevice_t * self)
{
    uint32_t increment_time;

    #if prvHRLD_RST_CNT > 0
    if(self->herald_not_found > prvHRLD_RST_CNT)
    {
        self->herald_not_found = 0;
    }
    #endif

    /* Calculate exponentail backoff */
    increment_time = prvHRLD_INTERVAL * prv_ipow(prvHRLD_RATE, self->herald_not_found);

    /* Increment herald not found counter */
    self->herald_not_found++;

    return increment_time;
}

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

    switch(err)
    {
        case BleErr_SYSTEM:
            LOG_DBG("System error!");
            /* Try again the next time it is scanned */
            increment_time = 0;
            break;

        case BleErr_ERR_CONNECTING:
            LOG_DBG("Error connecting");
            /* Exponential backoff for connection error */
            increment_time = prv_connection_error(self);
            break;

        case BleErr_ERR_GATT_DISCOVERY:
            LOG_DBG("GATT Discovery could not complete!");
            /* Treat as connection error */
            increment_time = prv_connection_error(self);
            break;

        case BleErr_ERR_HERALD_SERVICE_NOT_FOUND:
            LOG_DBG("Herald service not found!");
            /* Exponential backoff, herald not found */
            increment_time = prv_herald_not_found(self);
            break;

        case BleErr_ERR_HERALD_PAYLOAD_NOT_FOUND:
            LOG_WRN("Herald payload not found!");
            /* Treat as a connection error, every herald service UUID
            should have herald payload service */
            increment_time = prv_connection_error(self);
            break;

        case BleErr_ERR_PAYLOAD_TO_BIG:
            LOG_WRN("Herald payload too big");
            /* Treat as herald not found,
            it is unlikely we are going to beable to read this */
            increment_time = prv_herald_not_found(self);
            break;
        default:
            LOG_ERR("Unknown error! (%d)", err);
            increment_time = 0;
            break;
    }

    LOG_DBG("Next connection in %us", increment_time);
    self->nextRead = Timestamp_now_s() + increment_time;
}