#ifndef SOBEL_FILTER_H_
#define SOBEL_FILTER_H_
#include <systemc>
#include <cmath>
#include <iomanip>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "filter_def.h"

struct SobelFilter : public sc_module {
  tlm_utils::simple_target_socket<SobelFilter> tsock;

  sc_fifo< int > i_r;
  sc_fifo< int > i_g;
  sc_fifo< int > i_b;
	sc_fifo< int > o_result;

  sc_signal<bool> pe_done;

  SC_HAS_PROCESS(SobelFilter);

  SobelFilter(sc_module_name n): 
    sc_module(n), 
    tsock("t_skt"), 
    pe_done("false")
  {
    tsock.register_b_transport(this, &SobelFilter::blocking_transport);
    SC_THREAD(do_filter);
  }

  ~SobelFilter() {
	}

	unsigned int head,tail;
	unsigned int queue[6];
	unsigned int visited[6];
	unsigned int dist[6];
	unsigned int Edge[6][6];

  void do_filter(){
    int s;
    for(unsigned int y = 0; y < MAX_SIZE; y++){
      for(unsigned int x = 0; x < MAX_SIZE; x++){
        Edge[y][x] = 0;
        Edge[x][y] = 0;
      }
      visited[y] = false;
      dist[y] = 99;
    }
    while (true) {
      int c = i_b.read();
      int x = i_g.read();
      int y = i_r.read();
      //cout << "c = " << c << "x = " << x << "y = " << y << endl;
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
      //cout << "node = " << node << endl;
      for (unsigned int i = 0; i < MAX_SIZE; i++) {
        if (Edge[node][i] == 1 && !visited[i]) {
          visited[i] = true;
          dist[i] = dist[node] + 1;
          queue[tail] = i;
          tail++;
          cout << "node=" << node << ",next node=" << i << ",dist = " << dist[i] << endl;
        }
      }
    }
    for(unsigned int i = 0; i < MAX_SIZE; i++){
      //cout << i << ",dist = " << dist[i] << endl;
      o_result.write(dist[i]);
    }
    pe_done = true;
  }

  void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
    wait(delay);
    // unsigned char *mask_ptr = payload.get_byte_enable_ptr();
    // auto len = payload.get_data_length();
    tlm::tlm_command cmd = payload.get_command();
    sc_dt::uint64 addr = payload.get_address();
    unsigned char *data_ptr = payload.get_data_ptr();


    // cout << (int)data_ptr[0] << endl;
    // cout << (int)data_ptr[1] << endl;
    // cout << (int)data_ptr[2] << endl;
    word buffer;

    switch (cmd) {
      case tlm::TLM_READ_COMMAND:
        // cout << "READ" << endl;
        switch (addr) {
          case SOBEL_FILTER_RESULT_ADDR:
            buffer.uint = o_result.read();
            break;
          case SOBEL_FILTER_STATUS_ADDR:
            buffer.uc[0] = pe_done.read() ? 1 : 0;  // Read status register value
            break;
          default:
            std::cerr << "READ Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
          }
        data_ptr[0] = buffer.uc[0];
        data_ptr[1] = buffer.uc[1];
        data_ptr[2] = buffer.uc[2];
        data_ptr[3] = buffer.uc[3];
        break;
      case tlm::TLM_WRITE_COMMAND:
        // cout << "WRITE" << endl;
        switch (addr) {
          case SOBEL_FILTER_R_ADDR:
            i_r.write(data_ptr[0]);
            i_g.write(data_ptr[1]);
            i_b.write(data_ptr[2]);
            break;
          default:
            std::cerr << "WRITE Error! SobelFilter::blocking_transport: address 0x"
                      << std::setfill('0') << std::setw(8) << std::hex << addr
                      << std::dec << " is not valid" << std::endl;
        }
        break;
      case tlm::TLM_IGNORE_COMMAND:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      default:
        payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
      }
      payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
  }
};
#endif
