#include "header.hpp"

int main () {
  din_t A[N];
	dout_t accum;

	int i, j, retval=0;
	FILE        *fp;

	// Create input data
	for(i=0; i<N;++i) {
	  A[i]=i;
	}
	// Save the results to a file
	fp=fopen("result.dat","w");

	// Call the function
	for(j=0; j<NUM_TRANS;++j) {
		accum = nested_loop(A);
		fprintf(fp, "%d \n", accum);
		// New input data
		for(i=0; i<N;++i) {
			A[i]=A[i]+N;
		}
	}
	fclose(fp);

	// Compare the results file with the golden results
    retval = system("diff --brief -w result.dat result.golden.dat");

	if (retval != 0) {
		printf("Test failed  !!!\n");
		retval=1;
	} else {
		printf("Test passed !\n");
  }

	// Return 0 if the test
  return retval;
}
