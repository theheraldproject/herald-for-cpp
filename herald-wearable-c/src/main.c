/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include <zephyr.h>
#include "herald.h"
#include "payload/FixedPayloadSupplier.h"
#include "logger/herald_logger.h"
#include "abtt_payload.h"
#include <random/rand32.h>

static FixedPayloadSupplier_data_t fixedPayloadSupplier =
    FixedPayloadSupplier_INIT(
        826 // Country Code UK ISO 3166-1 numeric
        ,0 // State Code National default
        ,1234567890 // Client ID, needs to be set from random number;
    );

/* Initialize the payload supplier */
static PayloadSupplier_t payloadSupplier =
    FixedPayloadSupplier_PAYLOAD_SUPPLIER_DEF(&fixedPayloadSupplier);

void Main_didDetect(const BleAddress_t * pseudo)
{
    LOG_DBG("DidDetect: " BleAddr_printStr(), BleAddr_printParams(pseudo));
}

void Main_didRead(const BleAddress_t * pseudo, Data_t * payloadData)
{
    LOG_INF("DidRead: " BleAddr_printStr() " Data: ", BleAddr_printParams(pseudo));
    int i;
    for(i=0;i<payloadData->size;i++)
    {
        printk("%02X ", payloadData->data[i]);
    }
    printk("\r\n");

    if(payloadData->size < 1)
    {
        LOG_DBG("Emptry payload!");
        return;
    }

    switch(*payloadData->data)
    {
        case AbttPayload_HERALD_PROTOCOL_VERSION:
            LOG_DBG("Found ABTT payload: ");
            AbttPayload_parse(payloadData->data, payloadData->size);
            break;
        case FixedPayloadSupplierPAYLOAD_ID:
            LOG_DBG("Fixed payload detected!");
            PayloadSupplier_parse(&payloadSupplier, payloadData);
            break;
        default:
            LOG_WRN("Unknown payload!");
            break;
    }
}

void Main_didReceive(const BleAddress_t * pseudo, Data_t * immediateSendData)
{
    LOG_DBG("didReceive");
}

void Main_didShare(const BleAddress_t * pseudo, Data_t immediateSendData[], size_t dataLen)
{
    LOG_DBG("didShare");
}

void Main_didMeasure(const BleAddress_t * pseudo, double rssi)
{
    LOG_INF("didMeasure: " BleAddr_printStr() " RSSI: %d", BleAddr_printParams(pseudo), 
        (int8_t) rssi);
}

void Main_didVisit(int location)
{
    LOG_DBG("didVisit");
}

void Main_didUpdateState(SensorState_t sensorState)
{
    LOG_DBG("didUpdateState");
}

SensorDelegate_t Main_sensorDelegate =
    SensorDelegate_INIT(
        Main_didDetect,
        Main_didRead,
        Main_didReceive,
        Main_didShare,
        Main_didMeasure,
        Main_didVisit,
        Main_didUpdateState
    );

static BleDatabase_t ble_sensor_db = BleSensor_database_INIT(&Main_sensorDelegate);

#define prvSCAN_MSG_QUEUE_SIZE 256
#define prvPAYLOAD_PROCESS_MSG_QUEUE_SIZE 2
#define prvPAYLOAD_READ_MSG_QUEUE_SIZE 10

K_MSGQ_DEFINE(scan_queue, sizeof(struct scan_results_message), prvSCAN_MSG_QUEUE_SIZE, 4);
K_MSGQ_DEFINE(payload_process_queue, sizeof(struct payload_msg), prvPAYLOAD_PROCESS_MSG_QUEUE_SIZE, 4);
K_MSGQ_DEFINE(payload_read_queue, sizeof(struct payload_req_msg), prvPAYLOAD_READ_MSG_QUEUE_SIZE, 4);


static BleSensor_t ble_sensor = BleSensor_DEF(&ble_sensor_db, &scan_queue, &payload_read_queue, &payload_process_queue);

static void prv_process_scans_task(void * p1, void * p2, void * p3)
{
    while(1)
    {
        BleSensor_process_scan(&ble_sensor);
    }
}

static void prv_read_payloads_task(void * p1, void * p2, void * p3)
{
    while(1)
    {
        BleSensor_read_payloads(&ble_sensor);
    }
}

