#include "header.h"

int main()
{
    // SECTION 1: INITIALIZE STREAMS
    // These mimic FIFOs connecting to/from your module
    dataStream inDataFIFO;   // Input data stream
    cnStream cnDataFIFO;     // Control stream
    dataStream outDataFIFO;  // Output data stream

    // Variables for single transactions
    datau64b cndata;      // Control data
    data_axi indata;       // Input packet
    data_axi outdata;      // Output packet

    // SECTION 2: TEST PARAMETERS
    // [DCI length, ... other params not used in this test]
    int parameters[10][5] =
    {
        {23,128,108,17,120},   // Test 1: 23-bit DCI
        {70,256,216,0,0},      // Test 2: 70-bit DCI
        {12,512,432,10698,62289}, // Test 3: 12-bit DCI
        {24,128,108,14343,14343}, // Test 4: 24-bit DCI
        {71,256,216,23456,62351}, // Test 5: 71-bit DCI
        {128,256,216,43748,55858}, // Test 6: 128-bit DCI (exactly one burst)
        {120,512,864,1007,0},  // Test 7: 120-bit DCI
        {115,512,1728,2500,4111}, // Test 8: 115-bit DCI
        {140,512,1728,65535,65519}, // Test 9: 140-bit DCI
        {135,512,864,25258,34576}  // Test 10: 135-bit DCI
    };

    // SECTION 3: RUN ALL TESTS
    for(int test=1;test<=10;test++){

        // SECTION 3.1: WRITE CONTROL DATA
        datau8b A = parameters[test-1][0];  // Get DCI length
        cndata.range(7,0)=A;                 // Put in 8 LSBs
        cnDataFIFO.write(cndata);             // Send to DUT

        // SECTION 3.2: CALCULATE OUTPUT SIZE
        datau8b K = A+24;  // Total bits after adding CRC
        int burst = ceil((float)A/128);  // Input bursts needed

        // SECTION 3.3: READ INPUT TEST VECTOR
        string inFileName = "test_case_"+to_string(test)+"_in";
        ifstream input_file_data(inFileName);
        datau128b dataTemp;

        int count = burst;
        while (count>0)
        {
            // Read 32 hex characters (128 bits) from file
            char buffer[32];
            input_file_data.read(buffer, 32);

            // Convert ASCII hex to binary
            string s = "0x";
            for (int j = 0; j < 32 ; j++)
            {
                s += buffer[j];
            }
            istringstream iss(s);
            iss >> hex >> dataTemp;  // Parse hex string

            // Create AXI packet
            indata.data = dataTemp.read();
            indata.last = (burst==0) ? 1 : 0;  // Set 'last' appropriately

            // Write to input stream
            inDataFIFO.write(indata);

            // Skip newline character
            char trailing[1];
            input_file_data.read(trailing,1);
            count--;
        }
        input_file_data.close();

        // SECTION 3.4: CALL YOUR MODULE
        crc_addition(inDataFIFO, cnDataFIFO, outDataFIFO);

        // SECTION 3.5: WRITE OUTPUT TO FILE
        ofstream output_file("output_data");
        datau128b outTemp;
        count = ceil((float)K/128);  // Output bursts needed

        while(count>0){
            outDataFIFO >> outdata;  // Read from output stream
            outTemp = outdata.data.read();

            // Convert to hex string (32 chars)
            string s = "";
            for(int j=3;j>=0;j--){  // Process 4 32-bit words
                datau32b readTemp;
                stringstream ss;
                readTemp = outTemp.range(32*j+31,32*j);
                unsigned int u = readTemp.read();
                ss << std::setfill('0') << std::setw(8)
                   << uppercase << hex << u;
                s += ss.str();
            }
            output_file << s << endl;
            count--;
        }
        output_file.close();

        // SECTION 3.6: VERIFY RESULTS
        string cmd = "diff -w output_data test_case_"+to_string(test)+"_out_crc";
        if (system(cmd.c_str())){
            printf("Test failed !!!\nOutput data does not match");
            return 1;
        } else {
            cout<<"Test "<<test<<" passed !\n";
        }
    }
    return 0;
}
