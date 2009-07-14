/*
 * Interface to the libwebauth utility library.
 *
 * The libwebauth utility library contains the basic token handling functions
 * used by all other parts of the webauth code.  It contains functions to
 * encode and decode lists of attributes, generate tokens from them, encode
 * and decode tokens in base64 or hex encoding, and some additional utility
 * functions to generate random numbers or new AES keys.
 *
 * Written by Roland Schemers
 * Copyright 2002, 2003, 2008, 2009
 *     Board of Trustees, Leland Stanford Jr. University
 *
 * See LICENSE for licensing terms.
 */

#ifndef _WEBAUTH_H
#define _WEBAUTH_H 1

#include <time.h>

#if defined(_WIN32)

# include <WinSock2.h>
# include <windows.h>
# include <io.h>

typedef unsigned int uint32_t;
typedef int int32_t;

# ifndef WEBAUTH_DLL_BUILD
#  define WEBAUTH_API __declspec(dllimport)
# else
#  define WEBAUTH_API
# endif

#else

/* The necessary preprocessor magic to get int32_t and uint32_t. */
# include <inttypes.h>
# include <stdint.h>

# define WEBAUTH_API

#endif

/*
 * BEGIN_DECLS is used at the beginning of declarations so that C++
 * compilers don't mangle their names.  END_DECLS is used at the end.
 */
#undef BEGIN_DECLS
#undef END_DECLS
#ifdef __cplusplus
# define BEGIN_DECLS    extern "C" {
# define END_DECLS      }
#else
# define BEGIN_DECLS    /* empty */
# define END_DECLS      /* empty */
#endif

BEGIN_DECLS

/*
 * ERROR AND STATUS CODES
 */

/*
 * libwebauth error codes.
 *
 * Many libwebauth functions return an error status, or 0 on success.  For
 * those functions, the error codes are chosen from the following enum.
 *
 * Use webauth_error_message(code) to get the corresponding error message.
 */
typedef enum {
    WA_ERR_NONE = 0,         /* No error occured. */
    WA_ERR_NO_ROOM,          /* Supplied buffer too small. */
    WA_ERR_CORRUPT,          /* Data is incorrectly formatted. */
    WA_ERR_NO_MEM,           /* No memory. */
    WA_ERR_BAD_HMAC,         /* HMAC check failed. */
    WA_ERR_RAND_FAILURE,     /* Unable to get random data. */
    WA_ERR_BAD_KEY,          /* Unable to use key. */
    WA_ERR_KEYRING_OPENWRITE,/* Unable to open key ring for writing. */
    WA_ERR_KEYRING_WRITE,    /* Unable to write to key ring. */
    WA_ERR_KEYRING_OPENREAD, /* Unable to open key ring for reading. */
    WA_ERR_KEYRING_READ,     /* Unable to read key ring file. */
    WA_ERR_KEYRING_VERSION,  /* Bad keyring version. */
    WA_ERR_NOT_FOUND,        /* Item not found while searching. */
    WA_ERR_KRB5,             /* A Kerberos5 error occured. */
    WA_ERR_INVALID_CONTEXT,  /* Invalid context passed to function. */
    WA_ERR_LOGIN_FAILED,     /* Bad username/password. */
    WA_ERR_TOKEN_EXPIRED,    /* Token has expired. */
    WA_ERR_TOKEN_STALE,      /* Token is stale. */

    /* Update webauth_error_message when adding more codes. */

} WEBAUTH_ERR;

/*
 * Protocol error codes (PEC) for error-token and XML messages.  These numbers
 * must not change, as they are part of the protocol
 */
typedef enum {
    WA_PEC_SERVICE_TOKEN_EXPIRED       =  1,
    WA_PEC_SERVICE_TOKEN_INVALID       =  2, /* Can't decrypt / bad format */
    WA_PEC_PROXY_TOKEN_EXPIRED         =  3,
    WA_PEC_PROXY_TOKEN_INVALID         =  4, /* Can't decrypt / bad format */
    WA_PEC_INVALID_REQUEST             =  5, /* Missing/incorrect data, etc */
    WA_PEC_UNAUTHORIZED                =  6, /* Access denied */
    WA_PEC_SERVER_FAILURE              =  7, /* Server failure, try again */
    WA_PEC_REQUEST_TOKEN_STALE         =  8,
    WA_PEC_REQUEST_TOKEN_INVALID       =  9, /* Can't decrypt / bad format */
    WA_PEC_GET_CRED_FAILURE            = 10, /* Can't get credential */
    WA_PEC_REQUESTER_KRB5_CRED_INVALID = 11, /* <requesterCredential> was bad */
    WA_PEC_LOGIN_TOKEN_STALE           = 12,
    WA_PEC_LOGIN_TOKEN_INVALID         = 13, /* Can't decrypt / bad format */
    WA_PEC_LOGIN_FAILED                = 14, /* Username/passwword failed */
    WA_PEC_PROXY_TOKEN_REQUIRED        = 15, /* Missing required proxy-token */
    WA_PEC_LOGIN_CANCELED              = 16, /* User cancelled login */
    WA_PEC_LOGIN_FORCED                = 17, /* User must re-login */
    WA_PEC_USER_REJECTED               = 18, /* Principal not permitted */
} WEBAUTH_ET_ERR;

