Follows the following steps to run:

1) cd $HOME
2) cp /work/zhang/common/tools/llvm/src/llvm-3.1.sample/ . -a
3) source /work/zhang/common/tools/llvm/3.1/setup.sh
4) Clone the FDS folder into $HOME/llvm-3.1.sample/lib/
5) Add FDS in $HOME/llvm-3.1.sample/lib/Makefile 
	DIRS=sample FDS 	#FDS added in front of sample
6) cd $HOME/llvm-3.1.sample
7) mkdir build
8) cd build
9) ../configure --enable-optimized
10) make -j4

11) cd scheduling-algorithms/ILP/dfgs
12) Generate a .bc file for benchmark to evaluate
/work/zhang/common/tools/llvm/3.1/bin/llvm-as pr.ll	#similar for other benchmarks

13) $HOME/llvm-3.1.sample/build/Release+Asserts/bin/opt -analyze -S -p -load $HOME/llvm-3.1.sample/build/Release+Asserts/lib/LLVMFDS.so -fds < pr.bc > /dev/null
14) An output file schedule.txt is generated in the current working directory having force directed schedules with number of functional units required.
