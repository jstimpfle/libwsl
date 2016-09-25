#ifndef WSL_HEADER_WSL_H_
#define WSL_HEADER_WSL_H_

/**
 * Type used for most return status codes in this code base.
 *
 * @WSL_OK: Success
 * @WSL_ERROR: Generic error
 * @WSL_EINVAL: Invalid input
 * @WSL_ELEX: Lexer error
 * @WSL_EPARSE: Parser error
 * @WSL_ENOMEM: Failed memory allocation
 */
enum wsl_retcode {
        WSL_OK,
        WSL_ERROR,
        WSL_EINVAL,
        WSL_ELEX,
        WSL_EPARSE,
        WSL_ENOMEM
};

const char *
wsl_error_string(
        enum wsl_retcode r);

#define WSL_UNREACHABLE() __builtin_unreachable()

#endif
