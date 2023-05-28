#ifndef _BFS_H_
#define _BFS_H_

#include <systemc>
using namespace sc_core;

#include <string>
using namespace std;

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

class BFS: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in < bool >  i_rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_uint<10> >::in i_cxys;
	cynw_p2p< sc_dt::sc_uint<8> >::out o_result;
#else
	sc_fifo_in< sc_dt::sc_uint<10> > i_cxys;
	sc_fifo_out< sc_dt::sc_uint<8> > o_result;
#endif

	SC_HAS_PROCESS( BFS );
	BFS( sc_module_name n );
private:
	unsigned int head,tail;
	unsigned int queue[6];
	unsigned int visited[6];
	unsigned int dist[6];
	unsigned int Edge[6][6];
	void do_BFS();
};
#endif
