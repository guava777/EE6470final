#include "string"
#include "string.h"
#include "cassert"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

#define PROCESSORS 2

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

// Sobel Filter ACC
static char* const SOBELFILTER_START_ADDR[2] = {reinterpret_cast<char* const>(0x73000000),reinterpret_cast<char* const>(0x74000000)};
static char* const SOBELFILTER_READ_ADDR[2] = {reinterpret_cast<char* const>(0x73000004),reinterpret_cast<char* const>(0x74000004)};
static char* const SOBELFILTER_STATUS_ADDR  = reinterpret_cast<char* const>(0x73000008);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;
static const uint32_t DMA_OP_NOP = 0;

bool _is_using_dma = true;
//the barrier synchronization objects
uint32_t barrier_counter=0; 
uint32_t barrier_lock; 
uint32_t barrier_sem; 
//the mutex object to control global summation
uint32_t lock;  
//print synchronication semaphore (print in core order)
uint32_t print_sem[PROCESSORS]; 

int sem_init (uint32_t *__sem, uint32_t count) __THROW{
  *__sem=count;
  return 0;
}
int sem_wait (uint32_t *__sem) __THROW{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     beqz %[value],L%=                   # if zero, try again\n\t\
     addi %[value],%[value],-1           # value --\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int sem_post (uint32_t *__sem) __THROW{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     addi %[value],%[value], 1           # value ++\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int barrier(uint32_t *__sem, uint32_t *__lock, uint32_t *counter, uint32_t thread_count) {
	sem_wait(__lock);
	if (*counter == thread_count - 1) { //all finished
		*counter = 0;
		sem_post(__lock);
		for (int j = 0; j < thread_count - 1; ++j) sem_post(__sem);
	} else {
		(*counter)++;
		sem_post(__lock);
		sem_wait(__sem);
	}
	return 0;
}

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

int main(int argc, char *argv[] ,unsigned hart_id) {
  printf("======================================\n");
  if (hart_id == 0) {
		// create a barrier object with a count of PROCESSORS
		sem_init(&barrier_lock, 1);
		sem_init(&barrier_sem, 0); //lock all cores initially
		for(int i=0; i< 2; ++i){
			sem_init(&print_sem[i], 0); //lock printing initially
		}
		// Create mutex lock
		sem_init(&lock, 1);
	}
  unsigned int print_id = 0;
  
  unsigned char buffer[4] = {0};
  word data;

  //printf("pixel (%d, %d); \n", y, x);
  buffer[0] = 0;
  buffer[1] = 1;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR[0], buffer, 4);
  write_data_to_ACC(SOBELFILTER_START_ADDR[1], buffer, 4);
  buffer[0] = 0;
  buffer[1] = 2;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR[0], buffer, 4);
  write_data_to_ACC(SOBELFILTER_START_ADDR[1], buffer, 4);
  buffer[0] = 0;
  buffer[1] = 3;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR[0], buffer, 4);
  write_data_to_ACC(SOBELFILTER_START_ADDR[1], buffer, 4);
  buffer[0] = 2;
  buffer[1] = 5;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR[0], buffer, 4);
  write_data_to_ACC(SOBELFILTER_START_ADDR[1], buffer, 4);
  buffer[0] = 2;
  buffer[1] = 4;
  buffer[2] = 0;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR[0], buffer, 4);
  write_data_to_ACC(SOBELFILTER_START_ADDR[1], buffer, 4);
  buffer[0] = 5;
  buffer[1] = 0;
  buffer[2] = 1;
  buffer[3] = 0;
  write_data_to_ACC(SOBELFILTER_START_ADDR[0], buffer, 4);
  write_data_to_ACC(SOBELFILTER_START_ADDR[1], buffer, 4);

  unsigned long total_latency = 0;
  int total=0;
  for(unsigned int i = 0; i < 6; i++){
    read_data_from_ACC(SOBELFILTER_READ_ADDR[0], buffer, 4);
    memcpy(data.uc, buffer, 4);
    total = data.sint;
    
    read_data_from_ACC(SOBELFILTER_READ_ADDR[1], buffer, 4);
    memcpy(data.uc, buffer, 4);
    total = data.sint;
  }
  read_data_from_ACC(SOBELFILTER_STATUS_ADDR, buffer, 4);
  memcpy(data.uc, buffer, 4);
  total = data.sint;
  printf("Total latency: %d\n", total);

  return 0;
}
