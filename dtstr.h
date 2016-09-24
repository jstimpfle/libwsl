#ifndef WSL_HEADER_DTSTR_H_
#define WSL_HEADER_DTSTR_H_

/* string datatype */

#include <assert.h>
#include <string.h>
#include "wsl.h"
#include "vbuf.h"
#include "dt.h"

struct wsl_datatype wsl_dt_str;

WSL_DOMAIN_INITOR wsl_dt_str_init;
WSL_DOMAIN_EXITOR wsl_dt_str_exit;
WSL_COLUMN_INITOR wsl_dt_str_column_init;
WSL_COLUMN_EXITOR wsl_dt_str_column_exit;
WSL_VALUE_ENCODER wsl_dt_str_encode;
WSL_VALUE_DECODER wsl_dt_str_decode;

#endif
