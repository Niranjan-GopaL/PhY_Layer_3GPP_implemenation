#include "header.h"

void crc_addition(dataStream &inData, cnStream &cnData, dataStream &outData){
    // HLS Directives - tell the synthesizer how to implement interfaces
    #pragma HLS INTERFACE axis port=inData   // inData is AXI4-Stream interface
    #pragma HLS INTERFACE axis port=outData  // outData is AXI4-Stream interface
    #pragma HLS INTERFACE axis port=cnData   // cnData is AXI4-Stream interface
    #pragma HLS interface ap_ctrl_none port=return  // No control registers, pure streaming

    // SECTION 1: READ CONFIGURATION
    // Read the DCI length (A) from control stream (8 bits, 0-255 bits)
    datau8b A = cnData.read().range(7,0);  // DCI Length in bits

    // SECTION 2: DEFINE CRC PARAMETERS
    // 3GPP CRC24C polynomial: 0x1B2B3B (28487959 in decimal)
    // This is the divisor in binary polynomial division
    datau25b CRC_POLY_24 = 28487959;

    // Variables to hold input and output data packets
    data_axi input;
    data_axi output;

    // SECTION 3: CALCULATE BURST INFORMATION
    // Number of 128-bit transfers needed to send A bits
    // ceil(A/128) - because we might need a partial last burst
    datau2b num_bursts = ceil((float)A/128);

    // Number of valid bits in the last burst
    datau7b bits_in_last_burst = A % 128;
    if (bits_in_last_burst == 0) bits_in_last_burst = 128;  // If exact multiple

    // SECTION 4: INITIALIZE CRC CALCULATION
    // Initial CRC value (all 1's as per 3GPP spec)
    datau25b crc_temp = 0x1FFFFFF;  // 25 bits of 1 (bit 24 is MSB for division)

    // Tracking variables
    datau128b last_burst_data = 0;  // Store last burst for later CRC insertion
    datau8b total_bits_processed = 0;  // Count bits processed so far

    // SECTION 5: PROCESS ALL INPUT BURSTS
    LOOP_2: for (datau2b i = 0; i < num_bursts; i++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=2  // Help HLS estimate loop iterations

        // Read one 128-bit packet from input stream
        input = inData.read();
        datau128b temp = input.data;  // Current burst data
        last_burst_data = temp;        // Save last burst for later
        output.data = temp;             // Pass through data initially

        // SECTION 5.1: PROCESS EACH BIT IN THE BURST (MSB first)
        LOOP_2_1: for (int j = 127; j >= 0; j--) {
            #pragma HLS UNROLL factor=4  // Process 4 bits in parallel for speed

            // Only process bits that are part of the DCI
            if (total_bits_processed < A) {
                // CRC ALGORITHM: Binary polynomial division
                // Step 1: Shift CRC left by 1 (multiply by x)
                crc_temp *= 2;

                // Step 2: Bring in next data bit as LSB
                crc_temp[0] = temp[j];

                // Step 3: If MSB (bit 24) is 1, XOR with polynomial
                // This is the division step (subtract in GF2)
                if(crc_temp[24] == 1) {
                    crc_temp = crc_temp ^ CRC_POLY_24;
                }
                total_bits_processed++;
            }
        }

        // SECTION 5.2: WRITE OUTPUT (except last burst)
        if (i < num_bursts - 1) {
            output.last = 0;  // Not the last packet
            outData.write(output);  // Forward data unchanged
        }
    }

    // SECTION 6: PROCESS 24 ZERO BITS (CRC padding)
    // This simulates appending 24 zeros after the data
    LOOP_3: for (datau5b i = 0; i < 24; i++) {
        #pragma HLS UNROLL factor=4  // Parallel processing

        crc_temp *= 2;     // Shift left
        crc_temp[0] = 0;    // Bring in 0 (the appended bit)

        // XOR with polynomial if MSB is 1
        if(crc_temp[24] == 1) {
            crc_temp = crc_temp ^ CRC_POLY_24;
        }
    }

    // Extract the final 24-bit CRC (bits 23 down to 0)
    datau24b final_crc = crc_temp.range(23, 0); // get CRC

    // SECTION 7: INSERT CRC INTO OUTPUT STREAM
    datau7b available_bits = 128 - bits_in_last_burst;  // Free bits in last burst

    // SECTION 7.1: CASE 1 - CRC fits entirely in last burst
    if (available_bits >= 24) {
        // Shift CRC to align with free bits at the end
        datau7b shift_amount = available_bits - 24;
        // XOR the CRC into the free bits (preserve original data bits)
        output.data = last_burst_data ^ ((datau128b)final_crc << shift_amount);
        output.last = 1;  // This is the last packet
        outData.write(output);
    }
    // SECTION 7.2: CASE 2 - CRC spans across two bursts
    else {
        // Part 1: Put as much CRC as possible in last burst
        datau7b bits_for_next = 24 - available_bits;
        // Shift CRC right to put higher bits at the end of current burst
        output.data = last_burst_data ^ ((datau128b)final_crc >> bits_for_next);
        output.last = 0;  // More data coming
        outData.write(output);

        // Part 2: Create new burst with remaining CRC bits
        output.data = 0;  // Start with zeros
        // Place remaining CRC bits at the beginning of new burst
        output.data.range(127, 128 - bits_for_next) = final_crc.range(bits_for_next - 1, 0);
        output.last = 1;  // This is the last packet
        outData.write(output);
    }
}
