#ifndef NTB_NETIO_H
#define NTB_NETIO_H

#include "ntb_dp.h"

extern void
ntb_netio_write_lock(void);

extern void
ntb_netio_write_unlock(void);

extern void
ntb_netio_read_lock(void);

extern void
ntb_netio_read_unlock(void);

extern int
ntb_netio_init(ntb_pmd_ctx_t *ctx);

extern int
ntb_netio_deinit(ntb_pmd_ctx_t *ctx);

extern int
ntb_netio_run(ntb_pmd_ctx_t *ctx);

#endif
