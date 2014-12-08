Follows the following steps to run 
1) Clone the folder into <llvm>/lib/Transforms 
2) Generate a .bc file for benchmark to evaluate
3) opt -analyze -S -p -load <llvm_path>/build/Release+Asserts/lib/LLVMFDS.so -fds < <filename>.bc > /dev/null 
5) An output file schedule.txt is generated in the current working directory having asap, alap and force directed schedules with resource required in each schedule.
