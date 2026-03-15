#include "interleaver.h"
#include <hls_math.h>
#include <cassert>

void Interleaver(
    axis_stream_t& inData,
    axis_stream_t& outData,
    control_word_t cnData
) {
    #pragma HLS INTERFACE axis port=inData
    #pragma HLS INTERFACE axis port=outData
    #pragma HLS INTERFACE s_axilite port=cnData bundle=control
    #pragma HLS INTERFACE ap_ctrl_chain port=return bundle=control
    
    // Extract parameters from control word
    ap_uint<15> E = cnData.range(14, 0);      // Bits [14:0]
    ap_uint<4> Qm = cnData.range(18, 15);     // Bits [18:15]
    
    // Validate: E must be divisible by Qm
    assert((E % Qm) == 0 && "E must be divisible by Qm");
    
    const int numCols = E / Qm;
    const int numRows = Qm;
    
    // Calculate number of 96-bit input words needed
    const int bitsPerWord = 96;
    const int bytesPerWord = 12;
    const int wordsIn = (E + bitsPerWord - 1) / bitsPerWord; 
    
    // Buffer to hold all input bits
    ap_uint<1> bitBuffer[MAX_ROWS * MAX_COLS];
    #pragma HLS RESOURCE variable=bitBuffer core=RAM_2P_BRAM
    
    // Buffer to hold all output bits
    ap_uint<1> outBuffer[MAX_ROWS * MAX_COLS];
    #pragma HLS RESOURCE variable=outBuffer core=RAM_2P_BRAM
    
    // Step 1: Read input data and pack into bitBuffer
    int bitPos = 0;
    
    READ_LOOP: for (int w = 0; w < wordsIn; w++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_TRIPCOUNT min=1 max=137
        
        axis_data_t inDataWord = inData.read();
        ap_uint<96> inputWord = inDataWord.data;
        
        // Process each byte and reverse bits within the byte
        ap_uint<96> reversedWord = 0;
        for (int byte = 0; byte < bytesPerWord; byte++) {
            #pragma HLS UNROLL
            ap_uint<8> originalByte = inputWord.range(byte*8 + 7, byte*8);
            ap_uint<8> reversedByte = 0;
            
            // Reverse bits within the byte
            for (int bit = 0; bit < 8; bit++) {
                #pragma HLS UNROLL
                reversedByte[bit] = originalByte[7 - bit];
            }
            
            reversedWord.range(byte*8 + 7, byte*8) = reversedByte;
        }
        
        // Pack all 96 bits into buffer, but only up to E bits
        for (int b = 0; b < bitsPerWord; b++) {
            #pragma HLS UNROLL
            if (bitPos < E) {
                bitBuffer[bitPos++] = reversedWord[b];
            } else {
                bitPos++; // Skip padding
            }
        }
    }
    
    // Step 2: Interleave using Algorithm 1
    // Initialize outBuffer
    INIT_OUT: for (int i = 0; i < E; i++) {
        #pragma HLS PIPELINE II=1
        outBuffer[i] = 0;
    }
    
    // Perform interleaving
    INTERLEAVE_COL: for (int j = 0; j < numCols; j++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_TRIPCOUNT min=1 max=3280
        
        INTERLEAVE_ROW: for (int i = 0; i < numRows; i++) {
            #pragma HLS UNROLL
            int srcIdx = i * numCols + j;
            int dstIdx = i + j * numRows;
            
            if (srcIdx < E && dstIdx < E) {
                outBuffer[dstIdx] = bitBuffer[srcIdx];
            }
        }
    }
    
    // Step 3: Write output data
    int outBitPos = 0;
    
    WRITE_LOOP: for (int w = 0; w < wordsIn; w++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_TRIPCOUNT min=1 max=137
        
        axis_data_t outDataWord;
        outDataWord.data = 0;
        outDataWord.last = (w == wordsIn - 1) ? 1 : 0;
        outDataWord.keep = -1;
        outDataWord.strb = -1;
        
        // Assemble output word
        ap_uint<96> assembledWord = 0;
        
        for (int b = 0; b < bitsPerWord; b++) {
            #pragma HLS UNROLL
            int currentBit = w * bitsPerWord + b;
            if (currentBit < E) {
                assembledWord[b] = outBuffer[currentBit];
            }
        }
        
        // Reverse bits within each byte for output
        for (int byte = 0; byte < bytesPerWord; byte++) {
            #pragma HLS UNROLL
            ap_uint<8> originalByte = assembledWord.range(byte*8 + 7, byte*8);
            ap_uint<8> reversedByte = 0;
            
            for (int bit = 0; bit < 8; bit++) {
                #pragma HLS UNROLL
                reversedByte[7 - bit] = originalByte[bit];
            }
            
            outDataWord.data.range(byte*8 + 7, byte*8) = reversedByte;
        }
        
        outData.write(outDataWord);
    }
}