/*
 * Status for webauth_keyring_auto_update, indicating whether the keyring was
 * newly created, updated, or left alone.
 */
typedef enum {
    WA_KAU_NONE = 0,
    WA_KAU_CREATE,
    WA_KAU_UPDATE
} WEBAUTH_KAU_STATUS;


/*
 * PROTOCOL CONSTANTS
 */

/* Token constants. */
#define WA_TK_APP_STATE            "as"
#define WA_TK_COMMAND              "cmd"
#define WA_TK_CRED_DATA            "crd"
#define WA_TK_CRED_TYPE            "crt"
#define WA_TK_CRED_SERVER          "crs"
#define WA_TK_CREATION_TIME        "ct"
#define WA_TK_ERROR_CODE           "ec"
#define WA_TK_ERROR_MESSAGE        "em"
#define WA_TK_EXPIRATION_TIME      "et"
#define WA_TK_SESSION_KEY          "k"
#define WA_TK_LASTUSED_TIME        "lt"
#define WA_TK_PASSWORD             "p"
#define WA_TK_PROXY_TYPE           "pt"
#define WA_TK_PROXY_DATA           "pd"
#define WA_TK_PROXY_SUBJECT        "ps"
#define WA_TK_REQUEST_OPTIONS      "ro"
#define WA_TK_REQUESTED_TOKEN_TYPE "rtt"
#define WA_TK_RETURN_URL           "ru"
#define WA_TK_SUBJECT              "s"
#define WA_TK_SUBJECT_AUTH         "sa"
#define WA_TK_SUBJECT_AUTH_DATA    "sad"
#define WA_TK_TOKEN_TYPE           "t"
#define WA_TK_USERNAME             "u"
#define WA_TK_WEBKDC_TOKEN         "wt"

/* Token type constants. */
#define WA_TT_WEBKDC_SERVICE "webkdc-service"
#define WA_TT_WEBKDC_PROXY   "webkdc-proxy"
#define WA_TT_REQUEST        "req"
#define WA_TT_ERROR          "error"
#define WA_TT_ID             "id"
#define WA_TT_PROXY          "proxy"
#define WA_TT_CRED           "cred"
#define WA_TT_APP            "app"
#define WA_TT_LOGIN          "login"

/* Subject auth type constants. */
#define WA_SA_KRB5           "krb5"
#define WA_SA_WEBKDC         "webkdc"


/*
 * API CONSTANTS
 */

/* Supported key types. */
#define WA_AES_KEY 1

/* Supported AES key sizes. */
#define WA_AES_128 16
#define WA_AES_192 24
#define WA_AES_256 32

/* Flags to webauth_attr_list_add functions. */
#define WA_F_NONE       0x00
#define WA_F_COPY_VALUE 0x01
#define WA_F_COPY_NAME  0x02
#define WA_F_FMT_STR    0x04
#define WA_F_FMT_B64    0x08
#define WA_F_FMT_HEX    0x10
#define WA_F_COPY_BOTH  (WA_F_COPY_NAME | WA_F_COPY_VALUE)

/* Flags for webauth_krb5_get_principal. */
enum webauth_krb5_canon {
    WA_KRB5_CANON_NONE  = 0,    /* Do not canonicalize principals. */
    WA_KRB5_CANON_LOCAL = 1,    /* Strip the local realm. */
    WA_KRB5_CANON_STRIP         /* Strip any realm. */
};


/*
 * TYPES
 */

/*
 * Holds a generic name/value attribute for constructing and parsing tokens.
 * Names must not contain "=", and values may contain binary data, since the
 * length must be specified.
 */
typedef struct {
    const char *name;           /* Name of attribute. */
    int flags;                  /* flags passed in during add */
    void *value;                /* Value of attribute (binary data). */
    int length;                 /* Length of attribute value in bytes. */
    char val_buff[32];          /* Temp buffer to avoid malloc on encoding. */
} WEBAUTH_ATTR;

/*
 * Holds a list of attributes.  You must always use use webauth_attr_list_new
 * to construct a new attr list so that webauth_attr_list_{add,free} work
 * correctly.
 */
