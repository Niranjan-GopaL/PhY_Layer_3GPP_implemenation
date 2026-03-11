#include "header.h"

void ADDER(inStream &inData_A, inStream &inData_B, cnStream &cnData, outStream &outData_Y)
{
	// defining the input and output streams and AXI stream ports
	#pragma HLS INTERFACE axis port=inData_A
	#pragma HLS INTERFACE axis port=inData_B
	#pragma HLS INTERFACE axis port=cnData
	#pragma HLS INTERFACE axis port=outData_Y
	#pragma HLS interface ap_ctrl_none port=return // LEARN EXACTLY WHAT THIS LINE DOES
	// "interface" can be upper case or lower case


	in_data_axi A;
	in_data_axi B;
	out_data_axi Y;

	datau8b config = cnData.read();
	datau8b burst= config.range(7,0); // 8bits is ONE BURST

	LOOP_I : for(datau8b i=1; i<=burst; i++){
		A = inData_A.read();
		B = inData_B.read();

		Y.data = A.data + B.data;
		// Y is outData type i.e 40bits
		// A is inData type i.e 32bits

		cout<< hex << Y.data<<endl;

		if(i==burst){
			Y.last=1;
		}else{
			Y.last = 0;
		}

		outData_Y.write(Y);
	}

}
