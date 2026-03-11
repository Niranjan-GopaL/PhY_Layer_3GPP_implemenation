

#include "header.h"


void crc_addition(dataStream &inData, cnStream &cnData, dataStream &outData){
    #pragma HLS INTERFACE axis port=inData
    #pragma HLS INTERFACE axis port=outData
    #pragma HLS INTERFACE axis port=cnData
    #pragma HLS interface ap_ctrl_none port=return

    // Defining number of input and output variables
	data_axi input;
    data_axi output;

	datau8b A = ??;  // read DCI Length (A) from cnData port

    datau25b CRC_POLY_24 = 28487959;  // CRC polynomial (CRC24C) in decimal

    // Calculating the number of bits in last burst
    datau2b num_bursts = ??;  // calculate the number of 128-bit bursts
    datau7b bits_in_last_burst = ??;  // number of bits in last burst
    if (bits_in_last_burst == 0) bits_in_last_burst = 128;

    datau25b crc_temp = ??; // Initialize the register with all 1's

    // Finding the CRC of DCI
    datau128b last_burst_data = 0;
    datau8b total_bits_processed = 0;

    LOOP_2: for (datau2b i = 0; i < num_bursts; i++) {
        input = ;  // read from input port
        datau128b temp = ;  // access the data part
        last_burst_data = temp;
        output.data = ;  // write to output variable

        LOOP_2_1: for (int j = 127; j >= 0; j--) {
            if (total_bits_processed < A) {
                crc_temp = ;  // shifting the register
                crc_temp[0] = ; // take the next bit
                if(crc_temp[24] == 1) {
                    crc_temp = ;  // xor with polynomial
                }
                total_bits_processed++;
            }
        }

        // asserting the last signal
        if (i < num_bursts - 1) {
            output.last = 0;
            outData.write(output);  //writing to the output port
        }
    }

    // Finding the CRC of 24 0's
    LOOP_3: for (datau5b i = 0; i < 24; i++) {
        crc_temp = ;  // shifting the register
        crc_temp[0] = ; // take the next bit
        if(crc_temp[24] == 1) {
            crc_temp = ;  // xor with polynomial
        }
    }

    datau24b final_crc = ; // get final 24-bit CRC from reminder

    datau7b available_bits = ??; // Calculate the number position available

    // Writing the last set of output
    if (available_bits >= 24) {
    	datau7b shift_amount = ??;  // Calculate the shift amount
        output.data = last_burst_data | ((datau128b)final_crc << shift_amount);
        output.last = 1;
        outData.write(output);
    }
    else {
    	// Calculate number of bits that have to be written in next burst
    	datau7b bits_for_next = ??;
        output.data = last_burst_data | ((datau128b)final_crc >> bits_for_next);
        output.last = 0;
        outData.write(output);

        output.data = 0;
        output.data.range(127, 128 - bits_for_next) = final_crc.range(bits_for_next - 1, 0);
        output.last = 1;
        outData.write(output);
    }
}
