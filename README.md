# EE6470final

# HLS
cd BFS_unroll/stratus /n
make sim_V_BASIC

# RISCV
cd vp/build/ /n
cmake .. /n
make /n

cd sw/basic-sobel /n
make
