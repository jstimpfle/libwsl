#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lex.h"

/* TODO: move away */
#define WSL_UNREACHABLE() __builtin_unreachable()
void *wsl_realloc(void *ptr, size_t nbytes)
{
        return realloc(ptr, nbytes);
}

enum wsl_lex_retcode
wsl_lex_outbuf_store(
        struct wsl_lex_outbuf *buffer,
        unsigned char *begin,
        unsigned char *end)
{
        unsigned char *dst;
        size_t req;

        if (buffer == NULL)
                return WSL_LEX_OK;
       
        req = end - begin + 1;
        if (WSL_LEX_OUTBUF_SIZE - buffer->size >= req) {
                dst = &buffer->buf[buffer->size];
                buffer->size += req;
        } else {
                void *ptr = wsl_realloc(buffer->bigbuf, req);
                if (ptr == NULL)
                        return WSL_LEX_OOM;
                dst = ptr;
                buffer->bigbuf = ptr;
                buffer->bigsize = req;
        }
        memcpy(dst, begin, req-1);
        dst[req-1] = 0;
        return WSL_LEX_OK;
}

enum wsl_lex_retcode
wsl_lex_token(
        struct wsl_lex_cinfo *info,
        struct wsl_lex_outbuf *buffer,
        unsigned char *begin,
        unsigned char **end_r)
{
        int r = WSL_LEX_OK;
        unsigned char *c = begin;

        switch (info->type) {
        case WSL_LEX_STOP:
                WSL_UNREACHABLE();
        case WSL_LEX_SPACESEP:
                while (*c > 0x20)
                        c++;
                if (*c != 0x20)
                        return WSL_LEX_ERROR;
                r = wsl_lex_outbuf_store(buffer, begin, c);
                break;
        case WSL_LEX_BRACKETS:
                if (*c != 0x5b) {
                        r = WSL_LEX_ERROR;
                        break;
                }
                c++;
                while (*c != 0x5d) {
                        if (*c == 0x0a)
                                return WSL_LEX_ERROR;
                        c++;
                }
                c++;
                r = wsl_lex_outbuf_store(buffer, begin+1, c-1);
                break;
        default:
                WSL_UNREACHABLE();
        }
        *end_r = c;
        return r;
}

enum wsl_lex_retcode
wsl_lex_token_last(
        struct wsl_lex_cinfo *info,
        struct wsl_lex_outbuf *buffer,
        unsigned char *begin,
        unsigned char **end_r)
{
        int r = WSL_LEX_OK;
        unsigned char *c = begin;

        switch (info->type) {
        case WSL_LEX_STOP:
                while (*c != 0x0a)
                        c++;
                break;
        case WSL_LEX_SPACESEP:
                while (*c != 0x0a)
                        c++;
                r = wsl_lex_outbuf_store(buffer, begin, c);
                break;
        case WSL_LEX_BRACKETS:
                if (*c != 0x5b) {
                        r = WSL_LEX_ERROR;
                        break;
                }
                c++;
                while (*c != 0x0a)
                        c++;
                r = wsl_lex_outbuf_store(buffer, begin+1, c-1);
                break;
        default:
                WSL_UNREACHABLE();
        }
        *end_r = c;
        return r;
}

int
wsl_match_table(
        unsigned char *name,
        unsigned char *begin,
        unsigned char **end_r)
{
        unsigned char *n = name;
        unsigned char *c = begin;

        while (*n && *n == *c)
                n++, c++;
        if (*n)
                return 0;
        if (*c != 0x20 && *c != 0x0a)
                return 0;
        *end_r = c;
        return 1;
}

enum wsl_lex_retcode
wsl_lex_line(
        unsigned char *tablename,
        struct wsl_lex_cinfo **cinfo,
        struct wsl_lex_outbuf **buffer,
        unsigned char *begin,
        unsigned char **end_r)
{
        int r;
        unsigned char *c;
        struct wsl_lex_cinfo **ci;
        struct wsl_lex_outbuf **bu;

        if (!wsl_match_table(tablename, begin, &c))
                return WSL_LEX_WRONG_TABLE;

        r = WSL_LEX_OK;
        ci = cinfo;
        bu = buffer;
        for (; *ci != NULL; ci++, bu++) {
                if (*c != 0x20)
                        return WSL_LEX_ERROR;
                c++;
                if (ci[1] == NULL)
                        r = wsl_lex_token_last(*ci, *bu, c, &c);
                else
                        r = wsl_lex_token(*ci, *bu, c, &c);
                if (r != WSL_LEX_OK) {
                        return r;
                }
        }
        if (*c != 0x0a)
                return WSL_LEX_ERROR;
        *end_r = c + 1;
        return WSL_LEX_OK;
}

