/*
 * Test suite for libwebauth token manipulation functions.
 *
 * Written by Roland Schemers
 * Copyright 2002, 2003, 2006, 2009
 *     Board of Trustees, Leland Stanford Jr. University
 *
 * See LICENSE for licensing terms.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/webauth.h>
#include "webauthtest.h"

#define BUFSIZE 4096
#define MAX_ATTRS 128

int main(int argc, char *argv[])
{
    WEBAUTH_KEY *key;
    WEBAUTH_KEYRING *ring, *ring2;
    char key_material[WA_AES_128];
    WEBAUTH_ATTR_LIST *ain, *aout;
    int rlen, len, i, s;
    char *token;
    time_t curr;
    TEST_VARS;

    START_TESTS(81);

    time(&curr);
    ain = webauth_attr_list_new(32);
    webauth_attr_list_add_str(ain, WA_TK_TOKEN_TYPE, "id", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT_AUTH, "webkdc", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT, "krb5:schemers", 0,
                              WA_F_NONE);
    webauth_attr_list_add_time(ain, WA_TK_EXPIRATION_TIME, curr+3600,
                               WA_F_NONE);

    s = webauth_random_key(key_material, WA_AES_128);
    TEST_OK2(WA_ERR_NONE, s);

    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    TEST_OK(key != NULL);

    ring = webauth_keyring_new(32);
    TEST_OK(ring != NULL);
    
    time(&curr);
    s = webauth_keyring_add(ring, curr, curr, key);
    TEST_OK2(WA_ERR_NONE, s);

    webauth_key_free(key);

    rlen = webauth_token_encoded_length(ain);

    token = malloc(rlen+1);
    s = webauth_token_create(ain, 0, token, &len, rlen, ring);

    TEST_OK2(WA_ERR_NONE, s);
    TEST_OK2(len, rlen);

    /* now lets try and decode the token */
    aout = NULL;
    s = webauth_token_parse(token, len, 0, ring, &aout);
    TEST_OK2(s, WA_ERR_NONE);

    TEST_OK2(aout->num_attrs, ain->num_attrs);
    for (i=0; i < ain->num_attrs; i++) {
        TEST_OK(strcmp(aout->attrs[i].name, ain->attrs[i].name)==0);
        TEST_OK(aout->attrs[i].length == ain->attrs[i].length);
        TEST_OK(memcmp(aout->attrs[i].value, ain->attrs[i].value, 
                       ain->attrs[i].length)==0);
    }

    webauth_attr_list_free(aout);
    free(token);

    /* now lets encrypt a token in a key not on the ring and
       make sure it doesn't decrypt */

    s = webauth_random_key(key_material, WA_AES_128);
    TEST_OK2(WA_ERR_NONE, s);

    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    TEST_OK(key != NULL);

    ring2 = webauth_keyring_new(32);
    TEST_OK(ring2 != NULL);
    
    time(&curr);
    s = webauth_keyring_add(ring2, curr, curr, key);
    TEST_OK2(WA_ERR_NONE, s);

    webauth_key_free(key);

    rlen = webauth_token_encoded_length(ain);
    token = malloc(rlen+1);
    s = webauth_token_create(ain, 0, token, &len, rlen, ring2);
    TEST_OK2(WA_ERR_NONE, s);
    TEST_OK2(len, rlen);

    /* now lets try and decode the token with ring instead of ring2 */
    aout = NULL;
    s = webauth_token_parse(token, len, 0, ring, &aout);

    TEST_OK(s != WA_ERR_NONE);

    webauth_attr_list_free(ain);
    /*webauth_attr_list_free(aout);*/
    free(token);

    webauth_keyring_free(ring);
    webauth_keyring_free(ring2);

    /* now lets try the {create,parse}_with_key versions */

    ain = webauth_attr_list_new(32);
    webauth_attr_list_add_str(ain, WA_TK_TOKEN_TYPE, "id", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT_AUTH, "webkdc", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT, "krb5:schemers", 0, WA_F_NONE);
    webauth_attr_list_add_time(ain, WA_TK_EXPIRATION_TIME, curr+3600, WA_F_NONE);

    s = webauth_random_key(key_material, WA_AES_128);
    TEST_OK2(WA_ERR_NONE, s);

    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    TEST_OK(key != NULL);

    rlen = webauth_token_encoded_length(ain);

    token = malloc(rlen+1);
    s = webauth_token_create_with_key(ain, 0, token, &len, rlen, key);

    TEST_OK2(WA_ERR_NONE, s);
    TEST_OK2(len, rlen);

    /* now lets try and decode the token */
    aout = NULL;
    s = webauth_token_parse_with_key(token, len, 0, key, &aout);
    TEST_OK2(s, WA_ERR_NONE);

    TEST_OK2(aout->num_attrs, ain->num_attrs);
    for (i=0; i < ain->num_attrs; i++) {
        TEST_OK(strcmp(aout->attrs[i].name, ain->attrs[i].name)==0);
        TEST_OK(aout->attrs[i].length == ain->attrs[i].length);
        TEST_OK(memcmp(aout->attrs[i].value, ain->attrs[i].value, 
                       ain->attrs[i].length)==0);
    }

    webauth_attr_list_free(aout);
    webauth_attr_list_free(ain);
    free(token);
    webauth_key_free(key);


    /* lets try to parse expired token */

    ain = webauth_attr_list_new(32);
    webauth_attr_list_add_str(ain, WA_TK_TOKEN_TYPE, "id", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT_AUTH, "webkdc", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT, "krb5:schemers", 0, WA_F_NONE);
    webauth_attr_list_add_time(ain, WA_TK_EXPIRATION_TIME, curr-3600, WA_F_NONE);

    s = webauth_random_key(key_material, WA_AES_128);
    TEST_OK2(WA_ERR_NONE, s);

    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    TEST_OK(key != NULL);

    rlen = webauth_token_encoded_length(ain);

    token = malloc(rlen+1);
    s = webauth_token_create_with_key(ain, 0, token, &len, rlen, key);

    TEST_OK2(WA_ERR_NONE, s);
    TEST_OK2(len, rlen);

    /* now lets try and decode the token */
    aout = NULL;
    s = webauth_token_parse_with_key(token, len, 0, key, &aout);
    TEST_OK2(s, WA_ERR_TOKEN_EXPIRED);

    TEST_OK2(aout->num_attrs, ain->num_attrs);
    for (i=0; i < ain->num_attrs; i++) {
        TEST_OK(strcmp(aout->attrs[i].name, ain->attrs[i].name)==0);
        TEST_OK(aout->attrs[i].length == ain->attrs[i].length);
        TEST_OK(memcmp(aout->attrs[i].value, ain->attrs[i].value, 
                       ain->attrs[i].length)==0);
    }

    webauth_attr_list_free(aout);
    webauth_attr_list_free(ain);
    free(token);
    webauth_key_free(key);

    /* lets try to parse a stale token */

    ain = webauth_attr_list_new(32);
    webauth_attr_list_add_str(ain, WA_TK_TOKEN_TYPE, "id", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT_AUTH, "webkdc", 0, WA_F_NONE);
    webauth_attr_list_add_str(ain, WA_TK_SUBJECT, "krb5:schemers", 0, WA_F_NONE);
    webauth_attr_list_add_time(ain, WA_TK_CREATION_TIME, curr-3600, WA_F_NONE);

    s = webauth_random_key(key_material, WA_AES_128);
    TEST_OK2(WA_ERR_NONE, s);

    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    TEST_OK(key != NULL);

    rlen = webauth_token_encoded_length(ain);

    token = malloc(rlen+1);
    s = webauth_token_create_with_key(ain, 0, token, &len, rlen, key);

    TEST_OK2(WA_ERR_NONE, s);
    TEST_OK2(len, rlen);

    /* now lets try and decode the token */
    aout = NULL;
    s = webauth_token_parse_with_key(token, len, 300, key, &aout);
    TEST_OK2(s, WA_ERR_TOKEN_STALE);

    TEST_OK2(aout->num_attrs, ain->num_attrs);
    for (i=0; i < ain->num_attrs; i++) {
        TEST_OK(strcmp(aout->attrs[i].name, ain->attrs[i].name)==0);
        TEST_OK(aout->attrs[i].length == ain->attrs[i].length);
        TEST_OK(memcmp(aout->attrs[i].value, ain->attrs[i].value, 
                       ain->attrs[i].length)==0);
    }

    webauth_attr_list_free(aout);
    webauth_attr_list_free(ain);
    free(token);
    webauth_key_free(key);

    END_TESTS;
    exit(NUM_FAILED_TESTS ? 1 : 0);
}