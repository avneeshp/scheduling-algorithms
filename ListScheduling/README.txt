1) Clone the folder into ../../../llvm/llvm/lib/Transforms 
2) Generate a .bc file for your sample code 
3) opt -analyze -S -p -load ../build/Release+Asserts/lib/list_scheduling.so -l_s < test_ls.bc > /dev/null 
4) Resource_contraints contains the number of multipliers units followed by adder units
5) Outputs a schedule.txt  having asap, alap and list schedules.


