#ifndef SHC_NETIO_H
#define SHC_NETIO_H

#include "shc_dp.h"

extern void
shc_netio_write_lock(void);

extern void
shc_netio_write_unlock(void);

extern void
shc_netio_read_lock(void);

extern void
shc_netio_read_unlock(void);

extern int
shc_netio_init(shc_pmd_ctx_t *ctx);

extern int
shc_netio_deinit(shc_pmd_ctx_t *ctx);

extern int
shc_netio_run(shc_pmd_ctx_t *ctx);

#endif
