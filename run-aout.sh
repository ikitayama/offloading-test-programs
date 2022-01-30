#!/bin/bash 

#SBATCH --account=jinb33
#SBATCH --nodes=1
##SBATCH --ntasks=24
##SBATCH --ntasks-per-node=12
#SBATCH --output=job.%j.o
#SBATCH --error=job.%j.e
#SBATCH --time=00:05:00
#SBATCH --gres=gpu:1
#SBATCH --partition=dc-gpu-devel
##SBATCH --partition=dc-gpu
##SBATCH --partition=dc-cpu-devel

module restore cuda
module load Nsight-Systems # system-wide profiler, including CPUs metrics

# this is needed RTL
export LD_LIBRARY_PATH=$PROJECT/kitayama1/dev/clang/lib:$LD_LIBRARY_PATH
export OMP_NUM_THREADS=1
#export OMP_TARGET_OFFLOAD=enabled
export LIBOMPTARGET_DEBUG=1
# enable every flag run with every bit set
#export LIBOMPTARGET_INFO=-1
export LIBOMPTARGET_INFO=$((1 << 0x1 | 1 << 0x02 | 1 << 0x04 | 1 << 0x10))
#export LIBOMPTARGET_INFO=1024
#srun --exclusive -n 12 ./mpi-prog1 &
#srun --exclusive -n 12 ./mpi-prog2 &
#srun -n 12 nvprof ./a.out
#srun -n 12 ./a.out
./a.out
#nsys profile ./a.out
#:cd $SCRATCH/kitayama1/projects/llvm-project/openmp/libomptarget/test/api
#./a.out
#wait
