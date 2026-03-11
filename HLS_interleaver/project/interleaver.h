#ifndef INTERLEAVER_H_
#define INTERLEAVER_H_

#include <ap_axi_sdata.h>
#include <hls_stream.h>

// AXI-Stream data type with 96-bit data and last signal
// Parameters: <data_width, user_width, id_width, dest_width>
// Setting user, id, dest to 0 disables them
typedef ap_axiu<96, 0, 0, 0> axis_data_t;
typedef hls::stream<axis_data_t> axis_stream_t;

// Control port data type
typedef ap_uint<128> control_word_t;

// Maximum E value across test cases: 13120 bits
// Maximum Qm value: 8
// Maximum columns = ceil(13120/8) = 1640
#define MAX_COLS 1640
#define MAX_ROWS 8

// Function prototype
void Interleaver(
    axis_stream_t& inData,
    axis_stream_t& outData,
    control_word_t cnData
);

#endif
