#include <stdlib.h>
#include <stdio.h>

#include "webauth.h"
#include "webauthtest.h"

#define BUFSIZE 4096
#define MAX_ATTRS 128

int main(int argc, char *argv[])
{
    WEBAUTH_KEY *key;
    WEBAUTH_KEYRING *ring;
    WEBAUTH_KEYRING *ring2;

    int s, len;
    unsigned char key_material[WA_AES_128];
    unsigned char hex[2048];
    time_t curr;
    TEST_VARS;

    START_TESTS(10);


    ring = webauth_keyring_new(32);
    TEST_OK(ring != NULL);

    s = webauth_random_key(key_material, WA_AES_128);
    TEST_OK2(WA_ERR_NONE, s);

    s=webauth_hex_encode(key_material, WA_AES_128, hex, &len, sizeof(hex));
    hex[len] = '\0';
    /*printf("key[%s]\n", hex);*/

    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    TEST_OK(key != NULL);

    time(&curr);
    s = webauth_keyring_add(ring, curr, curr, curr+3600, key);
    TEST_OK2(WA_ERR_NONE, s);

    s = webauth_random_key(key_material, WA_AES_128);
    TEST_OK2(WA_ERR_NONE, s);

    s = webauth_hex_encode(key_material, WA_AES_128, hex, &len, sizeof(hex));
    hex[len] = '\0';
    /*printf("key[%s]\n", hex);*/

    key = webauth_key_create(WA_AES_KEY, key_material, WA_AES_128);
    TEST_OK(key != NULL);

    s = webauth_keyring_add(ring, curr, curr+3600, curr+7200, key);
    TEST_OK2(WA_ERR_NONE, s);

    s = webauth_keyring_write_file(ring,"webauth_keyring");
    TEST_OK2(WA_ERR_NONE, s);

    s = webauth_keyring_read_file("webauth_keyring", &ring2);
    TEST_OK2(WA_ERR_NONE, s);

    /* FIXME: compare ring2 to ring */

    s = webauth_keyring_write_file(ring2,"webauth_keyring2");
    TEST_OK2(WA_ERR_NONE, s);

    /* FIXME: compare two test key ring files */

    webauth_keyring_free(ring);
    webauth_keyring_free(ring2);

    END_TESTS;
    exit(NUM_FAILED_TESTS ? 1 : 0);
}
