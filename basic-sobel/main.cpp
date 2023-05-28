#include "string"
#include "string.h"
#include "cassert"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

// Sobel Filter ACC
static char* const SOBELFILTER_START_ADDR = reinterpret_cast<char* const>(0x73000000);
static char* const SOBELFILTER_READ_ADDR  = reinterpret_cast<char* const>(0x73000004);
static char* const SOBELFILTER_STATUS_ADDR  = reinterpret_cast<char* const>(0x73000008);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;

bool _is_using_dma = true;

void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

int main(int argc, char *argv[]) {
  printf("======================================\n");
  
  unsigned char buffer[4] = {0};
  word data;
  //printf("pixel (%d, %d); \n", y, x);
  buffer[0] = 0;
  buffer[1] = 1;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);
  buffer[0] = 0;
  buffer[1] = 2;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);
  buffer[0] = 0;
  buffer[1] = 3;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);
  buffer[0] = 2;
  buffer[1] = 5;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);
  buffer[0] = 2;
  buffer[1] = 4;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);
  buffer[0] = 5;
  buffer[1] = 0;
  buffer[2] = 1;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);

  unsigned long total_latency = 0;
  int total=0;
  for(unsigned int i = 0; i < 6; i++){
    read_data_from_ACC(SOBELFILTER_READ_ADDR, buffer, 4);
    memcpy(data.uc, buffer, 4);
    total = data.sint;
    printf("node %d dist %d\n",i,total);  
  }
  read_data_from_ACC(SOBELFILTER_STATUS_ADDR, buffer, 4);
  memcpy(data.uc, buffer, 4);
  total = data.sint;
  printf("Total latency: %d\n", total);

  return 0;
}
