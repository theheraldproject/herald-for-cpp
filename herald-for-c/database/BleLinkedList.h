/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __BLE_LINKED_LIST_H__
#define __BLE_LINKED_LIST_H__

/* Method for statically defining a linked list */
#define BleLinkedList_DEF() {NULL, 0}

/**
 * A linked list entry, hiding the actual implementation
 */
struct BleLinkedListEntry_s;

/**
 * The Linked list structure
 */
typedef struct ble_linked_list_s
{
    struct BleLinkedListEntry_s * head;
    size_t size;
}
BleLinkedList_t;

/* Define the type of DB data structure as a Linked List */
typedef BleLinkedList_t BleDbDataStruct_t;

/* Make the static definition, ignore the max entries for now */
#define BleDbDataStruct_DEF(_max_num_entries) BleLinkedList_DEF()

#endif /* __BLE_LINKED_LIST_H__ */