#include "interleaver.h"



// For spec bit k stored in ap_uint<96> words:
// Byte number within word: k / 8 (but capped to word boundary)
// Bit within byte (MSB-first): 7 - (k % 8)
// ap_uint bit index: byte_in_word * 8 + (7 - bit_in_byte)
//                  = (k % 96 / 8) * 8 + 7 - (k % 96 % 8)

inline ap_uint<7> spec_to_apuint_bit(ap_uint<7> pos_in_word) {
    // pos_in_word is the spec's 0-based bit position within the 96-bit word
    ap_uint<4> byte_idx = pos_in_word / 8;      // which byte (0..11)
    ap_uint<3> bit_in_byte = pos_in_word % 8;   // 0=MSB in spec
    return byte_idx * 8 + (7 - bit_in_byte);    // ap_uint LSB=0
}

#include "interleaver.h"

// Translate spec bit position within a 96-bit word (MSB-first per byte)
// to ap_uint<96> bit index (LSB=0).
// e.g. spec bit 0 → ap_uint bit 7 (MSB of byte 0)
//      spec bit 7 → ap_uint bit 0 (LSB of byte 0)
//      spec bit 8 → ap_uint bit 15 (MSB of byte 1)
static ap_uint<7> spec_to_ap(ap_uint<7> p) {
#pragma HLS INLINE
    return (p / 8) * 8 + (7 - (p % 8));
}

void Interleaver(
    axis_stream_t& inData,
    axis_stream_t& outData,
    control_word_t cnData
) {
    #pragma HLS INTERFACE axis port=inData
    #pragma HLS INTERFACE axis port=outData
    #pragma HLS INTERFACE s_axilite port=cnData bundle=control
    #pragma HLS INTERFACE ap_ctrl_chain port=return bundle=control

    ap_uint<15> E      = cnData.range(14, 0);
    ap_uint<4>  Qm     = cnData.range(18, 15);
    ap_uint<15> numCols = E / Qm;
    int wordsIn        = (E + 95) / 96;

    ap_uint<96> inBuf[MAX_WORDS];
    #pragma HLS RESOURCE variable=inBuf core=RAM_1P_BRAM
    #pragma HLS DEPENDENCE variable=inBuf inter false

    // =========================================================
    // STAGE 1: Read all input words into BRAM
    // =========================================================
    READ_LOOP: for (int w = 0; w < wordsIn; w++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_TRIPCOUNT min=1 max=137
        inBuf[w] = inData.read().data;
    }

    // =========================================================
    // STAGE 2+3: For each output bit position k (spec order),
    // compute source bit, extract it, pack into output word.
    //
    // Interleave mapping (Algorithm 1):
    //   output bit k  <-  input bit src_flat
    //   where src_flat = (k % Qm) * numCols + (k / Qm)
    //
    // Both k and src_flat are "spec" bit indices (MSB-first per byte).
    // We translate to ap_uint indices via spec_to_ap().
    // =========================================================
    ap_uint<96> outWord = 0;
    INTERLEAVE_WRITE: for (ap_uint<15> k = 0; k < E; k++) {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_TRIPCOUNT min=40 max=13120

        // Source bit in spec ordering
        ap_uint<15> src_flat = (k % Qm) * numCols + (k / Qm);

        // Break into word index and bit-within-word (spec ordering)
        ap_uint<8>  src_word     = src_flat / 96;
        ap_uint<7>  src_bit_spec = src_flat % 96;

        // Translate to ap_uint bit index (byte-reversed within byte)
        ap_uint<7> src_bit_ap = spec_to_ap(src_bit_spec);

        // Extract the bit
        ap_uint<1> b = inBuf[src_word][src_bit_ap];

        // Place into output word at the correct ap_uint position
        ap_uint<7> dst_bit_spec = k % 96;
        ap_uint<7> dst_bit_ap   = spec_to_ap(dst_bit_spec);
        outWord[dst_bit_ap] = b;

        // Emit word when full or at last bit
        bool last_in_word = (dst_bit_spec == 95);
        bool last_of_all  = (k == (ap_uint<15>)(E - 1));

        if (last_in_word || last_of_all) {
            axis_data_t outW;
            outW.data = outWord;
            outW.last = last_of_all ? 1 : 0;
            outW.keep = -1;
            outW.strb = -1;
            outData.write(outW);
            outWord = 0;
        }
    }
}
