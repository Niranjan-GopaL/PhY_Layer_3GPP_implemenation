#include "interleaver.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cstdint>

using namespace std;

// Helper function to convert hex string to byte array
vector<uint8_t> hexStringToBytes(const string& hex) {
    vector<uint8_t> bytes;
    string hexStr = hex;
    
    // Remove any whitespace
    hexStr.erase(remove_if(hexStr.begin(), hexStr.end(), ::isspace), hexStr.end());
    
    // Each line should have 24 hex chars = 12 bytes
    for (size_t i = 0; i < hexStr.length(); i += 2) {
        string byteString = hexStr.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    
    return bytes;
}

// Helper function to convert bytes to hex string
string bytesToHexString(const vector<uint8_t>& bytes) {
    stringstream ss;
    ss << hex << uppercase << setfill('0');
    for (size_t i = 0; i < bytes.size(); i++) {
        ss << setw(2) << (int)bytes[i];
    }
    return ss.str();
}

// Test case configuration
struct TestCase {
    int id;
    int Qm;
    int E;  // G in the table is the number of bits (E)
    string inFile;
    string outFile;
};

// Test cases from problem statement
TestCase testCases[] = {
    {1, 2, 40, "test_case_1_in", "test_case_1_out"},
    {2, 2, 672, "test_case_2_in", "test_case_2_out"},
    {3, 2, 1360, "test_case_3_in", "test_case_3_out"},
    {4, 2, 7200, "test_case_4_in", "test_case_4_out"},
    {5, 4, 8400, "test_case_5_in", "test_case_5_out"},
    {6, 4, 9840, "test_case_6_in", "test_case_6_out"},
    {7, 4, 12888, "test_case_7_in", "test_case_7_out"},
    {8, 4, 13120, "test_case_8_in", "test_case_8_out"},
    {9, 6, 9258, "test_case_9_in", "test_case_9_out"},
    {10, 6, 9336, "test_case_10_in", "test_case_10_out"},
    {11, 6, 9582, "test_case_11_in", "test_case_11_out"},
    {12, 6, 10800, "test_case_12_in", "test_case_12_out"},
    {13, 8, 9496, "test_case_13_in", "test_case_13_out"},
    {14, 8, 10000, "test_case_14_in", "test_case_14_out"}
};

int main() {
    int passedTests = 0;
    int totalTests = 14;
    
    cout << "Starting 5G-NR Bit Interleaver Testbench" << endl;
    cout << "========================================" << endl;
    
    for (int t = 0; t < totalTests; t++) {
        TestCase& tc = testCases[t];
        
        cout << "\nRunning Test Case " << tc.id << " (Qm=" << tc.Qm 
             << ", E=" << tc.E << ")..." << endl;
        
        // Read input file
        ifstream inFile(tc.inFile);
        if (!inFile.is_open()) {
            cerr << "Error: Cannot open " << tc.inFile << endl;
            continue;
        }
        
        string line;
        vector<uint8_t> inputBytes;
        int lineCount = 0;
        
        while (getline(inFile, line)) {
            if (line.empty()) continue;
            lineCount++;
            vector<uint8_t> lineBytes = hexStringToBytes(line);
            // Each line should be exactly 12 bytes (96 bits)
            if (lineBytes.size() != 12) {
                cout << "  Warning: Line " << lineCount << " has " << lineBytes.size() 
                     << " bytes, expected 12" << endl;
            }
            inputBytes.insert(inputBytes.end(), lineBytes.begin(), lineBytes.end());
        }
        inFile.close();
        
        // Read expected output file
        ifstream outFile(tc.outFile);
        if (!outFile.is_open()) {
            cerr << "Error: Cannot open " << tc.outFile << endl;
            continue;
        }
        
        vector<uint8_t> expectedBytes;
        lineCount = 0;
        
        while (getline(outFile, line)) {
            if (line.empty()) continue;
            lineCount++;
            vector<uint8_t> lineBytes = hexStringToBytes(line);
            if (lineBytes.size() != 12) {
                cout << "  Warning: Output line " << lineCount << " has " << lineBytes.size() 
                     << " bytes, expected 12" << endl;
            }
            expectedBytes.insert(expectedBytes.end(), lineBytes.begin(), lineBytes.end());
        }
        outFile.close();
        
        // Calculate number of 96-bit words
        int bitsPerWord = 96;
        int bytesPerWord = 12;
        int wordsNeeded = (tc.E + bitsPerWord - 1) / bitsPerWord;
        int expectedBytesTotal = wordsNeeded * bytesPerWord;
        
        cout << "  Input lines: " << inputBytes.size() / 12 << ", Total input bytes: " << inputBytes.size() << endl;
        cout << "  Expected lines: " << expectedBytes.size() / 12 << ", Total expected bytes: " << expectedBytes.size() << endl;
        cout << "  Words needed (96-bit): " << wordsNeeded << endl;
        
        // Prepare input AXI-Stream
        axis_stream_t inStream("input_stream");
        
        // Pack input bytes into 96-bit words
        for (int w = 0; w < wordsNeeded; w++) {
            axis_data_t inWord;
            inWord.data = 0;
            inWord.last = (w == wordsNeeded - 1) ? 1 : 0;
            inWord.keep = -1;  // All bytes valid
            inWord.strb = -1;
            
            // Fill 96-bit word (12 bytes)
            for (int b = 0; b < 12; b++) {
                int byteIdx = w * 12 + b;
                if (byteIdx < (int)inputBytes.size()) {
                    inWord.data.range(b*8 + 7, b*8) = inputBytes[byteIdx];
                }
            }
            
            inStream.write(inWord);
        }
        
        // Prepare control word
        control_word_t cnData = 0;
        cnData.range(14, 0) = tc.E;      // E in bits [14:0]
        cnData.range(18, 15) = tc.Qm;     // Qm in bits [18:15]
        
        // Create output stream
        axis_stream_t outStream("output_stream");
        
        // Call the interleaver
        Interleaver(inStream, outStream, cnData);
        
        // Collect output
        vector<uint8_t> producedBytes;
        int wordCount = 0;
        
        while (!outStream.empty()) {
            axis_data_t outWord = outStream.read();
            
            // Extract all 12 bytes from each word
            for (int b = 0; b < 12; b++) {
                uint8_t byte = outWord.data.range(b*8 + 7, b*8);
                producedBytes.push_back(byte);
            }
            
            wordCount++;
            
            if (outWord.last) {
                break;
            }
        }
        
        cout << "  Produced lines: " << producedBytes.size() / 12 << ", Total produced bytes: " << producedBytes.size() << endl;
        
        // Write produced output to file
        string producedFile = "/home/nira/Documents/code/ece/HLS/HLS_Mini_project/out/test_case_" + to_string(tc.id) + "_output_produced";
        ofstream producedOut(producedFile);
        if (producedOut.is_open()) {
            for (size_t i = 0; i < producedBytes.size(); i += 12) {
                vector<uint8_t> lineBytes(producedBytes.begin() + i, 
                                          producedBytes.begin() + min(i+12, producedBytes.size()));
                producedOut << bytesToHexString(lineBytes) << endl;
            }
            producedOut.close();
            cout << "  Produced output written to: " << producedFile << endl;
        }
        
        // Compare with expected
        bool testPassed = true;
        if (producedBytes.size() != expectedBytes.size()) {
            cout << "  Size mismatch: produced=" << producedBytes.size() 
                 << ", expected=" << expectedBytes.size() << endl;
            testPassed = false;
        } else {
            int mismatchCount = 0;
            for (size_t i = 0; i < expectedBytes.size(); i++) {
                if (producedBytes[i] != expectedBytes[i]) {
                    if (mismatchCount < 20) {
                        int wordIdx = i / 12;
                        int byteInWord = i % 12;
                        cout << "  Mismatch at word " << wordIdx << ", byte " << byteInWord 
                             << " (global byte " << i << "): produced=0x" 
                             << hex << setw(2) << setfill('0') << (int)producedBytes[i] 
                             << ", expected=0x" << (int)expectedBytes[i] << dec << endl;
                    }
                    mismatchCount++;
                    testPassed = false;
                }
            }
            if (mismatchCount > 0) {
                cout << "  Total mismatches: " << mismatchCount << endl;
            }
        }
        
        if (testPassed) {
            cout << "  Test Case " << tc.id << " PASSED" << endl;
            passedTests++;
        } else {
            cout << "  Test Case " << tc.id << " FAILED" << endl;
        }
    }
    
    cout << "\n========================================" << endl;
    cout << "Summary: " << passedTests << "/" << totalTests << " tests passed." << endl;
    cout << "========================================" << endl;
    
    if (passedTests == totalTests) {
        cout << "All tests passed!" << endl;
        return 0;
    } else {
        cout << "Some tests failed." << endl;
        return 1;
    }
}
