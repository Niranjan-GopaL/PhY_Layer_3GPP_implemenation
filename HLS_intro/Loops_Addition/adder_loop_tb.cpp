#include "header.h"


/*
 * THIS IS GOING TO BE THE TEMPLATE BEHIND EVERY TESTBENCH
 *
 * datavec -> strings -> Stream(HLS) ->
 *
 *
 *
 *
 *
 * */



int main(){

	//initializing input and output data stream variables that are passed to the function
	inStream inDataFIFO_A;
	outStream outDataFIFO;
	inStream inDataFIFO_B;
	cnStream cnDataFIFO;


	//initializing input and output variables
	datau8b cndata;
	out_data_axi outdata;
	in_data_axi indata1;
	in_data_axi indata2;

	int count;
	count = 20;
	cndata.range(7,0) = count;//feeding the configuration information

	string inFileName1 = "input1";
	ifstream input1_file_data(inFileName1); // open file
	datau32b dataTemp;


	// Reading 20 lines and writing to a FIFO
	while(count>0){
		char buffer[8]; // 1 char == 1 byte == 8 bits ( it'll take 8 chars to read 32 bit )
		input1_file_data.read(buffer,8);
		string s = "0x";
		for(int j=0;j<8;j++){
			s += buffer[j];
		}
		istringstream iss(s); // converting STRING into STREAM
		iss >> hex >> dataTemp;
		indata1.data = dataTemp.read();
		inDataFIFO_A.write(indata1);

		char trailing[1]; // don't READ THE NEWLINE character '\n'
		input1_file_data.read(trailing,1);
		count--;
	}
	input1_file_data.close(); // closing the file























	// copy paste for input 2


	// count-- in previous loop, so set it to 20;
	count = 20;
	string inFileName2 = "input2";
	ifstream input2_file_data(inFileName2); // open file
	// datau32b dataTemp; // REDEFINITIONS ARE ILLEGAL
	// Reading 20 lines and writing to a FIFO
	while(count>0){
		char buffer[8]; // 1 char == 1 byte == 8 bits ( it'll take 8 chars to read 32 bit )
		input2_file_data.read(buffer,8);
		string s = "0x";
		for(int j=0;j<8;j++){
			s += buffer[j];
		}
		istringstream iss(s); // converting STRING into STREAM
		iss >> hex >> dataTemp;
		indata2.data = dataTemp.read();
		inDataFIFO_B.write(indata2);

		char trailing[1]; // don't READ THE NEWLINE character '\n'
		input2_file_data.read(trailing,1);
		count--;
	}
	input2_file_data.close(); // closing the file




	// TEST YOUR DAMN FUNCTION

	cnDataFIFO.write(cndata);

	ADDER(inDataFIFO_A, inDataFIFO_B, cnDataFIFO, outDataFIFO);

	ofstream output_file_data("output_data");
	count = 20;
	while(count>0){
		outDataFIFO >> outdata; // reading from STREAM into a variable
		string s = "";
		datau40b outTemp = outdata.data.read(); // writing first 8 bits into stream

		datau8b first_set = outTemp.range(39,32);
		stringstream ss_first;
		unsigned int up = first_set.read();

		// up -> hex -> 2 character -> filling with 0 -> ss_first ( A STRING IS WHAT WE FINALLY GET )
		ss_first<<std::setfill('0')<<std::setw(2)<<uppercase<<hex<<up;


		s+=ss_first.str();

		stringstream ss;
		datau32b readTemp = outTemp.range(31, 0);
		unsigned int u = readTemp.read();
		ss<<std::setfill('0')<<std::setw(8)<<uppercase<<hex<<u;
		s+=ss.str();

		output_file_data<<s;
		if(count!=1) output_file_data<<endl;
		count--;
	}
	output_file_data.close();

	string cmd = "diff -w output_data output";
	if(system(cmd.c_str())){
		cout<<"failed"<<endl;
		return 1;
	}else{
		cout<<"passed"<<endl;
		return 0;
	}



}
