/*
 * lex.c
 *
 * This module reads WSL data from read buffers. It gets enough schema
 * information to split each table line into tokens corresponding to the
 * columns of the table. The lexems are stored into almost-fixed-size
 * per-column lexem buffers. (Parsing does not occur here. The lexems can be
 * parsed in per-column threads or contexts).
 *
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wsl.h"
#include "lex.h"

enum wsl_retcode
wsl_lex_outbuf_store(
        struct wsl_lex_outbuf *buffer,
        unsigned char *begin,
        unsigned char *end)
{
        unsigned char *dst;
        size_t req;

        if (buffer == NULL)
                return WSL_OK;
       
        req = end - begin + 1;
        if (WSL_LEX_OUTBUF_SIZE - buffer->size >= req) {
                dst = &buffer->buf[buffer->size];
                buffer->size += req;
        } else {
                void *ptr = realloc(buffer->bigbuf, req);
                if (ptr == NULL)
                        return WSL_ENOMEM;
                dst = ptr;
                buffer->bigbuf = ptr;
                buffer->bigsize = req;
        }
        memcpy(dst, begin, req-1);
        dst[req-1] = 0;
        return WSL_OK;
}

enum wsl_retcode
wsl_lex_token(
        struct wsl_lex_cinfo *info,
        struct wsl_lex_outbuf *buffer,
        unsigned char *begin,
        unsigned char **end_r)
{
        enum wsl_retcode r = WSL_OK;
        unsigned char *c = begin;

        if (info->type == WSL_LEX_SPACESEP) {
                while (*c > 0x20)
                        c++;
                if (*c != 0x20)
                        return WSL_ELEX;
                r = wsl_lex_outbuf_store(buffer, begin, c);
        } else if (info->type == WSL_LEX_BRACKETS) {
                if (*c != 0x5b)
                        return WSL_ELEX;
                c++;
                while (*c != 0x5d) {
                        if (*c == 0x0a)
                                return WSL_ELEX;
                        c++;
                }
                c++;
                r = wsl_lex_outbuf_store(buffer, begin+1, c-1);
        } else {
                WSL_UNREACHABLE();
        }
        *end_r = c;
        return r;
}

enum wsl_retcode
wsl_lex_token_last(
        struct wsl_lex_cinfo *info,
        struct wsl_lex_outbuf *buffer,
        unsigned char *begin,
        unsigned char **end_r)
{
        enum wsl_retcode r = WSL_OK;
        unsigned char *c = begin;

        if (info->type == WSL_LEX_STOP) {
                while (*c != 0x0a)
                        c++;
        } else if (info->type == WSL_LEX_SPACESEP) {
                while (*c != 0x0a)
                        c++;
                r = wsl_lex_outbuf_store(buffer, begin, c);
        } else if (info->type == WSL_LEX_BRACKETS) {
                if (*c != 0x5b) {
                        return WSL_ELEX;
                }
                c++;
                while (*c != 0x0a)
                        c++;
                r = wsl_lex_outbuf_store(buffer, begin+1, c-1);
        } else {
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

enum wsl_retcode
wsl_lex_row(
        struct wsl_lex_cinfo **cinfo,
        struct wsl_lex_outbuf **buffer,
        unsigned char *begin,
        unsigned char **end_r)
{
        enum wsl_retcode r = WSL_OK;
        unsigned char *c = begin;
        struct wsl_lex_cinfo **ci = cinfo;
        struct wsl_lex_outbuf **bu = buffer;

        while (*ci != NULL) {
                if (ci[1] == NULL)
                        r = wsl_lex_token_last(*ci, *bu, c, &c);
                else
                        r = wsl_lex_token(*ci, *bu, c, &c);
                if (r != WSL_OK)
                        return r;
                ci++;
                bu++;
                if (*ci == NULL)
                        break;
                if (*c != 0x20)
                        return WSL_ELEX;
                c++;
        }
        if (*c != 0x0a)
                return WSL_ELEX;
        *end_r = c + 1;
        return WSL_OK;
}

enum wsl_retcode
wsl_lex_lines(
        unsigned char *tablename,
        struct wsl_lex_cinfo **cinfo,
        struct wsl_lex_outbuf **buffer,
        unsigned char *begin,
        unsigned char *end,
        unsigned char **begin_r)
{
        enum wsl_retcode r;
        unsigned char *pos;

        assert(begin == end || (begin > end && end[-1] == 0x0a));

        r = WSL_OK;
        pos = begin;
        while (r == WSL_OK && pos < end
                        && wsl_match_table(tablename, pos, &pos)) {
                r = wsl_lex_row(cinfo, buffer, pos, &pos);
        }
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

enum wsl_retcode
wsl_lex_buffer(
        unsigned char **tablenames,
        struct wsl_lex_cinfo ***cinfo,
        struct wsl_lex_outbuf ***buffer,
        unsigned char *begin,
        unsigned char *end,
        unsigned char **begin_r)
{
        enum wsl_retcode r;
        size_t t;
        unsigned char *pos = begin;

        r = WSL_OK;
        while (r == WSL_OK && pos < end) {
                t = wsl_lookup_table(tablenames, pos);
                if (tablenames[t] == NULL)
                        return WSL_ELEX;
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
        enum wsl_retcode r;
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

                if (r != WSL_OK) {
                        fprintf(stderr, "lexer returned %d\n", r);
                        exit(1);
                }
        }
        return 0;
}
