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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

#include "xdiscord.h"
#include "frozen.h"

#define XD_USERAGENT_VERSION "92.0.4515.159"
#define XD_USERAGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/" XD_USERAGENT_VERSION " Safari/537.36"

#define XD_API_SERVER "https://discord.com"
#define XD_API_VERSION "v9"
#define XD_CDN_SERVER "https://cdn.discord.com"

CURL* XD_curl;
XD_Account* XD_astack[XD_MAX_ACCOUNTS];
char XD_gateway_url[64];

void XD_cookies_to_string(XD_Account* a, char* str) {
    struct XD_cookie_s* c;
    char LINE[192];
    str[0] = '\0';
    for (int i=0; i<32; i++) {
        c = &a->cookies[i];
        if (c->hash) {
            sprintf(LINE, "%s=%s; ", c->name, c->value);
            strncat(str, LINE, 192);
        }
    }
}

struct CallbackStruct_s {
    XD_Account* a;
    void (*callback)(char* dest, size_t size, XD_Account* userp);
};

size_t _fetch_cb(char* dest, size_t size, size_t nmemb, void *userp) {
    size_t b = size * nmemb;
    /*{ // handle errors
        char message[64] = "";
        int code = -2;
        json_scanf(dest, size, "{ message:%Q, code:%d }", &message, &code);
        if (code != -2) {
            printf("m:%s\n\n",message);
            return b;
        }
    }*/
    struct CallbackStruct_s* s = userp;
    s->callback(dest, b, s->a);
    return b;
}

CURLcode XD_fetch_url(XD_Account* a, char* URL, char* postdata, void (*_cb)(char*, size_t, void*)) {
    struct curl_slist* headers = NULL;
    curl_slist_append(headers, "Accept: */*");
    curl_slist_append(headers, "User-Agent: " XD_USERAGENT);
    if (a) {
        curl_slist_append(headers, "User-Agent: " XD_USERAGENT);
        if (a->TOKEN[0]) {
            char str[74] = "Authorization: ";
            strncat(str, a->TOKEN, 74);
            headers = curl_slist_append(headers, str);
        }
    }
    if (postdata) {
        if (postdata[0] == '{') {
            headers = curl_slist_append(headers, "Content-Type: application/json");
        } else {
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        }
        curl_easy_setopt(XD_curl, CURLOPT_POSTFIELDS, postdata);
    }
    curl_easy_setopt(XD_curl, CURLOPT_COOKIEFILE, "");
    //curl_easy_setopt(XD_curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(XD_curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(XD_curl, CURLOPT_URL, URL);
    struct CallbackStruct_s* cbs = calloc(1, sizeof(struct CallbackStruct_s));
    cbs->callback = _cb;
    cbs->a = a;
    curl_easy_setopt(XD_curl, CURLOPT_WRITEDATA, cbs);
    curl_easy_setopt(XD_curl, CURLOPT_WRITEFUNCTION, _fetch_cb);
    CURLcode res = curl_easy_perform(XD_curl);
    curl_slist_free_all(headers);
    free(cbs);
    return res;
}

XD_Account* XD_setup_account(char* TOKEN) {
    XD_Account* new_account = calloc(sizeof(XD_Account), 1);
    // Check token
    if (strnlen(TOKEN, 60)!=59) {
        new_account->ERROR |= XD_ACCOUNT_TOKENERROR;
    }
    for (int i=0; i<59; i++) {
        if (i==24 || i==31) {
            if (TOKEN[i]!='.') new_account->ERROR |= XD_ACCOUNT_TOKENERROR;
        } else {
            if ( !(isalpha(TOKEN[i]) || isdigit(TOKEN[i]) || TOKEN[i]=='_' || TOKEN[i]=='-') )
                new_account->ERROR |= XD_ACCOUNT_TOKENERROR;
        }
    }
    // Set token
    strncpy(new_account->TOKEN, TOKEN, 60);

    new_account->TOKEN[59] = '\0';
    for (int i=0; i<XD_MAX_ACCOUNTS; i++) {
        if (i==XD_MAX_ACCOUNTS) {
            free(new_account);
            return NULL;
        }
        if (!XD_astack[i]) {
            XD_astack[i] = new_account;
            break;
        }
    }
    return new_account;
}

void XD_forget_account(XD_Account* a) {
    for (int i=0; i<XD_MAX_ACCOUNTS; i++) {
        if (XD_astack[i] == a) {
            memcpy(&XD_astack[i], &XD_astack[i+1], sizeof(XD_Account*) * (XD_MAX_ACCOUNTS - i));
            break;
        }
    }
    free(a);
}

void XD_connect_account(XD_Account* a) {
}

void _set_gateway_cb(char* dest, size_t size, void *userp) {
    json_scanf(dest, size, "{url: %s}", XD_gateway_url);
    char* s = strtok(&XD_gateway_url, "\""); if (s) s = '\0'; // hotfix
}

XD_InitError XD_init() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    XD_curl = curl_easy_init();
    if (!XD_curl) {
        return XD_INIT_CURL_FAILURE;
    }
    CURLcode res = XD_fetch_url(NULL, XD_API_SERVER "/api/" XD_API_VERSION "/gateway", NULL, _set_gateway_cb);
    if (res != CURLE_OK || XD_gateway_url[0] == '\0') {
        fputs(curl_easy_strerror(res), stderr);
        return XD_INIT_ERROR_FETCHING_GATEWAY;
    }
    puts(XD_gateway_url);

    return XD_INIT_SUCCESS;
}

void XD_quit() {
    while(XD_astack[0]) {
        // autism
        XD_forget_account(XD_astack[0]);
    }
    curl_global_cleanup();
}
