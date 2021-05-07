/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "CppUTest/TestHarness.h"

extern "C"
{
    #include "database/BleLinkedList.h"
}

#define NUM_TEST_DATA 12

TEST_GROUP(ble_linked_list)
{

    BleLinkedList_t list;

    BleAddress_t addrs[NUM_TEST_DATA];

    BleDevice_t * bleDev[NUM_TEST_DATA];

    BleAddress_t base_addr = {
        .val = {0xAA,1,2,0xFF,4,0xFF}
    };

    void setup()
    {
        list = BleLinkedList_DEF();

        /* Populate test data */
        for(size_t i=0; i<NUM_TEST_DATA; i++)
        {
            /* Populate test devices, torn down later */
            bleDev[i] = (BleDevice_t*) malloc(sizeof(BleDevice_t));

            for(int j=0; j<6; j++)
            {
                /* Create random addresses,
                randomness should be avoided in unit test but this is
                fine for now */
                addrs[i].val[j] = rand();
            }
        }
    }

    void teardown()
    {
        BleLinkedList_delete_all(&list);
    }
};

TEST(ble_linked_list, compare)
{
    BleAddress_t addr1 = BleAddress_DEF(0x33,0x44,0x55,0x66,0x77,0x88);
    BleAddress_t addr2;

    /* Start off with the same address */
    BleAddress_copy(&addr2, &addr1);

    /* Make sure the copy works */
    MEMCMP_EQUAL(&addr1, &addr2, sizeof(BleAddress_t));

    /* Test compare match */
    LONGS_EQUAL(0, BleAddress_cmp(&addr1, &addr2));

    /* Change MSB addr1 < addr2 */
    addr2.val[5]++;
    LONGS_EQUAL(-1, BleAddress_cmp(&addr1, &addr2));

    /* And the other way around */
    LONGS_EQUAL(1, BleAddress_cmp(&addr2, &addr1));

    /* Now change a middle byte, addr1 > addr2 */
    addr2.val[3]--;
    LONGS_EQUAL(1, BleAddress_cmp(&addr1, &addr2));

    /* Change the LSB, addr1 < addr2 */
    addr2.val[0]++;
    LONGS_EQUAL(-1, BleAddress_cmp(&addr1, &addr2));
}

TEST(ble_linked_list, add)
{
    BleDevice_t * retDev;

    /* Add the items */
    for(size_t i=0; i<6; i++)
    {
        /* Add an item */
        LONGS_EQUAL(0, BleLinkedList_add(&list, &addrs[i], bleDev[i]));
        /* Check the list size */
        LONGS_EQUAL(i+1, BleLinkedList_get_size(&list));
    }

    retDev = BleLinkedList_find(&list, &addrs[2]);

    POINTERS_EQUAL(bleDev[2], retDev);


    /* Attempt to find an non existing item */
    retDev = BleLinkedList_find(&list, &base_addr);
    POINTERS_EQUAL(NULL, retDev);
}

// TEST(ble_linked_list, add_double)
// {
//     Ble
// }