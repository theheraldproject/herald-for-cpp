/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_LINKED_LIST__
#define __BLE_LINKED_LIST__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ble/BleDevice.h"

/**
 * \defgroup BleLinkedList Linked list for BLE devices
 * This is a linked list that stores devices sorted by address
 * \{
 */

typedef struct ble_linked_list_entry BleLinkedListEntry_t;

#define BleLinkedList_DEF() {NULL, 0}

/**
 * A database entry
 * We will only store a pointer to the device
 * so we can have multiple addresses for the same device
 */
struct ble_linked_list_entry
{
    BleAddress_t deviceAddress;
    BleDevice_t * device;
    
    BleLinkedListEntry_t * next;
};

/**
 * The Linked list structure
 */
typedef struct ble_linked_list_s
{
    BleLinkedListEntry_t * head;
    size_t size;
}
BleLinkedList_t;

int BleLinkedList_add(BleLinkedList_t * list, const BleAddress_t * address, 
    BleDevice_t * device);

BleDevice_t * BleLinkedList_find(BleLinkedList_t * list, const BleAddress_t * addr);

/**
 * @brief Delete the entire list
 * 
 * Mainly used for test cases
 * 
 * @param self 
 */
static inline void BleLinkedList_delete_all(BleLinkedList_t * self)
{
    BleLinkedListEntry_t * curr = self->head;
    BleLinkedListEntry_t * next;
    while(curr != NULL)
    {
        next = curr->next;

        /* Delete the device */
        if(curr->device != NULL)
        {
            free(curr->device);
        }

        /* Delete the node */
        free(curr);

        curr = next;
    }

    self->size = 0;
}

static inline size_t BleLinkedList_get_size(BleLinkedList_t * self)
{
    return self->size;
}

static inline void BleLinkedList_print(BleLinkedList_t * list)
{
    BleLinkedListEntry_t * curr = list->head;

    while(curr != NULL)
    {
        printf("ADDR: " BleAddr_printStr() "\r\n", BleAddr_printParams(&curr->deviceAddress));
        curr = curr->next;
    }
}

/** \} */

#endif /* __BLE_LINKED_LIST__ */