typedef struct {
    int num_attrs;
    int capacity;
    WEBAUTH_ATTR *attrs;
} WEBAUTH_ATTR_LIST;

/* A crypto key for encryption or decryption. */
typedef struct {
    int type;
    char *data;
    int length;
} WEBAUTH_KEY;

/* An entry in a keyring, holding a WEBAUTH_KEY with timestamps. */
typedef struct {
    time_t creation_time;
    time_t valid_after;
    WEBAUTH_KEY *key;
} WEBAUTH_KEYRING_ENTRY;

/* A keyring, holding encryption keys.  Can be serialized to disk. */
typedef struct {
    int num_entries;
    int capacity;
    WEBAUTH_KEYRING_ENTRY *entries;
} WEBAUTH_KEYRING;

/* A WebAuth Kerberos context for Kerberos support functions. */
typedef struct webauth_krb5_ctxt WEBAUTH_KRB5_CTXT;


/*
 * INFORMATIONAL FUNCTIONS
 */

/*
 * Returns the error message for the specified error code or "unknown error
 * code" if there is none.
 */
WEBAUTH_API const char *webauth_error_message(int errcode);

/* Returns the package name and version number, separated by a space. */
WEBAUTH_API const char *webauth_info_version(void);

/*
 * Returns a string describing the package build.
 *
 * Currently, this string contains the user and host on which the package was
 * built and the UTC timestamp of when it was configured.
 */
WEBAUTH_API const char *webauth_info_build(void);


/*
 * BASE64 ENCODING AND DECODING
 */

/*
 * Returns the amount of space required to base64-encode data.  Returned
 * length does NOT include room for nul-termination.
 */
WEBAUTH_API int webauth_base64_encoded_length(int length);

/*
 * Amount of space required to base64-decode data.
 *
 * Returns the amount of space required to base64-decode data of the given
 * length.  Does not actually attempt to ensure that the input contains a
 * valid base64-encoded string, other than checking the last two characters
 * for padding ("=").  Returned length does NOT include room for
 * nul-termination.
 *
 * Returns WA_ERR_NONE on success, or WA_ERR_CORRUPT if length is not greater
 * than 0 and a multiple of 4, since the input data cannot be valid
 * base64-encoded data.
 */
WEBAUTH_API int webauth_base64_decoded_length(const char *input,
                                              int length,
                                              int *decoded_length);

/*
 * Base64-encode the given data.
 *
 * Does NOT nul-terminate.  Output cannot point to the same memory space as
 * input.
 *
 * Returns WA_ERR_NONE on success, or WA_ERR_NO_ROOM if encoding the provided
 * data would require more space than max_output_len.
 */
WEBAUTH_API int webauth_base64_encode(const char *input, int input_len,
                                      char *output, int *output_len,
                                      int max_output_len);

/*
 * Base64-decode the given data.
 *
 * Does NOT nul-terminate.  Output may point to input.
 *
 * Returns WA_ERR_NONE on success, WA_ERR_NO_ROOM if decoding the provided
 * data would require more space than max_output_len, or WA_ERR_CORRUPT if
 * input is not valid base64-encoded data.
 */
WEBAUTH_API int webauth_base64_decode(char *input, int input_len,
                                      char *output, int *output_len,
                                      int max_output_len);


/*
 * HEX ENCODING AND DECODING
 */

/*
 * Returns the amount of space required to hex encode data of the given
 * length.  Returned length does NOT include room for a null-termination.
 */
WEBAUTH_API int webauth_hex_encoded_length(int length);

/*
 * Returns the amount of space required to decode the hex encoded data of the
 * given length.  Returned length does NOT include room for a
 * null-termination.
 *
 * Returns WA_ERR_NONE on succes, or WA_ERR_CORRUPT if length is not greater
 * then 0 and a multiple of 2.
 */
WEBAUTH_API int webauth_hex_decoded_length(int length, int *out_length);

/*
 * Hex-encodes the given data.  Does NOT null-terminate.  output can point to
 * input as long as max_output_len is long enough.
 *
 * Returns WA_ERR_NONE or WA_ERR_NO_ROOM.
 */
WEBAUTH_API int webauth_hex_encode(char *input, int input_len,
                                   char *output, int *output_len,
                                   int max_output_len);

/*
 * Hex-decodes the given data.  Does NOT null-terminate.  output can point to
 * input.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_ROOM, or WA_ERR_CORRUPT.
 */
WEBAUTH_API int webauth_hex_decode(char *input, int input_len,
                                   char *output, int *output_length,
                                   int max_output_len);


