/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#include "database/BleLinkedList.h"

#include <stdio.h>

/**
 * @brief Create a list entry 
 * 
 * @param address 
 * @param device 
 * @return BleLinkedListEntry_t* 
 */
static BleLinkedListEntry_t * prvCreateEntry(const BleAddress_t * address, BleDevice_t * device)
{
    /* Create a new entry */
    BleLinkedListEntry_t * entry = (BleLinkedListEntry_t*) malloc(sizeof(BleLinkedListEntry_t));

    /* Check success */
    if(entry == NULL)
    {
        printf("Error Creating entry\r\n");
        return NULL;
    }

    /* Copy the address */
    BleAddress_copy(&entry->deviceAddress, address);
    /* Set the device */
    entry->device = device;
    /* Set next to NULL */
    entry->next = NULL;

    return entry;
}

/**
 * @brief Destroy an entry
 * 
 * @param entry 
 */
static void prvDestroyEntry(BleLinkedListEntry_t * entry)
{
    free(entry);
    // TODO: We are not destroying the BleDevice, some sort of garbage collector is needed
    // It cannot happen here as there may be other references to it
}

/**
 * Add a device to the sorted linked list,
 * Adds a reference to the passed device,
 * make sure the memory of the device passed is not freeded
 * 
 * This is not a sorted list, it is VERY inefficient and should be upgraded
 * to something more efficeint like a binary tree
 * For initial test purposes a unsorted linked list is fine
 * 
 * \param list The list object
 * \param address the Key is the address
 * \param device Reference to the device, must not be destroyed
 * 
 * \return 0 if success nonzero otherwise
 */
int BleLinkedList_add(BleLinkedList_t * list, const BleAddress_t * address, 
    BleDevice_t * device)
{
    BleLinkedListEntry_t * cur;

    /* Create the entry */
    BleLinkedListEntry_t * newEntry = prvCreateEntry(address, device);

    /* Check success */
    if(newEntry == NULL)
    {
        return 1;
    }

    /* Check if we should add as head */
    if(list->head == NULL)
    {
        list->head = newEntry;
        list->size++;
        return 0;
    }

    /* Start at the head */
    cur = list->head;

    /* Check if the entry already exists */
    while(cur->next != NULL)
    {
        if(BleAddress_cmp(&cur->deviceAddress, &newEntry->deviceAddress)==0)
        {
            /* Change the devcie */
            free(cur->device);
            cur->device = newEntry->device;
            newEntry->device = NULL;
            prvDestroyEntry(newEntry);
            return 0;
        }

        /* Move to the next element */
        cur = cur->next;
    }

    /* Add the entry to the end of the list */
    cur->next = newEntry;

    list->size++;
    return 0;
}

/**
 * @brief Find a device by address in the linked list
 * 
 * @param list 
 * @param addr 
 * @return BleDevice_t* 
 */
BleDevice_t * BleLinkedList_find(BleLinkedList_t * list, const BleAddress_t * addr)
{
    BleLinkedListEntry_t * cur = list->head;

    while(cur != NULL)
    {
        if(BleAddress_cmp(&cur->deviceAddress, addr)==0)
        {
            /* Device found */
            return cur->device;
        }

        cur = cur->next;
    }

    /* The end of the list was reached */
    return NULL;
}