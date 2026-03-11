#include "header.h"

int main()
{
	//Initializing input and output stream variables that are passed to function
	dataStream inDataFIFO;
	cnStream cnDataFIFO;
	dataStream outDataFIFO;

	//Initializing input and output variables
	datau64b cndata;
	data_axi outdata;
	data_axi indata;

	// Writing the configuration information into stream interface
	datau8b A = 23;
	cndata.range(7,0)=A;
	cnDataFIFO.write(cndata);

	datau8b K = A+24;
	int burst=  ceil((float)A/128); // calculating the number of bursts
	int count;

	// Reading the data from input test vector file and writing to the input stream interface
	string inFileName = "test_case_1_in"; //file name
	ifstream input_file_data(inFileName); // opening the file
	datau128b dataTemp;

	count = burst;
	while(count>0)
	{
		char buffer[32];
		input_file_data.read(buffer,32); //reading from the file
		string s = "0x";
		for(int j=0;j<32;j++){
			s += buffer[j];
		}
		istringstream iss(s); // converting string to stream
		iss >> hex >> dataTemp;
		indata.data = dataTemp.read();
		if(count==1){
			indata.last=1;
		}
		else{
			indata.last=0;
		}
		inDataFIFO.write(indata);
		char trailing[1];
		input_file_data.read(trailing,1);
		count--;
	}
	input_file_data.close();  // Closing the file

	// Calling the function
	crc_addition(inDataFIFO,cnDataFIFO,outDataFIFO);

	// Writing the data from the output stream interface into created output_data file
	ofstream output_file_data("output_data");  //Creates an output file stream and opens/creates the file
	count = ceil((float)K/128);  // calculating number of output bursts
	while(count>0){
		outDataFIFO >> outdata;
		datau128b outTemp = outdata.data.read();
		string s = "";

		// Writing the next 32 bits into stream
		for(int j=3;j>=0;j--){
			datau32b readTemp;
			stringstream ss;
			readTemp = outTemp.range(32*j+31,32*j);
			unsigned int u = readTemp.read();
			ss<< std::setfill ('0') << std::setw(8) << uppercase << hex << u;
			s += ss.str();
		}

		output_file_data<<s;   //writing into file
		if(count!=1) output_file_data<<endl;
		count--;
	}
	output_file_data.close();  // Closing the file

	// Comparing the output test vector file and created output_data file
		string cmd = "diff -w output_data test_case_1_out_crc";
		if (system(cmd.c_str())){
			printf("Test failed !!!\nOutput data does not match");
			return 1;
		}else {
			cout<<"Test passed !\n";
		}
		return 0;
	}
