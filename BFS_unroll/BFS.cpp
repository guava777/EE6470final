#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "BFS.h"
using namespace std;
using namespace sc_core;

const int MAX_SIZE = 6; 

BFS::BFS( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	HLS_FLATTEN_ARRAY(Edge);
	HLS_FLATTEN_ARRAY(visited);
	HLS_FLATTEN_ARRAY(dist);
#endif
	for(unsigned int y = 0; y < MAX_SIZE; y++){
		for(unsigned int x = 0; x < MAX_SIZE; x++){
			HLS_UNROLL_LOOP( ON, "" );
			Edge[y][x] = 0;
		}
		visited[y] = false;
		dist[y] = 99;
	}
	SC_THREAD( do_BFS );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);
        
#ifndef NATIVE_SYSTEMC
	i_cxys.clk_rst(i_clk, i_rst);
  o_result.clk_rst(i_clk, i_rst);
#endif
}

void BFS::do_BFS() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_cxys.reset();
		o_result.reset();
#endif
		wait();
	}

	sc_dt::sc_uint<4> s;
	while (true) {
  sc_dt::sc_uint<10> cxys;
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("input");
			cxys = i_cxys.get();
			wait();
		}
#else
		cxys = i_cxys.read();
#endif
    sc_dt::sc_uint<2> c = cxys.range(9,8);
    sc_dt::sc_uint<4> x = cxys.range(7,4);
    sc_dt::sc_uint<4> y = cxys.range(3,0);
		if(c == 0){
			Edge[x][y] = 1;
			Edge[y][x] = 1;			
		} else {
			s = y;
			break;
		}
	}

	head = 0;
	tail = 0;

	visited[s] = true;  
	dist[s] = 0;
	queue[tail] = s;
	tail++;

	while (head != tail) {
		unsigned int node = queue[head];
		head++;

		for (unsigned int i = 0; i < MAX_SIZE; i++) {
			HLS_UNROLL_LOOP( ON, "" );
			if (Edge[node][i] == 1 && !visited[i]) {
				visited[i] = true;
				dist[i] = dist[node] + 1;
				queue[tail] = i;
				tail++;
			}
		}
	}

	for(unsigned int i = 0; i < MAX_SIZE; i++){
		HLS_UNROLL_LOOP( ON, "" );
		sc_dt::sc_uint<8> o_dist;
		o_dist.range(3,0) = dist[i];
		o_dist.range(7,4) = i;
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_result.put(o_dist);
			wait();
		}
#else
		o_result.write(o_dist);
#endif
	}
}