/*
 * ATTRIBUTE MANIPULATION
 */

/* Creates a new attribute list, returning it or NULL if no memory. */
WEBAUTH_API WEBAUTH_ATTR_LIST *webauth_attr_list_new(int initial_capacity);

/*
 * Adds an attribute to the attribute list, growing the list if need be.  Both
 * the name and value are copied, and value always has a null added to the end
 * of it.
 *
 * Returns WA_ERR_NONE or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_attr_list_add(WEBAUTH_ATTR_LIST *list,
                                      const char *name, void *value, int vlen,
                                      int flags);

/*
 * Adds an attribute string to the attribute list, growing the list if need
 * be.  Name and value are not copied; the pointers are added directly.  If
 * vlen is 0, then strlen(value) is used.
 *
 * Returns WA_ERR_NONE or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_attr_list_add_str(WEBAUTH_ATTR_LIST *list,
                                          const char *name,
                                          const char *value, int vlen,
                                          int flags);


/*
 * Adds a number to an attribute list, growing the list if need be.  All of
 * these interfaces imply WA_COPY_VALUE.
 *
 * Returns WA_ERR_NONE or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_attr_list_add_uint32(WEBAUTH_ATTR_LIST *list,
                                             const char *name,
                                             uint32_t value, int flags);
WEBAUTH_API int webauth_attr_list_add_int32(WEBAUTH_ATTR_LIST *list,
                                            const char *name,
                                            int32_t value, int flags);
WEBAUTH_API int webauth_attr_list_add_time(WEBAUTH_ATTR_LIST *list,
                                           const char *name,
                                           time_t value, int flags);

/*
 * Retrieve a specific attribute by name.  Stores its value in the value
 * parameter and its length in the value_len parameter.
 *
 * If flags contains WA_F_FMT_B64, base64-decode the value.  If flags contains
 * WA_F_FMT_HEX, hex-decode the value.  If flags contains WA_F_COPY_VALUE or
 * either of those previous flags, return a copy of the value rather than a
 * pointer into the attribute.
 *
 * Returns WA_ERR_NONE, WA_ERR_CORRUPT, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_attr_list_get(WEBAUTH_ATTR_LIST *list,
                                      const char *name,
                                      void **value,
                                      int *value_len,
                                      int flags);

/*
 * Retrieve a string attribute by name.  Stores the string in value and the
 * length of the string in value_len.  Takes the same flags as
 * webauth_attr_list_get.
 *
 * Returns WA_ERR_NONE, WA_ERR_CORRUPT, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_attr_list_get_str(WEBAUTH_ATTR_LIST *list,
                                          const char *name,
                                          char **value,
                                          int *value_len,
                                          int flags);

/*
 * Retrieve a numeric attribute by name, storing it in value.  Takes the same
 * flags as webauth_attr_list_get, but WA_F_COPY_VALUE is meaningless.
 *
 * Returns WA_ERR_NONE, WA_ERR_CORRUPT, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_attr_list_get_uint32(WEBAUTH_ATTR_LIST *list,
                                             const char *name,
                                             uint32_t *value,
                                             int flags);
WEBAUTH_API int webauth_attr_list_get_int32(WEBAUTH_ATTR_LIST *list,
                                            const char *name,
                                            int32_t *value,
                                            int flags);
WEBAUTH_API int webauth_attr_list_get_time(WEBAUTH_ATTR_LIST *list,
                                           const char *name,
                                           time_t *value,
                                           int flags);

/*
 * Searches for the named attribute in the list and returns the index in i and
 * WA_ERR_NONE or sets i to -1 and returns WA_ERR_NOT_FOUND.
 */
WEBAUTH_API int webauth_attr_list_find(WEBAUTH_ATTR_LIST *list,
                                       const char *name, int *i);

/*
 * Frees the memory associated with an attribute list, including all the
 * attributes in the list.
 */
WEBAUTH_API void webauth_attr_list_free(WEBAUTH_ATTR_LIST *list);

/*
 * Given an array of attributes, returns the amount of space required to
 * encode them.
 */
WEBAUTH_API int webauth_attrs_encoded_length(const WEBAUTH_ATTR_LIST *list);

/*
 * Given an array of attributes, encode them into the buffer.  max_buffer_len
 * must be set to the maxium size of the output buffer.  output is NOT
 * null-terminated
 *
 * Returns WA_ERR_NONE or WA_ERR_NO_ROOM.
 */
WEBAUTH_API int webauth_attrs_encode(const WEBAUTH_ATTR_LIST *list,
                                     char *output, int *output_len,
                                     int max_output_len);