enum wsl_lex_retcode
wsl_lex_lines(
        unsigned char *tablename,
        struct wsl_lex_cinfo **cinfo,
        struct wsl_lex_outbuf **buffer,
        unsigned char *begin,
        unsigned char *end,
        unsigned char **begin_r)
{
        int r;
        unsigned char *pos;

        assert(begin == end || (begin > end && end[-1] == 0x0a));

        r = WSL_LEX_OK;
        pos = begin;
        while (r == WSL_LEX_OK && pos < end)
                r = wsl_lex_line(tablename, cinfo, buffer, pos, &pos);
        *begin_r = pos;
        return r;
}

int
wsl_lookup_table(
        unsigned char **tablenames,
        unsigned char *line)
{
        int i = 0;
        unsigned char *dummy = NULL;
        while (tablenames[i] && !wsl_match_table(tablenames[i], line, &dummy))
                i++;
        return i;
}

enum wsl_lex_retcode
wsl_lex_buffer(
        unsigned char **tablenames,
        struct wsl_lex_cinfo ***cinfo,
        struct wsl_lex_outbuf ***buffer,
        unsigned char *begin,
        unsigned char *end,
        unsigned char **begin_r)
{
        int r;
        size_t t;
        unsigned char *pos = begin;

        r = WSL_LEX_OK;
        while (r == WSL_LEX_OK && pos < end) {
                t = wsl_lookup_table(tablenames, pos);
                if (tablenames[t] == NULL)
                        return WSL_LEX_ERROR;
                r = wsl_lex_lines(tablenames[t], cinfo[t], buffer[t],
                                 pos, end, &pos);
        }
        *begin_r = pos;
        return r;
}

struct wsl_lex_cinfo spacesep = { WSL_LEX_SPACESEP };
struct wsl_lex_cinfo bracketsep = { WSL_LEX_BRACKETS };
struct wsl_lex_cinfo skip = { WSL_LEX_STOP };

/* some buffers, use for free */
struct wsl_lex_outbuf buf[16];
struct wsl_lex_outbuf *bufarray[] = { &buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5], &buf[6], &buf[7] };
struct wsl_lex_outbuf *empty[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/* baden-wuerttemberg dataset */
struct wsl_lex_cinfo *info_way[] = { /* &skip, NULL, */ &spacesep, NULL };
struct wsl_lex_cinfo *info_node[] = { /* &skip, NULL, */ &spacesep, &spacesep, &spacesep, NULL };
struct wsl_lex_cinfo *info_waypoint[] = { /*&skip, NULL, */ &spacesep, &spacesep, &spacesep, NULL };

char *bw_tables[] = {
        "way", "node", "waypoint", NULL
};

struct wsl_lex_cinfo **bw_cinfo[] = {
        info_way, info_node, info_waypoint
};

struct wsl_lex_outbuf **bw_buffers[] = {
        empty, empty, empty,
        bufarray, bufarray, bufarray
};


/* world dataset */
struct wsl_lex_cinfo *info_country[] = { &spacesep, &spacesep, &bracketsep, NULL };
struct wsl_lex_cinfo *info_city[] = { &spacesep, &bracketsep, &spacesep, &bracketsep, &spacesep, NULL };
struct wsl_lex_cinfo *info_capital[] = { &spacesep, &spacesep, NULL };
struct wsl_lex_cinfo *info_language[] = { &spacesep, &bracketsep, &spacesep, &spacesep, NULL };

char *world_tables[] = {
        "Country", "City", "Capital", "Language", NULL
};

struct wsl_lex_cinfo **world_cinfo[] = {
        info_country, info_city, info_capital, info_language
};

struct wsl_lex_outbuf **world_buffers[] = {
        empty, empty, empty, empty,
        bufarray, bufarray, bufarray, bufarray
};

#define BUFSIZE (16*(size_t)4096)

int main(void)
{
        int r;
        size_t size, end;
        size_t nread;
        unsigned char buf[BUFSIZE];
        unsigned char *lexpos;

        size = 0;
        end = 0;

        for (;;) {
                size = size - end;
                memmove(&buf[0], &buf[end], size);
                nread = fread(&buf[size], 1, BUFSIZE-size, stdin);
                size += nread;

                if (size == 0)
                        break;

                end = size;
                while (end > 0 && buf[end-1] != 0x0a)
                        end--;

                if (end == 0) {
                        fprintf(stderr, "Line did not fit in buffer!\n");
                        fprintf(stderr, "ERROR: Handling this situation not implemented!\n");
                        exit(1);
                }

#if 0
                r = wsl_lex_buffer((unsigned char **)world_tables,
                                   world_cinfo, world_buffers,
                                   &buf[0], &buf[end], &lexpos);
#else
                r = wsl_lex_buffer((unsigned char **)bw_tables,
                                   bw_cinfo, bw_buffers,
                                   &buf[0], &buf[end], &lexpos);
#endif

                if (r != WSL_LEX_OK) {
                        fprintf(stderr, "lexer returned %d\n", r);
                        exit(1);
                }
        }
        return 0;
}
