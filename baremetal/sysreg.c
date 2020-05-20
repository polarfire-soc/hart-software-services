#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include "mss_sysreg.h"

mss_sysreg_t * const SYSREG = (mss_sysreg_t * const)BASE32_ADDR_MSS_SYSREG; 

#if 0
/*==============================================================================
 * Copy hardware configuration to registers.
 * This function should be used in place of memcpy() to cover the use case
 * where C library code has not yet been copied from its LMA to VMA. For
 * example copying before the copy_section of the .text section has taken
 * place.
 */
char * config_copy(void *dest, const void * src, size_t len);
char * config_copy(void *dest, const void * src, size_t len)
{
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    for(int inc = 0; inc < len; inc++) {
        cdest[inc] = csrc[inc];
    }

    return(csrc);
}
#endif