/*
 * Decodes the given buffer into an array of attributes.  The buffer is
 * modifed as part of the decoding.
 *
 * Returns WA_ERR_NONE, WA_ERR_CORRUPT, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_attrs_decode(char *buffer, int buffer_len,
                                     WEBAUTH_ATTR_LIST **list);


/*
 * RANDOM DATA
 */

/*
 * Returns pseudo random bytes, suitable for use a nonce or random data, but
 * not necessarily suitable for use as an encryption key.  Use
 * webauth_random_key for that.  The number of bytes specified by output_len
 * is placed in output, which must contain enough room to contain the
 * requested number of bytes.
 *
 * Returns WA_ERR_NONE on success, or WA_ERR_RAND_FAILURE on error.
 */
WEBAUTH_API int webauth_random_bytes(char *output, int num_bytes);

/*
 * Used to create random bytes suitable for use as a key.  The number of bytes
 * specified in key_len is placed in key, which must contain enough room to
 * hold key_len byte of data.
 *
 * Returns WA_ERR_NONE on success, or WA_ERR_RAND_FAILURE on error.
 */
WEBAUTH_API int webauth_random_key(char *key, int key_len);


/*
 * KEY AND KEYRING MANIPULATION
 */

/*
 * Construct new key.  key_type is the key type; currently the only supported
 * type is WA_AES_KEY.  key_material points to the key material and will get
 * copied into the new key.  key_len is the length of the key material and
 * should be WA_AES_128, WA_AES_192, or WA_AES_256.
 *
 * Returns a newly allocated key or NULL on error.
 */
WEBAUTH_API WEBAUTH_KEY *webauth_key_create(int key_type,
                                            const char *key_material,
                                            int key_len);

/* Make a copy of a key.  Returns the new key or NULL on error. */
WEBAUTH_API WEBAUTH_KEY *webauth_key_copy(const WEBAUTH_KEY *key);

/* Free a key, zeroing out the key material memory first. */
WEBAUTH_API void webauth_key_free(WEBAUTH_KEY *key);

/* Create a new keyring, returning the new keyring or NULL on error. */
WEBAUTH_API WEBAUTH_KEYRING * webauth_keyring_new(int initial_capacity);

/* Free a keyring and any keys in it. */
WEBAUTH_API void webauth_keyring_free(WEBAUTH_KEYRING *ring);

/*
 * Add a new entry to a keyring.  key is copied and the copy will be freed
 * when the keyring is freed.  If creation_time or valid_after time is 0, then
 * the current time is used.
 *
 * Returns WA_ERR_NONE or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_keyring_add(WEBAUTH_KEYRING *ring,
                                    time_t creation_time, time_t valid_after,
                                    WEBAUTH_KEY *key);

/*
 * Removes (and frees) the key at the specified index, shifting the remaining
 * keys down.
 *
 * Returns WA_ERR_NONE or WA_ERR_NOT_FOUND.
 */
WEBAUTH_API int webauth_keyring_remove(WEBAUTH_KEYRING *ring, int index);

/*
 * Given a keyring, return the best key on the ring for either encryption or
 * decryption.  The best key for encryption is the key with the most current
 * valid valid_after time.  The best key for decryption is the key with the
 * the valid_after time closest to but not more current then hint.
 *
 * Returns the key or NULL on error.
 */
WEBAUTH_API WEBAUTH_KEY *webauth_keyring_best_key(const WEBAUTH_KEYRING *ring,
                                                  int encryption, time_t hint);

/*
 * Encodes a keyring into a buffer and returns the encoded length.  buffer
 * should be freed when no longer needed.
 *
 * Returns WA_ERR_NONE or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_keyring_encode(WEBAUTH_KEYRING *ring,
                                       char **buffer, int *buffer_len);

/*
 * Deecodes a keyring from a buffer.  ring should be freed with
 * webauth_keyring_free when no longer needed.
 *
 * Returns WA_ERR_NONE, WA_ERR_CORRUPT, WA_ERR_NO_MEM, or
 * WA_ERR_KEYRING_VERSION.
 */
WEBAUTH_API int webauth_keyring_decode(char *buffer, int buffer_len,
                                       WEBAUTH_KEYRING **ring);

/*
 * Write a keyring to a file in encoded form.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_MEM, WA_ERR_KEYRING_OPENWRITE, or
 * WA_ERR_KEYRING_WRITE.
 */
WEBAUTH_API int webauth_keyring_write_file(WEBAUTH_KEYRING *ring,
                                           const char *path);

/*
 * Reads a keyring from a file in encoded form.  The newly allocated keyring
 * is stored in ring, which should be freed with webauth_keyring_Free when no
 * longer needed.
 *
 * Returns WA_ERR_NONE, WA_ERR_CORRUPT, WA_ERR_NO_MEM, WA_ERR_KEYRING_READ, or
 * WA_ERR_KEYRING_OPENREAD.
 */
