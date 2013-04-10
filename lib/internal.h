/*
 * Internal data types, definitions, and prototypes for the WebAuth library.
 *
 * Written by Russ Allbery <rra@stanford.edu>
 * Copyright 2011, 2012, 2013
 *     The Board of Trustees of the Leland Stanford Junior University
 *
 * See LICENSE for licensing terms.
 */

#ifndef LIB_INTERNAL_H
#define LIB_INTERNAL_H 1

#include <portable/macros.h>
#include <portable/stdbool.h>

#include <apr_errno.h>          /* apr_status_t */
#include <apr_pools.h>          /* apr_pool_t */
#include <apr_xml.h>            /* apr_xml_elem */
#include <webauth/basic.h>      /* enum webauth_log_level, webauth_log_func */

struct webauth_keyring;
struct webauth_token;

/*
 * Data for a logging callback for a WebAuth context.  The function pointer is
 * called with the WebAuth context, the user-provided opaque data, and the
 * string of the message to log.
 */
struct wai_log_callback {
    webauth_log_func callback;
    void *data;
};

/*
 * The internal context struct, which holds any state information required for
 * general WebAuth library interfaces.
 */
struct webauth_context {
    apr_pool_t *pool;           /* Pool used for all memory allocations. */
    const char *error;          /* Error message from last failure. */
    int code;                   /* Error code from last failure. */

    /* Logging callbacks. */
    struct wai_log_callback warn;
    struct wai_log_callback notice;
    struct wai_log_callback info;
    struct wai_log_callback trace;

    /* The below are used only for the WebKDC functions. */

    /* General WebKDC configuration. */
    struct webauth_webkdc_config *webkdc;

    /* Configuration for contacting the user metadata service. */
    struct webauth_user_config *user;
};

/*
 * An APR-managed buffer, used to accumulate data that comes in chunks.  This
 * is managed by the wai_buffer_* functions.
 */
struct buffer {
    apr_pool_t *pool;
    size_t size;
    size_t used;
    char *data;
};

/*
 * The types of data that can be encoded.  WA_TYPE_REPEAT is special and
 * indicates a part of the encoding that is repeated some number of times.
 * This is represented as a count followed by that many repetitions of some
 * structure.
 */
enum wai_encoding_type {
    WA_TYPE_DATA,
    WA_TYPE_STRING,
    WA_TYPE_INT32,
    WA_TYPE_UINT32,
    WA_TYPE_ULONG,
    WA_TYPE_TIME,
    WA_TYPE_REPEAT
};

/*
 * An encoding specification.  This is used to turn data elements into an
 * encoded attribute string, or to translate an encoded attribute string back
 * into a data structure.
 *
 * All types use offset as the offset to the basic value (obtained via
 * offsetof).  WA_TYPE_DATA also uses lenoff as the offset to the length.  For
 * WA_TYPE_REPEAT, the named attribute will be the count of elements and will
 * be stored as WA_TYPE_UINT32, and then size specifies the size of the
 * structure to store each element and repeat is a set of rules for each
 * element.  In this case, a number will be appended to the name in each rule
 * inside the repeated structure.
 *
 * Only one level of nesting of WA_TYPE_REPEAT is supported.
 */
struct wai_encoding {
    const char *attr;                   /* Attribute name in encoding */
    const char *desc;                   /* Description for error reporting */
    enum wai_encoding_type type;        /* Data type */
    bool optional;                      /* Whether attribute is optional */
    bool ascii;                         /* Whether to use ASCII-safe format */
    bool creation;                      /* Whether this is a creation time */
    size_t offset;                      /* Offset of data value */
    size_t len_offset;                  /* Offset of data value length */
    size_t size;                        /* Size of nested structure */
    const struct wai_encoding *repeat;  /* Rules for nested structure */
};

/* Used as the terminator for an encoding specification. */
#define WA_ENCODING_END { NULL, NULL, 0, false, false, false, 0, 0, 0, NULL }

