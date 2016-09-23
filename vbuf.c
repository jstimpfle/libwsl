#include <stdlib.h>
#include "wsl.h"

struct wsl_vbuf {
        unsigned char *bs;
        unsigned char **es;
        size_t num_bs;
        size_t num_es;
        size_t cap_bs;
        size_t cap_es;
};

enum wsl_retcode
wsl_vbuf_init(
        struct wsl_vbuf *buf)
{
        memset(buf, 0, sizeof buf);
}

enum wsl_retcode
wsl_vbuf_exit(
        struct wsl_vbuf *buf)
{
        free(buf->bs);
        free(buf->es);
        memset(buf, 0, sizeof buf);
}

enum wsl_retcode
wsl_vbuf_append(
        struct wsl_vbuf *buf,
        unsigned char *bytes,
        size_t len)
{
        size_t num_bs = buf->num_bs + len;
        while (num_bs >= buf->cap_bs) {
                void *ptr = realloc(buf->bs, 2 * cap_bs);
                if (ptr == NULL)
                        return WSL_ENOMEM;
                buf->bs = ptr;
                buf->cap_bs = 2 * cap_bs;
        }
        while (num_es >= buf->cap_es) {
                void *ptr = realloc(buf->es, 2 * cap_es);
                if (ptr == NULL)
                        return WSL_ENOMEM;
                buf->es = ptr;
                buf->cap_es = 2 * cap_es;
        }
        memcpy(&buf->bs[buf->num_bs], bytes, len);
        buf->es[buf->num_es] = buf->num_bs;
        buf->num_bs += len;
        buf->num_es += 1;
        return WSL_OK;
}
