#ifndef _LOOP_PIPELINE_H_
#define _LOOP_PIPELINE_H_

#include <stdio.h>
#include "ap_cint.h"
#define N 20
#define NUM_TRANS 20

typedef int5 din_t;
typedef int20 dout_t;

dout_t nested_loop(din_t A[N]);

#endif
