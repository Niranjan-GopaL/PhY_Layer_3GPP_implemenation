#ifndef HEADER_H
#define HEADER_H

#include <hls_stream.h> // myabe has axi lib functions inside this ???
#include <fstream>
#include <iostream>
#include <hls_math.h>
#include <ap_int.h>
#include <stdio.h>


using namespace std;

typedef ap_uint<8> datau8b;
typedef ap_uint<32> datau32b;
typedef ap_uint<40> datau40b;


struct in_data_axi
{
	ap_uint<32> data;
	ap_uint<1> last;
};


struct out_data_axi
{
	ap_uint<40> data;  // if we add 2 in_data,i.e 32bit => 33bit => 40bit(nearest byte)
	ap_uint<1> last;
};


/*
 * t_data
 * [ANY n-bit user defined
 * (AXI ROUNDS TO THE NEXT byte-multiple) data]
 * Eg : if data is 50 bits, then AXI_Stream will SYNTHESIZE to 56 bit width HARDWARE
 *
 *
 * t_last
 * t_valid
 * t_ready [ bit that will be set FOR THE LAST ]
 *
 * */



// defining the stream interface
typedef hls::stream<in_data_axi> inStream;
typedef hls::stream<out_data_axi> outStream;
typedef hls::stream<datau8b> cnStream; // control/Config interface

void ADDER(inStream &inData_A, inStream &inData_B, cnStream &cnData, outStream &outData_Y);
#endif