/*
 * Encoding rules.  These are defined in the lib/rules-*.c files, which in
 * turn are automatically generated by the lib/encoding-rules script from the
 * comments in specific structs.
 */
extern const struct wai_encoding wai_keyring_encoding[];
extern const struct wai_encoding wai_keyring_entry_encoding[];
extern const struct wai_encoding wai_krb5_cred_encoding[];
extern const struct wai_encoding wai_krb5_cred_address_encoding[];
extern const struct wai_encoding wai_krb5_cred_authdata_encoding[];
extern const struct wai_encoding wai_token_app_encoding[];
extern const struct wai_encoding wai_token_cred_encoding[];
extern const struct wai_encoding wai_token_error_encoding[];
extern const struct wai_encoding wai_token_id_encoding[];
extern const struct wai_encoding wai_token_login_encoding[];
extern const struct wai_encoding wai_token_proxy_encoding[];
extern const struct wai_encoding wai_token_request_encoding[];
extern const struct wai_encoding wai_token_webkdc_factor_encoding[];
extern const struct wai_encoding wai_token_webkdc_proxy_encoding[];
extern const struct wai_encoding wai_token_webkdc_service_encoding[];
extern const struct wai_encoding wai_was_token_cache_encoding[];

/*
 * The internal representation of a Kerberos credential.  This representation
 * avoids any nested data structures and uses informative member names (so
 * that their stringification can be used in diagnostic output).  It is also
 * standard across all Kerberos implementations.  We later translate from this
 * structure into the krb5_creds structure in implementation-specific code.
 *
 * These structs are only used in the Kerberos code, but are present here to
 * make it easier to generate encoding rules for them.
 */
struct wai_krb5_cred_address {
    int32_t type;                       /* encode: A */
    void *data;                         /* encode: a */
    size_t data_len;
};
struct wai_krb5_cred_authdata {
    int32_t type;                       /* encode: D */
    void *data;                         /* encode: d */
    size_t data_len;
};
struct wai_krb5_cred {
    char *client_principal;             /* encode: c, optional */
    char *server_principal;             /* encode: s, optional */
    int32_t keyblock_enctype;           /* encode: K */
    void *keyblock_data;                /* encode: k */
    size_t keyblock_data_len;
    int32_t auth_time;                  /* encode: ta */
    int32_t start_time;                 /* encode: ts */
    int32_t end_time;                   /* encode: te */
    int32_t renew_until;                /* encode: tr */
    int32_t is_skey;                    /* encode: i */
    int32_t flags;                      /* encode: f */
    uint32_t address_count;             /* encode: na, optional, repeat */
    struct wai_krb5_cred_address *address;
    void *ticket;                       /* encode: t, optional */
    size_t ticket_len;
    void *second_ticket;                /* encode: t2, optional */
    size_t second_ticket_len;
    uint32_t authdata_count;            /* encode: nd, optional, repeat */
    struct wai_krb5_cred_authdata *authdata;
};

/*
 * The internal representation of a WebAuth keyring, used for encoding and
 * decoding.  This is converted to and from the public webauth_keyring struct
 * after decoding and encoding.
 *
 * These structs are only used in the internal keyring code, but are present
 * here to make it easier to generate encoding rules for them.
 */
struct wai_keyring_entry {
    time_t creation;                    /* encode: ct, ascii */
    time_t valid_after;                 /* encode: va, ascii */
    uint32_t key_type;                  /* encode: kt, ascii */
    void *key;                          /* encode: kd, ascii */
    size_t key_len;
};
struct wai_keyring {
    uint32_t version;                   /* encode: v, ascii */
    uint32_t entry_count;               /* encode: n, ascii, repeat */
    struct wai_keyring_entry *entry;
};

BEGIN_DECLS

/* Default to a hidden visibility for all internal functions. */
#pragma GCC visibility push(hidden)

/* Allocate a new buffer and initialize its contents. */
struct buffer *wai_buffer_new(apr_pool_t *)
    __attribute__((__nonnull__));

/*
 * Resize a buffer to be at least as large as the provided size.  Invalidates
 * pointers into the buffer.
 */