WEBAUTH_API int webauth_keyring_read_file(const char *path,
                                          WEBAUTH_KEYRING **ring);

/*
 * Attempts to read a keyring file.  If create is non-zero, it will create the
 * file if it doesn't exist.  If lifetime is non-zero, there must be at least
 * one key in the ring where valid_after + lifetime is greater then the
 * current time; otherwise, a new key will be created with valid_after set to
 * the current time and the key ring file will be updated.
 *
 * This function does no file locking.
 *
 * kau_status will be set to WA_KAU_NONE if we didn't create or update the
 * ring, WA_KAU_CREATE if we attempted to create it, and WA_KAU_UPDATE if we
 * attempted to update it.
 *
 * The return code applies to only the open and/or create.  If the open and/or
 * create succeed, then WA_ERR_NONE will always be returned, even if the
 * update fails.  If the update fails, then update_status will be set to
 * someting other then WA_ERR_NONE.
 *
 * Returns WA_ERR_NONE, WA_ERR_CORRUPT, WA_ERR_NO_MEM, WA_ERR_KEYRING_READ, or
 * WA_ERR_KEYRING_OPENREAD.
 */
WEBAUTH_API int webauth_keyring_auto_update(const char *path,
                                            int create, int lifetime,
                                            WEBAUTH_KEYRING **ring,
                                            WEBAUTH_KAU_STATUS *kau_status,
                                            WEBAUTH_ERR *update_status);

/*
 * TOKEN MANIPULATION
 */

/*
 * Returns the space required to encode and encrypt a token, not including
 * nul-termination.
 */
WEBAUTH_API int webauth_token_encoded_length(const WEBAUTH_ATTR_LIST *list);

/*
 * Encodes and encrypts attributes into a token, using the key from the
 * keyring that has the most recent valid valid_from time.  If hint is 0 then
 * the current time will be used.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_ROOM, WA_ERR_NO_MEM, or WA_ERR_BAD_KEY.
 */
WEBAUTH_API int webauth_token_create(const WEBAUTH_ATTR_LIST *list,
                                     time_t hint, char *output,
                                     int *output_len, int max_output_len,
                                     const WEBAUTH_KEYRING *ring);

/*
 * Encodes and encrypts attributes into a token using the specified key.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_ROOM, WA_ERR_NO_MEM, or WA_ERR_BAD_KEY.
 *
 */
WEBAUTH_API int webauth_token_create_with_key(const WEBAUTH_ATTR_LIST *list,
                                              time_t hint, char *output,
                                              int *output_len,
                                              int max_output_len,
                                              const WEBAUTH_KEY *key);

/*
 * Decrypts and decodes attributes from a token.  The best decryption key on
 * the ring will be tried first, and if that fails all the remaining keys will
 * be tried.  input is modified and the returned attrs in list point into
 * input.
 *
 * The following checks are made:
 *
 * * If the token has a WA_TK_EXPIRATION_TIME attribute, it must be 4 bytes
 *   long and is assumed to be the expiration time of the token in network
 *   byte order.  It is compared against the current time, and
 *   WA_ERR_TOKEN_EXPIRED is returned if the token has expired.
 *
 * * WA_TK_CREATION_TIME is checked if and only if the token doesn't have an
 *   explicit expiration time and ttl is non-zero.
 *
 * * If the token has a WA_TK_CREATION_TIME attribute, it must be 4 bytes long
 *   and is assumed to be the creation time of the token in network byte
 *   order.  The creation time is compared against the current time + ttl and
 *   WA_ERR_TOKEN_STALE is returned if the token is stale.
 *
 * List will point to the dynamically-allocated list of attributes and must be
 * freed when no longer needed.
 *
 * Note: If WA_ERR_TOKEN_EXPIRED or WA_ERR_TOKEN_STALE are returned, an
 * attribute list is still allocated and needs to be freed.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_MEM, WA_ERR_CORRUPT, WA_ERR_BAD_HMAC,
 * WA_ERR_BAD_KEY, WA_ERR_TOKEN_EXPIRED, or WA_ERR_TOKEN_STALE.
 */
WEBAUTH_API int webauth_token_parse(char *input, int input_len, int ttl,
                                    const WEBAUTH_KEYRING *ring,
                                    WEBAUTH_ATTR_LIST **list);

/* Same as webauth_token_parse but takes a key instead of a keyring. */
WEBAUTH_API int webauth_token_parse_with_key(char *input, int input_len,
                                             int ttl, const WEBAUTH_KEY *key,
                                             WEBAUTH_ATTR_LIST **list);


