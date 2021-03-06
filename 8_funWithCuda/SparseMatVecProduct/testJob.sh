#!/usr/bin/env bash
#SBATCH --job-name=test
#SBATCH --output=test.out
#SBATCH --error=test.err
#SBATCH -p sirocco
#SBATCH --time=00:1:00
#SBATCH --gres=gpu:2

module load compiler/gcc/4.8.4 compiler/cuda/7.0/toolkit/7.0.28
cd ~/funWithGPUs/SparseMatVecProduct
make sparseMatVec
nvprof ./sparseMatVec