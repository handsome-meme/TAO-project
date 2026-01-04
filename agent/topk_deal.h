#ifndef TOPK_DEAL_H
#define TOPK_DEAL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "entry_api.h"


#ifdef __cplusplus
extern "C" {
#endif

void * shc_topk_thread(void *);
void diff_cmp(int a);

#ifdef __cplusplus
}
#endif

#endif
