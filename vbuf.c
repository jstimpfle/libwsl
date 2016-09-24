#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "vbuf.h"

void
wsl_vbuf_init(
        struct wsl_vbuf *buf)
{
        memset(buf, 0, sizeof *buf);
}

void
wsl_vbuf_exit(
        struct wsl_vbuf *buf)
{
        free(buf->bs);
        free(buf->es);
        memset(buf, 0, sizeof *buf);
}

enum wsl_retcode
wsl_vbuf_append(
        struct wsl_vbuf *buf,
        unsigned char *bytes,
        size_t len)
{
        size_t num_bs = buf->num_bs + len;
        while (num_bs >= buf->cap_bs) {
                void *ptr = realloc(buf->bs, 2 * buf->cap_bs);
                if (ptr == NULL)
                        return WSL_ENOMEM;
                buf->bs = ptr;
                buf->cap_bs = 2 * buf->cap_bs;
        }
        while (buf->num_es >= buf->cap_es) {
                void *ptr = realloc(buf->es, 2 * buf->cap_es);
                if (ptr == NULL)
                        return WSL_ENOMEM;
                buf->es = ptr;
                buf->cap_es = 2 * buf->cap_es;
        }
        memcpy(&buf->bs[buf->num_bs], bytes, len);
        buf->es[buf->num_es] = buf->bs + buf->num_bs;
        buf->num_bs += len;
        buf->num_es += 1;
        return WSL_OK;
}

size_t
wsl_vbuf_size(
        struct wsl_vbuf *buf,
        size_t id)
{
        assert(id < buf->num_es);
        unsigned char *beg = buf->es[id];
        unsigned char *end = (id + 1 < buf->num_es)
                                 ?  buf->es[id+1]
                                 : (buf->bs + buf->num_bs);
        return end - beg;
}

unsigned char *
wsl_vbuf_get(
        struct wsl_vbuf *buf,
        size_t id)
{
        assert(id < buf->num_es);
        return buf->es[id];
}