void wai_buffer_resize(struct buffer *, size_t);

/* Set the buffer contents, ignoring anything currently there. */
void wai_buffer_set(struct buffer *, const char *data, size_t length)
    __attribute__((__nonnull__));

/* Append data to the buffer. */
void wai_buffer_append(struct buffer *, const char *data, size_t length)
    __attribute__((__nonnull__));

/* Append printf-style data to the buffer. */
void wai_buffer_append_sprintf(struct buffer *, const char *, ...)
    __attribute__((__nonnull__, __format__(printf, 2, 3)));
void wai_buffer_append_vsprintf(struct buffer *, const char *, va_list)
    __attribute__((__nonnull__));

/*
 * Find a given string in the buffer.  Returns the offset of the string (with
 * the same meaning as start) in offset if found, and returns true if the
 * terminator is found and false otherwise.
 */
bool wai_buffer_find_string(struct buffer *, const char *, size_t start,
                            size_t *offset)
    __attribute__((__nonnull__));

/*
 * Decode the binary attribute representation into the struct pointed to by
 * data following the provided rules.
 */
int wai_decode(struct webauth_context *, const struct wai_encoding *,
               const void *input, size_t, void *data)
    __attribute__((__nonnull__));

/*
 * Similar to wai_decode, but decodes a WebAuth token, including handling the
 * determination of the type of the token from the attributes.  Uses the
 * memory pool from the WebAuth context.  This does not perform any sanity
 * checking on the token data; that must be done by higher-level code.
 */
int wai_decode_token(struct webauth_context *, const void *input, size_t,
                     struct webauth_token *)
    __attribute__((__nonnull__));

/*
 * Encode the struct pointed to by data according to given the rules into the
 * output parameter, storing the encoded data length.  The result will be in
 * WebAuth attribute encoding format.
 */
int wai_encode(struct webauth_context *, const struct wai_encoding *,
               const void *data, void **, size_t *)
    __attribute__((__nonnull__));

/*
 * Similar to wai_encode, but encodes a WebAuth token, including adding the
 * appropriate encoding of the token type.  This does not perform any sanity
 * checking on the token data; that must be done by higher-level code.
 */
int wai_encode_token(struct webauth_context *,
                     const struct webauth_token *, void **, size_t *)
    __attribute__((__nonnull__));

/* Add context to the current error, whatever it is. */
void wai_error_add_context(struct webauth_context *, const char *, ...)
    __attribute__((__nonnull__, __format__(printf, 2, 3)));

/* Set the internal WebAuth error message and error code. */
void wai_error_set(struct webauth_context *, int err, const char *, ...)
    __attribute__((__nonnull__, __format__(printf, 3, 4)));

/* The same, but include the string expansion of an APR error. */
void wai_error_set_apr(struct webauth_context *, int err, apr_status_t,
                       const char *, ...)
    __attribute__((__nonnull__, __format__(printf, 4, 5)));

/* The same, but include the string expansion of an errno. */
void wai_error_set_system(struct webauth_context *, int err, int syserr,
                          const char *, ...)
    __attribute__((__nonnull__, __format__(printf, 4, 5)));

/* Read the contents of a file into memory. */
int wai_file_read(struct webauth_context *, const char *, void **, size_t *)
    __attribute__((__nonnull__));

/* Replace the contents of a file with the provided data. */
int wai_file_write(struct webauth_context *, const void *, size_t,
                   const char *path)
    __attribute__((__nonnull__));

/*
 * Returns the amount of space required to hex encode data of the given
 * length.  Returned length does NOT include room for a null-termination.
 */
size_t webauth_hex_encoded_length(size_t length)
    __attribute__((__const__));

/*
 * Returns the amount of space required to decode the hex encoded data of the
 * given length.  Returned length does NOT include room for a
 * null-termination.
 *
 * Returns WA_ERR_NONE on succes, or WA_ERR_CORRUPT if length is not greater
 * then 0 and a multiple of 2.
 */
int webauth_hex_decoded_length(size_t length, size_t *out_length)
    __attribute__((__nonnull__));

