#ifndef INTERLEAVER_H_
#define INTERLEAVER_H_

#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <ap_int.h>

typedef ap_axiu<96,0,0,0> axis_data_t;
typedef hls::stream<axis_data_t> axis_stream_t;
typedef ap_uint<128> control_word_t;

// ceil(13120 / 96) = 137 words max
#define MAX_WORDS 137

void Interleaver(
    axis_stream_t& inData,
    axis_stream_t& outData,
    control_word_t cnData
);
#endif
