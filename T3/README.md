# T3: Perfect Hashing
This subproject is the repo of the experiment on perfect hashing tables for this course, it consist of a stress test of the data structure and analyzing results on how well the data structure held on insertion and search in different specific cases of insertion and search.

## Contents

1. The code
   composed of the following folders
   - src
   - include
   - build
2. The report

# The code

## How to compile
1. Move on the command line to the build directory
2. Execute `cmake ..` and cmake will create the makefile
3. on build execute `make` and this will output the `T3.bin`

## How to use
Just use the binary output file and the test will start, the results will be outputted in the command line, use whatever tool you may like to save the output in a file, like the following command `./T3.bin | tee -a "output.csv"`, our use the logs that will appear in the folder logs in the format of `(exp number)_(date)_(time).csv`