/*
 * KERBEROS
 */

/*
 * Create new webauth krb5 context for use with all the webauth_krb5_* calls.
 * The context must be freed with webauth_krb5_free when finished.  One of the
 * various webauth_krb5_init_via* calls should be made before the context is
 * fully usable, except when using webauth_krb5_rd_req.
 *
 * If this call returns WA_ERR_KRB5, the only calls that can be made using the
 * context are webauth_krb5_error_code and webauth_krb5_error_message.  The
 * context still needs to be freed.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_MEM, or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_new(WEBAUTH_KRB5_CTXT **ctxt);

/*
 * Sets an internal flag in the context that causes webauth_krb5_free to close
 * the credential cache instead of destroying it.  This call is only useful
 * when you need a file-based cache to remain intact after a call to
 * webauth_krb5_free.
 *
 * Currently always returns WA_ERR_NONE.
 */
WEBAUTH_API int webauth_krb5_keep_cred_cache(WEBAUTH_KRB5_CTXT *context);

/*
 * Frees a context.  If the credential cache hasn't been closed, it will be
 * destroyed unless webauth_krb5_keep_cred_cache was previously called with
 * this context.
 *
 * Currently always returns WA_ERR_NONE.
 */
WEBAUTH_API int webauth_krb5_free(WEBAUTH_KRB5_CTXT *context);

/*
 * Returns the internal Kerberos error code from the last Kerberos call or 0
 * if there wasn't any error.  This code is internal to the Kerberos
 * libraries; one can't do much useful with it except report it.
 */
WEBAUTH_API int webauth_krb5_error_code(WEBAUTH_KRB5_CTXT *context);

/*
 * Returns the error message from the last Kerberos call or the string
 * "success" if the error code was 0.  The returned string points to internal
 * storage and does not need to be freed.
 */
WEBAUTH_API const char *webauth_krb5_error_message(WEBAUTH_KRB5_CTXT *context);

/*
 * Initialize a context with username/password to obtain a ticket-granting
 * ticket (TGT).  The TGT is verified using the specified keytab.  The TGT
 * will be placed in the specified cache, or a memory cache if cache_name is
 * NULL.
 *
 * If server_princpal is NULL, the first principal in the keytab will be used.
 * Otherwise, the specifed server principal will be used.
 *
 * server_principal_out will be set to the fully qualified server principal
 * used and should be freed if WA_ERR_NONE is returned.
 *
 * Returns WA_ERR_NONE, WA_ERR_LOGIN_FAILED, WA_ERR_NO_MEM, or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_init_via_password(WEBAUTH_KRB5_CTXT *context,
                                               const char *username,
                                               const char *password,
                                               const char *keytab,
                                               const char *server_principal,
                                               const char *cache_name,
                                               char **server_principal_out);

/*
 * Initialize a context with a keytab.  Credentials will be placed in the
 * specified cache, or a memory cache if cache_name is NULL.
 *
 * If server_princpal is NULL, the first principal in the keytab will be used;
 * otherwise, the specifed server principal will be used.
 *
 * Returns WA_ERR_NONE, WA_ERR_LOGIN_FAILED, or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_init_via_keytab(WEBAUTH_KRB5_CTXT *context,
                                             const char *path,
                                             const char *server_principal,
                                             const char *cache_name);

/*
 * Initialize a context with an existing credential cache.  If cache_name is
 * NULL, krb5_cc_default is used.
 *
 * Returns WA_ERR_NONE or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_init_via_cache(WEBAUTH_KRB5_CTXT *context,
                                            const char *cache_name);

/*
 * Initialize a context with a credential that was created via
 * webauth_krb5_export_tgt or webauth_krb5_export_ticket.  If cache_name is
 * NULL, a memory cache is used.
 *
 * Returns WA_ERR_NONE or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_init_via_cred(WEBAUTH_KRB5_CTXT *context,
                                           char *cred, int cred_len,
                                           const char *cache_name);

/*
 * Export the TGT from the context.  This is used to construct a proxy-token
 * after a call to webauth_krb5_init_via_password or
 * webauth_krb5_init_via_tgt.  Memory returned in tgt should be freed when it
 * is no longer needed.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_MEM, or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_export_tgt(WEBAUTH_KRB5_CTXT *context,
                                        char **tgt, int *tgt_len,
                                        time_t *expiration);

/*
 * Import a credential (TGT or ticket) that was exported via
 * webauth_krb5_export_{ticket,tgt}.  The context should have been initialized
 * by calling webauth_krb5_init_via_import first.
 *
 * Returns WA_ERR_NONE, WA_ERR_CORRUPT, WA_ERR_NO_MEM, or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_import_cred(WEBAUTH_KRB5_CTXT *context,
                                         char *cred, int cred_len);

/*
 * Create a service principal (service/hostname.do.main@realm) from a service
 * and hostname.  If service is NULL, "host" is used, and if hostname is NULL,
 * the local hostname is used.  server_principal should be freed when it is no
 * longer needed.
 *
 * Returns WA_ERR_NONE or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_service_principal(WEBAUTH_KRB5_CTXT *context,
                                               const char *service,
                                               const char *hostname,
                                               char **server_principal);

/*
 * Get the string form of the principal from the context.  This should only be
 * called after a succesfull call to webauth_krb5_init_via_*.
 *
 * If the canon argument is WA_KRB5_CANON_LOCAL, krb5_aname_to_localname is
 * called on the principal.  If krb5_aname_to_localname returns an error, the
 * fully-qualified principal name is returned.
 *
 * If the canon argument is WA_KRB5_CANON_STRIP, the realm is stripped,
 * regardless of what it is.
 *
 * If the canon argument is WA_KRB5_CANON_NONE, the fully-qualified Kerberos
 * principal is always returned.
 *
 * principal should be freed when it is no longer needed.
 *
 * Returns WA_ERR_NONE, WA_ERR_INVALID_CONTEXT, or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_get_principal(WEBAUTH_KRB5_CTXT *context,
                                           char **principal, int canon);

/*
 * Get the realm from the context.  This should only be called after a
 * successful call to webauth_krb5_init_via_*.  realm should be freed when it
 * is no longer needed.
 *
 * Returns WA_ERR_NONE, WA_ERR_INVALID_CONTEXT, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_krb5_get_realm(WEBAUTH_KRB5_CTXT *context,
                                       char **realm);

/*
 * Export a ticket for the given server_principal.  ticket should be freed
 * when no longer needed.  This should only be called after one of the
 * webauth_krb5_init_via_* methods has been successfully called.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_MEM, or WA_ERR_KRB5.
 */
