#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/STLExtras.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <map>
#include <set>
#include <list>
#include <string.h>

#define MAX_VAL 100

using namespace llvm;

#define DEBUG_TYPE "l_s"

namespace {
    
  struct l_s : public BasicBlockPass {
    static char ID; // Pass identification, replacement for typeid
    l_s() : BasicBlockPass(ID) {}
    
    std::map<Instruction* , int> ASAP(BasicBlock &BB); //Function for asap
    int getTotalCSteps(std::map<Instruction*, int> asapList);
    std::set<Instruction*> getSuccessors(Instruction*);
    std::map<Instruction* , int> ALAP(BasicBlock &BB, int csteps);
    void printCycleTime(std::map<Instruction*, int>, std::ofstream& );
    std::map<Instruction*, int> ListSchedule(BasicBlock &BB, int csteps, std::map<Instruction*, int> asapList,  std::map<Instruction*, int> alapList);

    bool runOnBasicBlock(BasicBlock &BB) override {
	std::map <Instruction*, int> asapSchedList = ASAP(BB);
	int csteps = getTotalCSteps(asapSchedList);
        std::map <Instruction* , int> alapSchedList = ALAP(BB, csteps);
 	std::map <Instruction*, int> listschedule = ListSchedule(BB, csteps,asapSchedList,alapSchedList);
	//std::list <Operation> OpList; //= new std::list<Operation*>();
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		int i = Inst->getNumOperands();
		if (i > 0) {
			/*Operation* Op = new Operation(Inst, 1);
			Op->setEarliest(asapSchedList[Inst]);
			//Op->setLatest(alapSchedList[Inst]);
			if (asapSchedList[Inst] == alapSchedList[Inst])
				Op->setFixed();
			OpList.push_back(Op);*/
		}
	} 
	
	std::ofstream myfile;
	myfile.open ("/home/deepagm/llvm/TestFiles/schedule.txt");
	myfile << "\n***** ASAP Scheduling ***** \n\n";
	printCycleTime(asapSchedList, myfile);
        myfile << "\n\n***** ALAP Scheduling ***** \n\n";
	printCycleTime(alapSchedList, myfile);
	myfile << "\n\n***** List Scheduling ***** \n\n";
	printCycleTime(listschedule, myfile);
	myfile.close();
	return true;
    }
  };
}

// Function returns a map of scheduled operations
std::map<Instruction* , int>l_s::ASAP(BasicBlock &BB)
{
	std::map <Instruction* , int> asapList ; //= new std::map<Instruction*, int>();
	std::vector <llvm::StringRef> instList; //= new std::set<llvm::StringRef>();
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		instList.push_back(Inst->getName());
		asapList[Inst] = 0;
	}
        std::ofstream myfile;
	
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		unsigned i = Inst->getNumOperands();
		Value* leftOp = (i > 0) ? Inst->getOperand(0) : NULL;
		Value* rightOp = (i > 1) ? Inst->getOperand(1) : NULL;
		std::vector<llvm::StringRef>::iterator iter;
		bool isPresent = false;
		for (iter = instList.begin(); iter != instList.end(); iter++)
		{
			if ((leftOp && (*iter == leftOp->getName())) || (rightOp && (*iter == rightOp->getName())))
				isPresent = true;
		}
		if ((i > 0) && (false == isPresent)) asapList[Inst] = 1;
	}

	unsigned i = 0;
	while (i < asapList.size()) {
		Instruction* Inst;
		for (std::map<Instruction*, int>::iterator mapIter=asapList.begin(); mapIter != asapList.end(); ++mapIter) {
			Inst = mapIter->first;
			if ((unsigned)mapIter->second == i) {
				for (Use &U : Inst->uses()) { // Inst->uses returns iterator_range //rangebased for loop
					Instruction *User = cast<Instruction>(U.getUser());
					asapList[User] = asapList[Inst] + 1;
				}
			}
		}
		i++;
	}
	instList.clear();
	return asapList;
}

// Function write instructions scheduled in each cycle for all scheduling algorithms in a file
void l_s::printCycleTime(std::map<Instruction*, int> opMap, std::ofstream& myfile) {
	int cycle_num = 1;
	int max_value = getTotalCSteps(opMap);
	Instruction *Inst;
	while (cycle_num <= max_value) {
		myfile << "[ Cycle Time " << cycle_num << " ]\n";
		for (std::map<Instruction*, int>::iterator mapIter = opMap.begin(); mapIter != opMap.end(); ++mapIter) {
			Inst = mapIter->first;
			std::string str;
			llvm::raw_string_ostream rso(str);
			Inst->print(rso);
			if (mapIter->second == cycle_num) {
				myfile << str << "\n";
			}
		}
		cycle_num = cycle_num + 1;
	}
	return;
}

