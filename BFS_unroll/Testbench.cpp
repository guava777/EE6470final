#include "Testbench.h"
using namespace std;

#include <queue>
static std::queue<sc_time> time_queue;

Testbench::Testbench(sc_module_name n) : sc_module(n) {
  SC_THREAD(feed_rgb);
  sensitive << i_clk.pos();
  dont_initialize();
  SC_THREAD(fetch_result);
  sensitive << i_clk.pos();
  dont_initialize();
}

void Testbench::feed_rgb() {
total_start_time = sc_time_stamp();
#ifndef NATIVE_SYSTEMC
	o_cxys.reset();
#endif
	o_rst.write(false);
	wait(5);
	o_rst.write(true);
	wait(1);

	sc_dt::sc_uint<10> cxys;
	cxys.range(3,0) = 0;
	cxys.range(7,4) = 1;
	cxys.range(9,8) = 0;
#ifndef NATIVE_SYSTEMC
	o_cxys.put(cxys);
#else
	o_cxys.write(cxys);
#endif

	cxys.range(3,0) = 0;
	cxys.range(7,4) = 2;
	cxys.range(9,8) = 0;
#ifndef NATIVE_SYSTEMC
	o_cxys.put(cxys);
#else
	o_cxys.write(cxys);
#endif

	cxys.range(3,0) = 2;
	cxys.range(7,4) = 5;
	cxys.range(9,8) = 0;
#ifndef NATIVE_SYSTEMC
	o_cxys.put(cxys);
#else
	o_cxys.write(cxys);
#endif

	cxys.range(3,0) = 0;
	cxys.range(7,4) = 3;
	cxys.range(9,8) = 0;
#ifndef NATIVE_SYSTEMC
	o_cxys.put(cxys);
#else
	o_cxys.write(cxys);
#endif

	cxys.range(3,0) = 4;
	cxys.range(7,4) = 2;
	cxys.range(9,8) = 0;
#ifndef NATIVE_SYSTEMC
	o_cxys.put(cxys);
#else
	o_cxys.write(cxys);
#endif

	cxys.range(3,0) = 5;
	cxys.range(7,4) = 0;
	cxys.range(9,8) = 1;
#ifndef NATIVE_SYSTEMC
	o_cxys.put(cxys);
#else
	o_cxys.write(cxys);
#endif
time_queue.push( sc_time_stamp() );

}

void Testbench::fetch_result() {
unsigned long total_latency = 0;
#ifndef NATIVE_SYSTEMC
	i_result.reset();
#endif
	wait(5);
	wait(1);
  for(unsigned int i = 0; i < 6; i++){
#ifndef NATIVE_SYSTEMC
	sc_dt::sc_uint<8> dist = i_result.get();
#else
	sc_dt::sc_uint<8> dist = i_result.read();
#endif
      sc_time sent_time( time_queue.front() );
        time_queue.pop();
        unsigned long latency = clock_cycle( sc_time_stamp() - sent_time );
        total_latency += latency;
    cout << "i = " << dist.range(7,4) << " dist = " << dist.range(3,0) << endl;
  }
  unsigned long average_latency = total_latency / 6;
  esc_log_latency( "dut", average_latency, "average_latency" );
  cout << "Average latency " << average_latency << "." << endl;
	total_run_time = sc_time_stamp() - total_start_time;
  sc_stop();
}

int Testbench::clock_cycle( sc_time time )
{
    sc_clock * clk_p = dynamic_cast< sc_clock * >( i_clk.get_interface() );
    sc_time clock_period = clk_p->period(); // get period from the sc_clock object.
    return ( int )( time / clock_period );

}
