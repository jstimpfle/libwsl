/* integer datatype */

#include <assert.h>
#include "wsl.h"
#include "buf.h"
#include "dt.h"

WSL_DOMAIN_INITOR wsl_dt_str_init;
WSL_DOMAIN_EXITOR wsl_dt_str_exit;

struct wsl_datatype wsl_dt_str = {
        wsl_dt_str_init,
        wsl_dt_str_exit
};

enum wsl_retcode
wsl_dt_str_init(
        const char *decl,
        void **parm_r)
{
        if (*decl)
                return WSL_EPARSE;
        *parm_r = NULL;
        return WSL_OK;
}

void
wsl_dt_str_exit(
        void *parm)
{
        (void) parm;
        return;
}