/*
 * Hex-encodes the given data.  Does NOT null-terminate.  output can point to
 * input as long as max_output_len is long enough.
 *
 * Returns WA_ERR_NONE or WA_ERR_NO_ROOM.
 */
int webauth_hex_encode(const char *input, size_t input_len,
                       char *output, size_t *output_len,
                       size_t max_output_len)
    __attribute__((__nonnull__));

/*
 * Hex-decodes the given data.  Does NOT null-terminate.  output can point to
 * input.
 *
 * Returns WA_ERR_NONE, WA_ERR_NO_ROOM, or WA_ERR_CORRUPT.
 */
int webauth_hex_decode(char *input, size_t input_len,
                       char *output, size_t *output_length,
                       size_t max_output_len)
    __attribute__((__nonnull__));

/*
 * Log a message at various possible log levels.  This is controlled by the
 * configured callback.  If the callback is NULL, the message will be silently
 * discarded.
 */
void wai_log_info(struct webauth_context *, const char *format, ...)
    __attribute__((__nonnull__, __format__(printf, 2, 3)));
void wai_log_notice(struct webauth_context *, const char *format, ...)
    __attribute__((__nonnull__, __format__(printf, 2, 3)));
void wai_log_trace(struct webauth_context *, const char *format, ...)
    __attribute__((__nonnull__, __format__(printf, 2, 3)));
void wai_log_warn(struct webauth_context *, const char *format, ...)
    __attribute__((__nonnull__, __format__(printf, 2, 3)));

/*
 * The same, but the message is the result of calling webauth_error_message on
 * the provided error code and the level at which to log it is identified by
 * an enum.
 */
void wai_log_error(struct webauth_context *, enum webauth_log_level, int code)
    __attribute__((__nonnull__));

/*
 * Map a token type code to the corresponding encoding rule set and data
 * pointer.  Takes the token struct (which must have the type filled out), and
 * stores a pointer to the encoding rules and a pointer to the correct data
 * portion of the token struct in the provided output arguments.  Returns an
 * error code, which will be set to an error if the token type is not
 * recognized.
 */
int wai_token_encoding(struct webauth_context *, const struct webauth_token *,
                       const struct wai_encoding **, const void **)
    __attribute__((__nonnull__));

/*
 * Merge an array of webkdc-factor tokens into a single token.  Takes the
 * context, the array of webkdc-factor tokens, and a place to store the newly
 * created webkdc-factor token.  Returns a WebAuth error code.
 */
int wai_token_merge_webkdc_factor(struct webauth_context *,
                                  const apr_array_header_t *,
                                  struct webauth_token **)
    __attribute__((__nonnull__));

/*
 * Merge an array of webkdc-proxy tokens into a single token.  Takes the
 * context, the array of webkdc-proxy tokens, the maximum age in seconds for
 * tokens to contribute to the session factors, and a place to store the newly
 * created webkdc-proxy token.  Returns a WebAuth error code.
 */
int wai_token_merge_webkdc_proxy(struct webauth_context *,
                                 apr_array_header_t *,
                                 unsigned long session_limit,
                                 struct webauth_token **)
    __attribute__((__nonnull__(1, 2, 4)));

/*
 * Merge factors from a webkdc-factor token into a webkdc-proxy token,
 * returning the result in the last argument.  The webkdc-proxy token is given
 * first and the webkdc-factor token is given second.  Returns a WebAuth error
 * code.
 */
int wai_token_merge_webkdc_proxy_factor(struct webauth_context *,
                                        struct webauth_token *wkproxy,
                                        struct webauth_token *wkfactor,
                                        struct webauth_token **)
    __attribute__((__nonnull__(1, 2, 4)));

/* Retrieve all of the text inside an XML element and return it. */
int wai_xml_content(struct webauth_context *, apr_xml_elem *, const char **)
    __attribute__((__nonnull__));

/* Undo default visibility change. */
#pragma GCC visibility pop

END_DECLS

#endif /* !LIB_INTERNAL_H */
