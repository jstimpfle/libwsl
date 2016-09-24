#ifndef WSL_HEADER_VBUF_H_
#define WSL_HEADER_VBUF_H_

#include "wsl.h"

struct wsl_vbuf {
        unsigned char *bs;
        unsigned char **es;
        size_t num_bs;
        size_t num_es;
        size_t cap_bs;
        size_t cap_es;
};

void
wsl_vbuf_init(
        struct wsl_vbuf *buf);

void
wsl_vbuf_exit(
        struct wsl_vbuf *buf);

enum wsl_retcode
wsl_vbuf_append(
        struct wsl_vbuf *buf,
        unsigned char *bytes,
        size_t len);

size_t
wsl_vbuf_size(
        struct wsl_vbuf *buf,
        size_t id);

unsigned char *
wsl_vbuf_get(
        struct wsl_vbuf *buf,
        size_t id);

#endif
