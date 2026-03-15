import binascii

def hex_to_bits(hex_str):
    """
    Convert hex string to list of bits (MSB first within each byte)
    Example: "0D" -> [0,0,0,0,1,1,0,1] (0x0D = 13 decimal)
    """
    # Remove any whitespace
    hex_str = hex_str.strip()
    
    # Convert hex to bytes
    bytes_data = binascii.unhexlify(hex_str)
    
    # Convert each byte to bits (MSB first)
    bits = []
    for byte in bytes_data:
        # Format as 8-bit binary with MSB first
        bin_str = format(byte, '08b')
        # Convert each char to int bit
        bits.extend([int(b) for b in bin_str])
    
    return bits

def bits_to_hex(bits):
    """
    Convert list of bits to hex string (grouping 8 bits into bytes)
    Assumes bits are in MSB order within each byte
    """
    # Pad to multiple of 8
    while len(bits) % 8 != 0:
        bits.append(0)
    
    # Convert each group of 8 bits to byte
    bytes_data = bytearray()
    for i in range(0, len(bits), 8):
        byte_bits = bits[i:i+8]
        # Convert bits to integer (MSB first)
        byte_val = 0
        for bit in byte_bits:
            byte_val = (byte_val << 1) | bit
        bytes_data.append(byte_val)
    
    # Convert to hex string
    return binascii.hexlify(bytes_data).decode().upper()

def bit_interleaver(input_hex, E, Qm):
    """
    3GPP TS 38.212 Clause 5.4.2.2 Bit Interleaving
    
    Args:
        input_hex: Hex string of input bits (full 96-bit words)
        E: Total number of coded bits (G)
        Qm: Modulation order (2,4,6,8)
    
    Returns:
        Hex string of interleaved output
    """
    
    print(f"\n--- Bit Interleaver ---")
    print(f"E = {E} bits, Qm = {Qm}")
    print(f"Input hex: {input_hex}")
    
    # Step 1: Convert input hex to bits
    all_bits = hex_to_bits(input_hex)
    print(f"Total input bits: {len(all_bits)}")
    
    # Only first E bits are valid (rest are padding)
    input_bits = all_bits[:E]
    print(f"Valid bits (first {E}): {len(input_bits)}")
    
    # Step 2: Calculate matrix dimensions
    num_rows = Qm
    num_cols = E // Qm  # Integer division (E is multiple of Qm)
    
    print(f"Matrix: {num_rows} rows × {num_cols} columns")
    
    # Step 3: Write bits into matrix ROW-WISE
    # Matrix representation: matrix[row][col]
    matrix = [[0 for _ in range(num_cols)] for _ in range(num_rows)]
    
    bit_idx = 0
    for row in range(num_rows):
        for col in range(num_cols):
            if bit_idx < len(input_bits):
                matrix[row][col] = input_bits[bit_idx]
                bit_idx += 1
    
    # Step 4: Read bits from matrix COLUMN-WISE
    output_bits = []
    for col in range(num_cols):
        for row in range(num_rows):
            output_bits.append(matrix[row][col])
    
    print(f"Output bits: {len(output_bits)}")
    
    # Step 5: Convert back to hex (with padding to multiple of 96 bits)
    # Calculate how many 96-bit words needed
    bits_per_word = 96
    words_needed = (E + bits_per_word - 1) // bits_per_word
    total_output_bits = words_needed * bits_per_word
    
    # Pad output bits with zeros to reach total_output_bits
    padded_output = output_bits + [0] * (total_output_bits - len(output_bits))
    
    # Convert to hex
    output_hex = bits_to_hex(padded_output)
    print(f"Output hex: {output_hex}")
    
    return output_hex

def test_case_1():
    """Test case 1: Qm=2, E=40"""
    print("\n" + "="*50)
    print("TEST CASE 1")
    print("="*50)
    
    input_hex = "0D23D97BB200000000000000"
    expected_output = "41B74D4FA600000000000000"
    E = 40
    Qm = 2
    
    output = bit_interleaver(input_hex, E, Qm)
    
    print(f"\nExpected: {expected_output}")
    print(f"Got:      {output}")
    
    if output == expected_output:
        print("✓ TEST PASSED")
    else:
        print("✗ TEST FAILED")
    
    return output == expected_output