static void prv_process_payloads_task(void * p1, void * p2, void * p3)
{
    while(1)
    {
        BleSensor_process_payload(&ble_sensor);
    }
}

#define prvSCAN_PROCESS_TASK_STACK_SIZE 1024
#define prvSCAN_PROCESS_TASK_PRIORITY 5

#define prvPAYLOAD_PROCESS_TASK_STACK_SIZE 1024
#define prvPAYLOAD_PROCESS_TASK_PRIORITY 5

#define prvPAYLOAD_READ_TASK_STACK_SIZE 1024
#define prvPAYLOAD_READ_TASK_PRIORITY 5

/* The scan processing task, initialized and started here */
K_THREAD_DEFINE(scan_task_tid, prvSCAN_PROCESS_TASK_STACK_SIZE,
                prv_process_scans_task, NULL, NULL, NULL,
                prvSCAN_PROCESS_TASK_PRIORITY, 0, 0);

/* The payload processing task, initialized and started here */
K_THREAD_DEFINE(payload_process_task_tid, prvPAYLOAD_PROCESS_TASK_STACK_SIZE,
                prv_process_payloads_task, NULL, NULL, NULL,
                prvPAYLOAD_PROCESS_TASK_PRIORITY, 0, 0);

/* The payload processing task, initialized and started here */
K_THREAD_DEFINE(payload_read_task_tid, prvPAYLOAD_READ_TASK_STACK_SIZE,
                prv_read_payloads_task, NULL, NULL, NULL,
                prvPAYLOAD_READ_TASK_PRIORITY, 0, 0);

void prv_db_clean_handler(struct k_work *work)
{
    BleDatabase_remove_old_devices(&ble_sensor_db);
}

K_WORK_DEFINE(clean_db_work, prv_db_clean_handler);

void clean_db_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&clean_db_work);
}

K_TIMER_DEFINE(clean_db_timer, clean_db_timer_handler, NULL);


uint8_t payload_mem_data[128];

Data_t payload_mem =
{
    .size = sizeof(payload_mem_data) / sizeof(uint8_t),
    .data = payload_mem_data
};

void prv_update_payload_handler(struct k_work * work)
{
    uint64_t cid;
    int sz;

    /* Get cryptographically secure random number */
    sys_csrand_get(&cid, sizeof(uint64_t));
    //sys_rand_get(&cid, sizeof(uint64_t));
    //cid = rand() * rand();
    
    /* Set new CID */
    FixedPayloadSupplier_setClientId(&fixedPayloadSupplier, cid);
    /* Create the payload, it does not depend on timestamp */
    sz = PayloadSupplier_createPayload(&payloadSupplier, 0, &payload_mem);
    /* Error check */
    if(sz <= 0)
    {
        LOG_ERR("Create payload!");
        return;
    }

    /* Initialize the payload data structure */
    Data_t payload = 
    {
        .data = payload_mem.data,
        .size = sz
    };

    BleSensor_update_payload(&ble_sensor, &payload);

    LOG_DBG("Updated payload");
}

K_WORK_DEFINE(update_payload_work, prv_update_payload_handler);

void update_payload_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&update_payload_work);
}

K_TIMER_DEFINE(update_payload_timer, update_payload_timer_handler, NULL);


int main(void)
{
    int err;

    /* Start the DB clean timer */
    k_timer_start(&clean_db_timer, K_SECONDS(CONFIG_HERALD_DEVICE_EXPIRY_SEC),
        K_SECONDS(CONFIG_HERALD_DEVICE_EXPIRY_SEC));

    /* Start the update payload timer */
    k_timer_start(&clean_db_timer, K_SECONDS(60), K_SECONDS(60));

    err = BleSensor_init(&ble_sensor);

    if(err)
    {
        LOG_ERR("Init sensor!");
        return 1;
    }

    prv_update_payload_handler(&update_payload_work);

    err = BleSensor_start(&ble_sensor);

    if(err)
    {
        LOG_ERR("Start sensor!");
        return 1;
    }

    while(1)
    {
        LOG_DBG("Main thread");
        k_sleep(K_SECONDS(10));
    }
    
    return 0;
}