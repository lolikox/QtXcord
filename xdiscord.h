/*
 * Copyright (C) 2021 by LolikoX
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef XD_H
#define XD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <curl/curl.h>

#include <stdbool.h>

typedef enum {
    XD_INIT_SUCCESS = 0,
    XD_INIT_CURL_FAILURE,
    XD_INIT_ERROR_FETCHING_GATEWAY,
} XD_InitError;

typedef struct {
    char TOKEN[60];
    bool connected;
    enum {
        XD_ACCOUNT_TOKENERROR = 1<<0,
        XD_ACCOUNT_CONNECTIONERROR = 1<1,
    } ERROR;
    struct XD_cookie_s {
        unsigned hash;
        char name[64];
        char value[128];
    } cookies[32];
} XD_Account;

// Setup a new account struct. DOES NOT CONNECT.
// Returns NULL when XD_MAX_ACCOUNTS is reached.
XD_Account* XD_setup_account(char* TOKEN);

// Connect an account, may set ERROR if connection fails.
void XD_connect_account(XD_Account* a);

// Disconnect account and remove it from the stack.
void XD_forget_account(XD_Account* a);

// 256 MAXIMUM ACCOUNTS, memory tracking
#define XD_MAX_ACCOUNTS 256
// Initialize xdiscord. Returns XD_InitError
XD_InitError XD_init();
// Call on module exit, memory cleanup
void XD_quit();

#ifdef __cplusplus
}
#endif

#endif // XD_H
