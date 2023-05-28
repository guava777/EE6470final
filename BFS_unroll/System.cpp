#include "System.h"
System::System( sc_module_name n ): sc_module( n ), 
	tb("tb"), BFS("BFS"), clk("clk", CLOCK_PERIOD, SC_NS), rst("rst")
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	BFS.i_clk(clk);
	BFS.i_rst(rst);

	tb.o_cxys(cxys);
	tb.i_result(result);
	BFS.i_cxys(cxys);
	BFS.o_result(result);
}
