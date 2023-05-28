#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <string>
using namespace std;

#include <systemc>
using namespace sc_core;

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

class Testbench : public sc_module {
public:
	sc_in_clk i_clk;
	sc_out < bool >  o_rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<10> >::base_out o_cxys;
	cynw_p2p< sc_dt::sc_uint<8> >::base_in i_result;
#else
	sc_fifo_out< sc_dt::sc_uint<10> > o_cxys;
	sc_fifo_in< sc_dt::sc_uint<8> > i_result;
#endif

  SC_HAS_PROCESS(Testbench);
  Testbench(sc_module_name n);
  int clock_cycle( sc_time time );

private:
  sc_time total_run_time;
  sc_time total_start_time;
  	void feed_rgb();
	void fetch_result();
};
#endif