// Function returns total number of cycles/control-steps used in ASAP scheduling
int l_s::getTotalCSteps(std::map<Instruction*, int> asapList)
{
	int max_value = 0;
	for (std::map<Instruction*, int>::iterator mapIter=asapList.begin(); mapIter != asapList.end(); mapIter++) {
	    if (mapIter->second > max_value)
		max_value = mapIter->second; 
	}
	return max_value;
}

std::set<Instruction*> l_s::getSuccessors(Instruction* Inst)
{
	std::set<Instruction*> succSet;
	for (Use &U : Inst->uses()) {
		Instruction *User = cast<Instruction>(U.getUser());
		succSet.insert(User);
	}
	return succSet;
}

std::map<Instruction* , int> l_s::ALAP(BasicBlock &BB, int csteps)
{
	std::map <Instruction* , int> alapList;
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		unsigned i = Inst->getNumOperands();
		if (( i > 0) && (true == Inst->hasNUses(0))) 
			alapList[Inst] = csteps;
		else if (i > 0)
			alapList[Inst] = csteps + 1;
	}
	
	int j = csteps;
	while (j > 0) {
		Instruction* Inst;
		for (std::map<Instruction*, int>::iterator mapIter=alapList.begin(); mapIter != alapList.end(); ++mapIter) {
			Inst = mapIter->first;
			std::set<Instruction*> succ = getSuccessors(Inst);
			bool allSuccSched = true;
			int min_succ_val = csteps + 1;
			for (std::set<Instruction*>::iterator iter=succ.begin(); iter != succ.end(); ++iter) {
				if (alapList[*iter] == (csteps + 1)) {
					allSuccSched = false;
					break;
				} else if (alapList[*iter] < min_succ_val) {
					min_succ_val = alapList[*iter];
				}
			}
			if (allSuccSched == true) alapList[Inst] = min_succ_val - 1;
		}
		j--;
	}

	return alapList;
}
// Function returns a map of scheduled operations
std::map<Instruction* , int>l_s::ListSchedule(BasicBlock &BB, int csteps, std::map<Instruction*, int> asapList,  std::map<Instruction*, int> alapList)
{
	int mul_units;
	int add_units;
	std::ifstream myfile;
	myfile.open ("/home/deepagm/llvm/TestFiles/Resource_constraints.txt");
	if (myfile.is_open())
  	{
  		myfile >>mul_units;
		//errs() <<mul_units<<'\n';
		myfile >>add_units;
		//errs() <<add_units<<'\n';
	        myfile.close();
	}
	else
	{
	        errs() << "Unable to open file"<<'\n';
	}

	std::map <Instruction* , int> List_p ; //Instruction list with priority
	std::map <Instruction* , int> List_ls ; //map for listscheduled instructions
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
               	List_p[Inst] =  csteps - (alapList[Inst] - asapList[Inst]);
		List_ls[Inst]= 0;
	}
	int c_m=0; 
	int c_a=0;
	Instruction* I;
	for(auto i = List_p.begin(); i != List_p.end(); i++){
		I = i->first;	
		if(i->second == csteps) //max priority = csteps;
		{
				List_ls[I] = 1;	
		       		if(I->getOpcode()== Instruction::Mul)
					c_m++;
		       		if((I->getOpcode()== Instruction::Add) || (I->getOpcode() == Instruction::Sub))
					c_a++;
		}
	}
	//errs() << c_m << '\n';
	if(c_m >= mul_units){
		
		errs() << "Unable to schedule minimum number of mul_units needed = " << c_m << '\n';
		errs() << "Reacllocating Max number of available units to " << c_m << '\n';
		mul_units = c_m;
	}
	if(c_a >= mul_units){
		
		errs() << "Unable to schedule minimum number of AddSub_units needed = " << c_a << '\n';
		errs() << "Reacllocating Max number of available units to " << c_a << '\n';
		add_units = c_a;
	}

	int i = 2;	
	int p;
	while(i<=csteps)
	{   
		p = csteps-1;		
		c_m = mul_units;
		c_a = add_units;
		//errs()<< "i " << i << '\n';
		Instruction* It;
		std::map<Instruction*, int>::iterator j ;
		while((c_m!=0) || (c_a!=0)||(p>0))
		{
			for(auto j = List_p.begin(); j!=List_p.end(); j++) {
					It = j->first;			
					if(j->second == p)
					{	if(!((List_ls[It]<i) && (List_ls[It]>1) && (asapList[It]<=i))){
							if(It->getOpcode()== Instruction::Mul){
								if(c_m)List_ls[It] = i;	c_m--;}
							else if((It->getOpcode()== Instruction::Add) || (It->getOpcode() == Instruction::Sub)){
								if(c_a)List_ls[It] = i; c_a--;}
							else List_ls[It] = alapList[It];					
										        
					}}			 
			            }
			p--;
			if(p==0) break;
		}		
		i++;
	}
	List_p.clear();
	return List_ls;
}			        
	

char l_s::ID = 0;
static RegisterPass<l_s> X("l_s", "List Scheduler");

