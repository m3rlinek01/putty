/*
 * Escaping/unescaping functions to translate between a saved session
 * name, and the key name used to represent it in the Registry area
 * where we store saved sessions.
 *
 * The basic technique is to %-escape characters we can't use in
 * Registry keys.
 */

#include "putty.h"

void escape_registry_key(const char *in, strbuf *out)
{
    bool candot = false;
    static const char hex[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};

    while (*in) {
        if (*in == ' ' || *in == '\\' || *in == '*' || *in == '?' ||
            *in == '%' || *in < ' ' || *in > '~' || (*in == '.'
                                                     && !candot)) {
            put_byte(out, '%');
            put_byte(out, hex[((unsigned char) *in) >> 4]);
            put_byte(out, hex[((unsigned char) *in) & 15]);
        } else
            put_byte(out, *in);
        in++;
        candot = true;
    }
}

void unescape_registry_key(const char *in, strbuf *out)
{
    while (*in) {
        if (*in == '%' && in[1] && in[2]) {
            int i, j;

            i = in[1] - '0';
            i -= (i > 9 ? 7 : 0);
            j = in[2] - '0';
            j -= (j > 9 ? 7 : 0);

            put_byte(out, (i << 4) + j);
            in += 3;
        } else {
            put_byte(out, *in++);
        }
    }
}