def test_case_2():
    """Test case 2: Qm=2, E=672"""
    print("\n" + "="*50)
    print("TEST CASE 2")
    print("="*50)
    
    # Test case 2 input (7 lines of 12 bytes each)
    input_lines = [
        "C40ACEC578810F615DA292DF",
        "5871F1298E2B85C43324F767",
        "87CA3948FF0BD5346A6D87E8",
        "5819BB12DE91E7F3CDA99733",
        "617F753CE828916B41E798F6",
        "1500561492E198BE1E76B9A7",
        "60D4974E724A6F011C4D1608"
    ]
    input_hex = "".join(input_lines)
    
    # Expected output (7 lines)
    expected_lines = [
        "F435558DF0F9E4636B958507",
        "14AB3D5737B38D58D648A6EA",
        "63813E47BA035C97C1E85D9E",
        "8133A0201B1E0930EB2E7C2B",
        "C16AE5DC0BD63594EFEB449F",
        "B6225B30699D38F6952EB8C4",
        "36D502838BDA1259A3BC8242"
    ]
    expected_output = "".join(expected_lines)
    
    E = 672
    Qm = 2
    
    output = bit_interleaver(input_hex, E, Qm)
    
    print(f"\nExpected length: {len(expected_output)}")
    print(f"Got length:      {len(output)}")
    
    # Compare first few bytes
    print(f"\nFirst 24 bytes comparison:")
    print(f"Expected: {expected_output[:48]}")
    print(f"Got:      {output[:48]}")
    
    if output == expected_output:
        print("\n✓ TEST PASSED")
    else:
        print("\n✗ TEST FAILED")
        # Find first mismatch
        for i in range(min(len(expected_output), len(output))):
            if expected_output[i] != output[i]:
                print(f"First mismatch at position {i}: expected '{expected_output[i]}', got '{output[i]}'")
                break
    
    return output == expected_output

def verify_bit_ordering():
    """Verify bit ordering with a simple example"""
    print("\n" + "="*50)
    print("BIT ORDERING VERIFICATION")
    print("="*50)
    
    # Test with a single byte 0x0D = 00001101 (MSB first)
    test_hex = "0D"
    bits = hex_to_bits(test_hex)
    print(f"Hex {test_hex} -> bits (MSB first): {bits}")
    
    # Convert back
    back_to_hex = bits_to_hex(bits)
    print(f"Bits -> hex: {back_to_hex}")
    
    if back_to_hex == test_hex:
        print("✓ Bit ordering consistent")
    else:
        print("✗ Bit ordering issue")

def test_all_cases():
    """Run all test cases from the problem statement"""
    
    # Test cases: (E, Qm, input_file_lines_count)
    test_cases = [
        (40, 2, 1),    # Test case 1: 1 line
        (672, 2, 7),   # Test case 2: 7 lines
        (1360, 2, 15), # Test case 3: 15 lines
        (7200, 2, 75), # Test case 4: 75 lines
        (8400, 4, 88), # Test case 5: 88 lines
        (9840, 4, 103),# Test case 6: 103 lines
        (12888, 4, 135),# Test case 7: 135 lines
        (13120, 4, 137),# Test case 8: 137 lines
        (9258, 6, 97), # Test case 9: 97 lines
        (9336, 6, 98), # Test case 10: 98 lines
        (9582, 6, 100),# Test case 11: 100 lines
        (10800, 6, 113),# Test case 12: 113 lines
        (9496, 8, 99), # Test case 13: 99 lines
        (10000, 8, 105) # Test case 14: 105 lines
    ]
    
    print("\n" + "="*50)
    print("RUNNING ALL TEST CASES")
    print("="*50)
    
    for i, (E, Qm, num_lines) in enumerate(test_cases, 1):
        print(f"\n--- Test Case {i} (E={E}, Qm={Qm}) ---")
        
        # Generate dummy input for demonstration
        # In reality, you'd read from actual files
        # Here we just note the parameters
        print(f"Would process {num_lines} lines of 12 bytes each")
        print(f"Total bits: {E}, Words needed: {(E+95)//96}")
        print(f"Matrix: {Qm} rows × {E//Qm} columns")

if __name__ == "__main__":
    # Verify bit ordering first
    verify_bit_ordering()
    
    # Run test case 1
    test_case_1()
    
    # Run test case 2
    test_case_2()
    
    # Show all test case parameters
    test_all_cases()
    
    print("\n" + "="*50)
    print("ALGORITHM EXPLANATION")
    print("="*50)
    print("""
    3GPP Bit Interleaving Algorithm (TS 38.212 Clause 5.4.2.2):
    
    1. Input: E bits = [e0, e1, ..., e(E-1)]
    
    2. Arrange into matrix with Qm rows and E/Qm columns
    
    3. Write bits ROW-WISE:
       Row 0: e0, e1, ..., e(E/Qm-1)
       Row 1: e(E/Qm), e(E/Qm+1), ..., e(2*E/Qm-1)
       etc.
    
    4. Read bits COLUMN-WISE:
       First column (top to bottom): e0, e(E/Qm), e(2*E/Qm), ...
       Second column: e1, e(E/Qm+1), e(2*E/Qm+1), ...
    
    5. Output: f = [e0, e(E/Qm), e(2*E/Qm), ..., e1, e(E/Qm+1), ...]
    
    For test case 1:
    - E=40 bits, Qm=2
    - Matrix: 2 rows × 20 columns
    - Input (first 40 bits): from hex "0D23D97BB2..."
    - Output: column-wise reading → matches "41B74D4FA6..."
    """)