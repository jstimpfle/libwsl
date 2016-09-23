#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "wsl.h"

#define WSL_DEFINE_BUF(type, name) \
struct wsl_buf_##name { \
        type *buf; \
        size_t size; \
        size_t capacity; \
}; \
\
void \
wsl_buf_##name ##_init(struct wsl_buf_##name *buf) \
{ \
        memset(buf, 0, sizeof *buf); \
} \
\
void \
wsl_buf_##name ##_exit(struct wsl_buf_##name *buf) \
{ \
        free(buf->buf); \
        memset(buf, 0, sizeof *buf); \
} \
\
enum wsl_retcode \
wsl_buf_##name ##_append(struct wsl_buf_##name *buf, type elt) \
{ \
        if (buf->size == buf->capacity) { \
                void *ptr = realloc(buf->buf, \
                                    2 * buf->capacity * sizeof (type)); \
                if (ptr == NULL) \
                        return WSL_ENOMEM; \
                buf->buf = ptr; \
                buf->capacity *= 2; \
        } \
        buf->buf[buf->size++] = elt; \
        return WSL_OK; \
} \
\
type \
wsl_buf_##name ##_get(struct wsl_buf_##name *buf, size_t id) \
{ \
        assert(id < buf->size); \
        return buf->buf[id]; \
} \
\
void \
wsl_buf_##name ##_clear(struct wsl_buf_##name *buf) \
{ \
        buf->size = 0; \
} \
\
enum wsl_retcode \
wsl_buf_##name ##_shrink_to_fit(struct wsl_buf_##name *buf) \
{ \
        void *ptr = realloc(buf->buf, buf->size * sizeof (type)); \
        if (ptr == NULL) \
                return WSL_ENOMEM; \
        buf->buf = ptr; \
        buf->capacity = buf->size; \
        return  WSL_OK; \
}

WSL_DEFINE_BUF(int, int)
