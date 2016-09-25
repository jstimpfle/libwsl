#include "wsl.h"

const char *
wsl_error_string(
        enum wsl_retcode r)
{
        switch (r) {
        case WSL_OK: return "WSL_OK";
        case WSL_ERROR: return "WSL_ERROR";
        case WSL_EINVAL: return "WSL_EINVAL";
        case WSL_ELEX: return "WSL_ELEX";
        case WSL_EPARSE: return "WSL_EPARSE";
        case WSL_ENOMEM: return "WSL_ENOMEM";
        }
}
