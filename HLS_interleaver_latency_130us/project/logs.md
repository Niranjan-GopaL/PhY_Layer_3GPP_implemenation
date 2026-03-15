## FIX ONE

The issue is with bit-level handling and byte alignment. 
The test expects 12 bytes (96 bits) per word, 
but we're getting fewer bytes because we're not properly handling the last word when E is not a multiple of 96 bits.


Let me analyze test case 1:

E=40 bits = 5 bytes

Input: "0D23D97BB200000000000000" (12 bytes, but only first 5 bytes are valid? Actually it's 12 bytes - 96 bits)
Output: "41B74D4FA600000000000000" (12 bytes)

The key insight: 
The input files contain full 96-bit words (12 bytes) for each line, even when E < 96. 
The extra bytes are padding (zeros in test case 1). 
We need to process all 12 bytes per word, 
but only consider the first E bits for interleaving.


## FIX TWO

The key fix:
Bit reversal within each byte: The 3GPP spec likely expects MSB-first ordering within bytes, 
but the AXI-Stream interface provides LSB-first. 
We need to reverse bits within each byte on both input and output.

This explains why test case 8 had all zeros in our output - the bit ordering was completely wrong.