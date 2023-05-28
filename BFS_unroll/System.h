#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <systemc>
using namespace sc_core;

#include "Testbench.h"
#ifndef NATIVE_SYSTEMC
#include "BFS_wrap.h"
#else
#include "BFS.h"
#endif

class System: public sc_module
{
public:
	SC_HAS_PROCESS( System );
	System( sc_module_name n );
private:
  Testbench tb;
#ifndef NATIVE_SYSTEMC
	BFS_wrapper BFS;
#else
	BFS BFS;
#endif
	sc_clock clk;
	sc_signal<bool> rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<10> > cxys;
	cynw_p2p< sc_dt::sc_uint<8> > result;
#else
	sc_fifo< sc_dt::sc_uint<10> > cxys;
	sc_fifo< sc_dt::sc_uint<8> > result;
#endif
};
#endif
