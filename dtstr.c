/* string datatype */

#include <string.h>
#include <stdlib.h>
#include "dt.h"
#include "dtstr.h"
#include "vbuf.h"

struct wsl_datatype wsl_dt_str = {
        wsl_dt_str_init,
        wsl_dt_str_exit
};

enum wsl_retcode
wsl_dt_str_init(
        const char *decl,
        void **parm_r)
{
        if (*decl)
                return WSL_EPARSE;
        *parm_r = NULL;
        return WSL_OK;
}

void
wsl_dt_str_exit(
        void *parm)
{
        (void) parm;
        return;
}

enum wsl_retcode
wsl_dt_str_column_init(
        void *parm,
        void **column_r)
{
        struct wsl_vbuf *col;

        if (parm != NULL)
                WSL_UNREACHABLE();
        col = malloc(sizeof *col);
        if (col == NULL)
                return WSL_ENOMEM;
        wsl_vbuf_init(col);
        *column_r = col;
        return WSL_OK;
}

void
wsl_dt_str_column_exit(
        void *column)
{
        struct wsl_vbuf *col = column;
        wsl_vbuf_exit(col);
        free(col);
}

enum wsl_retcode
wsl_dt_str_encode(
        void *column,
        size_t first,
        size_t last,
        size_t *last_r,
        unsigned char *buf,
        unsigned char *end,
        unsigned char **end_r)
{
        enum wsl_retcode r = WSL_OK;
        struct wsl_vbuf *col = column;
        unsigned char *c = buf;
        size_t id = first;

        while (id < last) {
                size_t sz = wsl_vbuf_size(col, id);
                if (c + sz + 1 >= end) {
                        r = WSL_ENOMEM;
                        break;
                }
                memcpy(c, wsl_vbuf_get(col, id), sz);
                c += sz;
                *c++ = 0x00;
                id++;
        }
        *last_r = id;
        *end_r = c;
        return r;
}

enum wsl_retcode
wsl_dt_str_decode(
        void *column,
        unsigned char *buf,
        unsigned char *end,
        unsigned char **end_r)
{
        enum wsl_retcode r = WSL_OK;
        struct wsl_vbuf *col = column;
        unsigned char *c = buf;

        while (c < end) {
                size_t sz = strlen((char *) buf);
                r = wsl_vbuf_append(col, buf, sz);
                if (r != WSL_OK)
                        break;
                c += sz + 1;
        }
        *end_r = c;
        return r;
}
