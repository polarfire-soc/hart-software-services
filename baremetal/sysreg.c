#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include "mss_sysreg.h"

mss_sysreg_t*   SYSREG = (mss_sysreg_t*)BASE32_ADDR_MSS_SYSREG; // TODO: find a better place for this

