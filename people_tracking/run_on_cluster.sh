#!/bin/bash -l
#SBATCH --job-name=CPSE
#SBATCH --time=20:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --output=CPSE-%j.out
#SBATCH --error=CPSE-%j.err
#SBATCH --exclusive
#SBATCH --partition=gpu

# your commands
module load python/3.5.6-DL-Lab-tf1.8
srun python3 -m pip install numpy --user
# srun python3 -m pip install tensorflow==1.15 --user
srun python3 -m pip install keras --user
srun python3 -m pip install opencv-python --user
srun python3 -m pip install beautifulsoup4 --user
srun python3 -m pip install scikit-learn --user
srun python3 -m pip install lxml --user
srun python3 -m pip install pydot --user
srun python3 ssd7_training.py