WEBAUTH_API int webauth_krb5_export_ticket(WEBAUTH_KRB5_CTXT *context,
                                           char *server_principal,
                                           char **ticket, int *ticket_len,
                                           time_t *expiration);

/*
 * Calls krb5_mk_req using the specified service and stores the resulting
 * request in req, which should be freed when it is no longer needed.  This
 * should only be called after one of the webauth_krb5_init_via_* methods has
 * been successfully called.
 *
 * Returns WA_ERR_NONE, WA_ERR_KRB5, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_krb5_mk_req(WEBAUTH_KRB5_CTXT *context,
                                    const char *server_principal,
                                    char **req, int *length);

/*
 * Calls krb5_rd_req on the specified request and returns the client principal
 * in client_principal on success.  client_principal should be freed when it
 * is no longer needed.
 *
 * If server_princpal is NULL, the first principal in the keytab will be used;
 * otherwise, the specifed server principal will be used.
 *
 * If local is 1, then krb5_aname_to_localname is called on the principal.  If
 * krb5_aname_to_localname returns an error, the fully-qualified principal
 * name is returned.
 *
 * This function can be called any time after calling webauth_krb5_new.
 *
 * Returns WA_ERR_NONE, WA_ERR_KRB5, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_krb5_rd_req(WEBAUTH_KRB5_CTXT *context,
                                    const char *req, int length,
                                    const char *keytab,
                                    const char *server_principal,
                                    char **client_principal, int local);

/*
 * Similar to webauth_krb5_mk_req, but additionally calls krb5_mk_priv
 * on in_data and places the encrypted data in the out_data buffer.
 *
 * Returns WA_ERR_NONE, WA_ERR_KRB5, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_krb5_mk_req_with_data(WEBAUTH_KRB5_CTXT *context,
                                              const char *server_principal,
                                              char **req, int *length,
                                              char *in_data, int in_length,
                                              char **out_data,
                                              int *out_length);

/*
 * Similar to webauth_krb5_rd_req, but additionally calls krb5_rd_priv
 * on in_data and places the decrypted data in the out_data buffer.
 *
 * Returns WA_ERR_NONE, WA_ERR_KRB5, or WA_ERR_NO_MEM.
 */
WEBAUTH_API int webauth_krb5_rd_req_with_data(WEBAUTH_KRB5_CTXT *context,
                                              const char *req, int length,
                                              const char *keytab,
                                              const char *server_principal,
                                              char **out_server_princ,
                                              char **client_principal,
                                              int local, char *in_data,
                                              int in_length, char **out_data,
                                              int *out_length);

END_DECLS

#endif /* !_WEBAUTH_H */
