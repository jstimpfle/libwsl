#include <stddef.h>  /* size_t */

/**
 * The size of a lex output buffer
 *
 * While a table is lexed, each column's values parses into its own output
 * buffer. The size of this buffer controls how much row data can be lexed
 * before the output needs to be processed.
 */
#define WSL_LEX_OUTBUF_SIZE (16*4096)

/**
 * Lex buffer for values of a column.
 *
 * If a lexem unexpectedly is too big to fit in the remaining space,
 * a big enough extra buffer is dynamically allocated and the lexec
 * is copied to it.
 */
struct wsl_lex_outbuf {
        unsigned char buf[WSL_LEX_OUTBUF_SIZE];
        size_t size;
        unsigned char *bigbuf;
        size_t bigsize;
};

/**
 * Lexical element types
 *
 * @WSL_LEX_STOP: Special symbol; signals skipping the rest of the line
 * @WSL_LEX_SPACESEP: Space (or newline) delimited token
 * @WSL_LEX_BRACKETS: Angle bracket delimited token; [looks like this]
 */
enum wsl_lex_coltype {
        WSL_LEX_STOP = 0,
        WSL_LEX_SPACESEP = 1,
        WSL_LEX_BRACKETS = 2
};

/**
 * Lexical parsing info for a table column
 *
 * @type: The lexer type to use.
 */
struct wsl_lex_cinfo {
        enum wsl_lex_coltype type;
};

/**
 * Metadata indicating how to lex a table
 *
 * @name: Name of the table
 * @cinfo: A NULL-terminated array of wsl_lex_cinfo.
 */
struct wsl_lex_tinfo {
        const char *name;
        struct wsl_lex_cinfo **cinfo;
};

/*
 * @WSL_LEX_OK: Everything lexed as expected
 * @WSL_LEX_OUTBUF_FULL: At least one of the output buffers is full
 * @WSL_LEX_WRONG_TABLE: Parsing was stopped because another table found
 * @WSL_LEX_ERROR: An error in the input was detected
 * @WSL_LEX_OOM: Dynamic memory allocation failed
 */
enum wsl_lex_retcode {
        WSL_LEX_OK = 0,
        WSL_LEX_OUTBUF_FULL = 1,
        WSL_LEX_WRONG_TABLE = 2,
        WSL_LEX_ERROR = 3,
        WSL_LEX_OOM = 4
};

/**
 * Lex a single token
 *
 * @info: What kind of token to lex.
 * @buffer: Where to store the lexem, or NULL
 * @begin: Where to start lexing. This buffer must be terminated by a newline
 *      character
 * @end_r: If lexing was successful, the address of the first unconsumed
 *      character is stored here.
 */
enum wsl_lex_retcode
sl_lex_token(
        struct wsl_lex_cinfo *info,
        struct wsl_lex_outbuf *buffer,
        unsigned char *begin,
        unsigned char **end_r);

/**
 * Lex the last token in a line
 *
 * @info: What kind of token to lex.
 * @buffer: Where to store the lexem, or NULL
 * @begin: Where to start lexing. This buffer must be terminated by a newline
 *      character
 * @end_r: If lexing was successful, the address of the first unconsumed
 *      character is stored here.
 *
 * This works like wsl_lex_token(), except that the token is expected to be
 * followed by the terminating newline character (instead of space).
 */
enum wsl_lex_retcode
wsl_lex_token_last(
        struct wsl_lex_cinfo *info,
        struct wsl_lex_outbuf *buffer,
        unsigned char *begin,
        unsigned char **end_r);

/**
 * Lex a line of tokens separated by single spaces.
 *
 * @tablename: Name of the table of which a row is expected.
 * @info: NULL-terminated array of cinfo pointers corresponding to given table.
 * @buffer: Array of buffer pointers corresponding to the columns in the given
 *      table. Buffer pointers may be NULL in which case the lexems are
 *      discarded.
 * @begin: Pointer to beginning of the line to be lexed. Line must be
 *      terminated by a newline character.
 * @begin_r: If lexing was successful, the address of the character following
 *      the terminating newline character is stored here.
 */
enum wsl_lex_retcode
wsl_lex_line(
        unsigned char *tablename,
        struct wsl_lex_cinfo **info,
        struct wsl_lex_outbuf **buffer,
        unsigned char *begin,
        unsigned char **end_r);

/**
 * Lex lines of a given table
 *
 * @tablename: Name of the table of which rows should be lexed
 * @info: NULL-terminated array of cinfo pointers corresponding to given table.
 * @buffer: Array of buffer pointers corresponding to the columns in the given
 *      table. Buffer pointers may be NULL in which case the lexems are
 *      discarded.
 * @begin: Pointer to beginning of the buffer to be lexed.
 * @end: Pointer to the byte past the end of the last line that should be lexed.
 * @begin_r: When this function returns, the storage at that location contains
 *      the address of the first line that was *not* lexed successfully.
 *
 * Return: WSL_OK if all lines in the buffer were lexed. Otherwise, the reason
 *      why the line at *begin_r* was not lexed successfully:
 *
 *      - WSL_LEX_ERROR
 *      - WSL_LEX_WRONG_TABLE
 *      - WSL_LEX_OUTBUF_FULL
 *      - WSL_LEX_OOM
 */
enum wsl_lex_retcode
wsl_lex_lines(
        unsigned char *tablename,
        struct wsl_lex_cinfo **info,
        struct wsl_lex_outbuf **buffer,
        unsigned char *begin,
        unsigned char *end,
        unsigned char **begin_r);

/**
 * Lookup the table of a tuple line from a list of table names.
 *
 * @tablenames: NULL-terminated array of tablenames.
 * @line: A newline-terminated buffer.
 *
 * Returns: The index of the tablename matching the first word (delimited by
 * space or the terminating newline) in the buffer. If no word matches, the
 * index of the terminating NULL-pointer is returned.
 */
int
wsl_lookup_table(
        unsigned char **tablenames,
        unsigned char *line);

/**
 * Lex multiple rows of given table and copy lexems to buffers.
 *
 * @tablenames: Array of the names of all tables
 * @info: For each table name an array of cinfo pointers.
 * @buffer: For each table name an array of buffer pointers. Buffer pointers
 *      may be NULL in which case the lexems are discarded.
 * @begin: Pointer to the beginning of the first line that should be lexed.
 * @end: Pointer to the byte one past the end of the last line that should be
 *      lexed (newline character + 1). It must hold `begin <= end`.
 * @begin_r: On exit, the address of the byte one past the end of the last
 *      line that was successfully lexed is stored in this pointer (or `begin`
 *      if no line was lexed successfully).
 *
 * Return: All values of `enum wsl_lex_retcode` are possible return values.
 */
enum wsl_lex_retcode
wsl_lex_buffer(
        unsigned char **tablenames,
        struct wsl_lex_cinfo ***info,
        struct wsl_lex_outbuf ***buffer,
        unsigned char *begin,
        unsigned char *end,
        unsigned char **begin_r);
