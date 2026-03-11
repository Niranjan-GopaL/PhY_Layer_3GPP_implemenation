#ifndef _HEADER_H_  // Header guard - prevents multiple inclusions
#define _HEADER_H_

// HLS-specific includes for hardware synthesis
#include <hls_stream.h>     // For streaming interfaces (FIFO-like)
#include <ap_int.h>          // For arbitrary precision integers
#include <ap_fixed.h>        // For fixed-point arithmetic
#include <hls_math.h>        // HLS-optimized math functions
#include <ap_axi_sdata.h>    // For AXI stream protocol (with 'last' signal)

// Standard C++ includes (for testbench, not synthesizable)
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <fstream>           // File operations (testbench only)
#include <iomanip>           // Output formatting (testbench only)
#include <math.h>            // Math functions

using namespace std;

// Type definitions - making code more readable by giving meaningful names
typedef ap_uint<128> datau128b;  // 128-bit unsigned integer (main data bus)
typedef ap_uint<108> datau108b;  // Various bit-widths for different purposes
typedef ap_uint<64> datau64b;    // 64-bit for control/configuration
typedef ap_uint<32> datau32b;    // 32-bit for word operations
typedef ap_uint<8> datau8b;      // 8-bit for bytes (like A - DCI length)
typedef ap_uint<16> datau16b;
typedef ap_uint<17> datau17b;
typedef ap_uint<19> datau19b;
typedef ap_uint<240> datau240b;
typedef ap_uint<5> datau5b;      // 5-bit for loop counters (0-31)
typedef ap_uint<2> datau2b;      // 2-bit for small counters (0-3)
typedef ap_uint<6> datau6b;
typedef ap_uint<20> datau20b;
typedef ap_uint<25> datau25b;    // 25-bit for CRC operations
typedef ap_uint<30> datau30b;
typedef ap_uint<12> datau12b;
typedef ap_uint<7> datau7b;      // 7-bit for bit positions (0-127)
typedef ap_uint<24> datau24b;    // 24-bit for final CRC
typedef ap_uint<1> datau1b;      // 1-bit for boolean flags

// AXI Stream structure - standard for high-speed data transfer
struct data_axi
{
    ap_uint<128> data;  // The actual data payload (128 bits)
    ap_uint<1> last;    // 'last' signal - indicates end of packet
};

// Stream type definitions
typedef hls::stream<data_axi> dataStream;  // Stream of data packets
typedef hls::stream<datau64b> cnStream;    // Stream for control/config

// Function prototype
void crc_addition(dataStream &inData, cnStream &cnData, dataStream &outData);

#endif
