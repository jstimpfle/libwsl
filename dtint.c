/* integer datatype */

#include <assert.h>
#include "wsl.h"
#include "buf.h"
#include "dt.h"

WSL_DOMAIN_INITOR wsl_dt_int_init;
WSL_DOMAIN_EXITOR wsl_dt_int_exit;

struct wsl_datatype wsl_dt_int = {
        wsl_dt_int_init,
        wsl_dt_int_exit
};

enum wsl_retcode
wsl_parseint(
        unsigned char *buf,
        unsigned char **end_r,
        int *value_r)
{
        int n;
        int neg;
        unsigned char *c;

        if (*buf == 0x30) {
                if (buf[1] != 0x00)
                        return WSL_EPARSE;
                n = 0;
                c = buf+2;
                goto out;
        }
        if (*buf == 0x2d) {
                neg = 1;
                c = buf+1;
        } else {
                neg = 0;
                c = buf;
        }
        if (!(0x30 <= *c && *c <= 0x39))
                return WSL_EPARSE;
        n = *c++ - 0x30;
        while (0x30 <= *c && *c <= 0x39)
                n = 10*n + *c++ - 0x30;
        if (neg)
                n = -n;
        if (*c != 0x00)
                return WSL_EPARSE;
out:
        *end_r = c;
        *value_r = n;
        return WSL_OK;
}

enum wsl_retcode
wsl_formatint(
        int x,
        unsigned char *buf,
        unsigned char *end,
        unsigned char **end_r)
{
        int y;
        size_t n;
        unsigned char *c;
        unsigned char d[64];

        n = 0;
        if (x == 0) {
                d[n++] = 0 + 0x30;
        } else {
                y = x;
                while (y != 0) {
                        d[n++] = (y % 10) + 0x30;
                        y = y / 10;
                }
        }
        if (x < 0)
                d[n++] = 0x2d;
        if (end < buf + n+1)
                return WSL_ENOMEM;

        c = buf;
        while (n--)
                *c++ = d[n];
        *c++ = 0x00;
        *end_r = c;
        return WSL_OK;
}

enum wsl_retcode
wsl_dt_int_init(
        const char *decl,
        void **parm_r)
{
        if (*decl)
                return WSL_EPARSE;
        *parm_r = NULL;
        return WSL_OK;
}

void
wsl_dt_int_exit(
        void *parm)
{
        (void) parm;
        return;
}

enum wsl_retcode
wsl_dt_int_encode_one(
        void *column,
        size_t id,
        unsigned char *buf,
        unsigned char *end,
        unsigned char **end_r)
{
        int x;
        enum wsl_retcode r;

        x = wsl_buf_int_get((struct wsl_buf_int *) column, id);
        r = wsl_formatint(x, buf, end, end_r);
        assert(r == WSL_OK || r == WSL_EINVAL || r == WSL_ENOMEM);
        return r;
}

enum wsl_retcode
wsl_dt_int_decode_one(
        void *column,
        unsigned char *buf,
        unsigned char **end_r)
{
        enum wsl_retcode r;
        struct wsl_buf_int *col = column;

        int x;
        r = wsl_parseint(buf, end_r, &x);
        if (r != WSL_OK) {
                assert(r == WSL_EPARSE);
                return r;
        }

        r = wsl_buf_int_append(col, x);
        return r;
}

enum wsl_retcode
wsl_dt_int_encode(
        void *column,
        size_t first,
        size_t last,
        size_t *last_r,
        unsigned char *buf,
        unsigned char *end,
        unsigned char **end_r)
{
        enum wsl_retcode r = WSL_OK;
        size_t id = first;
        unsigned char *c = buf;

        while (id < last) {
                r = wsl_dt_int_encode_one(column, id, c, end, &c);
                if (r != WSL_OK)
                       break;
                id++;
        }

        *last_r = id;
        *end_r = c;
        return r;
}

enum wsl_retcode
wsl_dt_int_decode(
        void *column,
        unsigned char *buf,
        unsigned char *end,
        unsigned char **end_r)
{
        enum wsl_retcode r = WSL_OK;
        unsigned char *c = buf;

        while (r == WSL_OK && c < end)
                r = wsl_dt_int_decode_one(column, c, &c);
        *end_r = c;
        return r;
}
