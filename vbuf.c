#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "vbuf.h"

enum wsl_retcode
wsl_vbuf_require_mem(
        void **buf,
        size_t *cap,
        size_t elsize,
        size_t init,
        size_t req)
{
        void *ptr;
        size_t alloc = *cap;

        if (alloc >= req)
                return WSL_OK;
        if (alloc == 0)
                alloc = init;
        while (alloc < req)
                alloc *= 2;
        ptr = realloc(*buf, alloc * elsize);
        if (ptr == NULL)
                return WSL_ENOMEM;
        *buf = ptr;
        *cap = alloc;
        return WSL_OK;
}

/**
 * Make sure that the bytes buffer in *buf* has capacity of at least *num_bs*
 * bytes.
 *
 * Return:
 *      - WSL_OK: if there is now enough capacity, possibly after a required
 *              realloction
 *      - WSL_ENOMEM: if reallocation is required but couldn't be satisfied.
 */
enum wsl_retcode
wsl_vbuf_require_bs(
        struct wsl_vbuf *buf,
        size_t num_bs)
{
        return wsl_vbuf_require_mem(&buf->bs, &buf->cap_bs, 1, 128, num_bs);
}

/**
 * Make sure that the element pointers buffer in *buf* has capacity for at least
 * *num_bs* pointers.
 *
 * Return:
 *      - WSL_OK: if there is now enough capacity, possibly after a required
 *              realloction
 *      - WSL_ENOMEM: if reallocation is required but couldn't be satisfied.
 */
enum wsl_retcode
wsl_vbuf_require_es(
        struct wsl_vbuf *buf,
        size_t num_es)
{
        return wsl_vbuf_require_mem(
                        &buf->es, &buf->cap_es, sizeof *buf->es, 16, num_es);
}

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
        enum wsl_retcode r;
        size_t num_bs;
        size_t num_es;

        num_bs = buf->num_bs + len;
        r = wsl_vbuf_require_bs(buf, num_bs);
        if (r != WSL_OK)
                return r;
        num_es = buf->num_es + 1;
        r = wsl_vbuf_require_es(buf, num_es);
        if (r != WSL_OK)
                return r;
        memcpy(&buf->bs[buf->num_bs], bytes, len);
        buf->num_bs += len;
        buf->es[buf->num_es] = buf->bs + buf->num_bs;
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
