#ifndef _LOOP_PIPELINE_H_
#define _LOOP_PIPELINE_H_

#include <stdio.h>
//#include "ap_cint.h" // deprecated
#include <ap_int.h>
#define N 20
#define NUM_TRANS 20

// typedef int5 din_t;
// typedef int20 dout_t;
// typedef ap_int<5>  din_t; // this is small 
typedef ap_int<8>  din_t;
typedef ap_int<20> dout_t;


dout_t nested_loop(din_t A[N]);

#endif
