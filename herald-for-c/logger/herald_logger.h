/*
 * Copyright 2020-2021 Herald Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 * 
 */

#ifndef __HERALD_LOGGER_H__
#define __HERALD_LOGGER_H__

#include <stdio.h>

/* Logger overrides */
#define log_strdup(...) __VA_ARGS__
#define LOG_DBG(_frmt, ...) printf("DBG: " _frmt "\r\n", ## __VA_ARGS__)
#define LOG_INF(_frmt, ...) printf("INF: " _frmt "\r\n", ## __VA_ARGS__)
#define LOG_WRN(_frmt, ...) printf("WRN: " _frmt "\r\n", ## __VA_ARGS__)
#define LOG_ERR(_frmt, ...) printf("ERR: " _frmt "\r\n", ## __VA_ARGS__)

#endif /* __HERALD_LOGGER_H__ */