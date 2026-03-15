- [+] HLS example from GH ( DCT example used in slides )
- [+] compare solutions
- [+] look at analys view ( across 3 solutions )
- [+] look at schedule view ( across 3 solutions )
- [+] look at resource util view

My Questions :

I wnat to understand 
AXI Interface in our code
the various pragmas used
why are we "word" processing - is that the reason why we need bit reversal ?
In the synthesized hardware - VISUALIZE


What I know :

word = 96bits
G can be 13,120 bits as well
meaning => we process in distinct chunks 



# How to open waveform :

1. DUMP ALL while doing co-sim
2. in Interleaver.tcl comment out quit that is soon after run all
❯ xsim Interleaver -gui -tclbatch Interleaver.tcl
