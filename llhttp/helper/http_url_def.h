#ifndef HTTP_URL_DEF_H
#define HTTP_URL_DEF_H

#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

enum http_parser_url_fields
{
    UF_SCHEMA           = 0
    , UF_HOST             = 1
    , UF_PORT             = 2
    , UF_PATH             = 3
    , UF_QUERY            = 4
    , UF_FRAGMENT         = 5
    , UF_USERINFO         = 6
    , UF_MAX              = 7
};

/* Result structure for http_parser_parse_url().
 *
 * Callers should index into field_data[] with UF_* values iff field_set
 * has the relevant (1 << UF_*) bit set. As a courtesy to clients (and
 * because we probably have padding left over), we convert any port to
 * a uint16_t.
 */
struct http_parser_url
{
    uint16_t field_set;           /* Bitmask of (1 << UF_*) values */
    uint16_t port;                /* Converted UF_PORT string */

    struct
    {
        uint16_t off;               /* Offset into buffer in which field starts */
        uint16_t len;               /* Length of run in buffer */
    } field_data[UF_MAX];
};

/* Initialize all http_parser_url members to 0 */
void http_parser_url_init(struct http_parser_url* u);

/* Parse a URL; return nonzero on failure */
int http_parser_parse_url(const char* buf, size_t buflen,
                          int is_connect,
                          struct http_parser_url* u);
#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // HTTP_URL_DEF_H
