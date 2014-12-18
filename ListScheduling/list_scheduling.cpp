#include "llvm/ADT/Statistic.h"
#include "llvm//BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/Instructions.h"
#include "llvm/User.h"
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
#include <unistd.h>

using namespace llvm;

#define DEBUG_TYPE "l_s"

namespace {
    
  struct l_s : public BasicBlockPass {
    static char ID; 
    l_s() : BasicBlockPass(ID) {}
    
    std::map<Instruction* , int> ASAP(BasicBlock &BB); 
    int getTotalCSteps(std::map<Instruction*, int> asapList);
    std::set<Instruction*> getSuccessors(Instruction*);
    std::map<Instruction*, int> getPredecessors(Instruction* Inst, std::map<Instruction*, int> instList);
    std::map<Instruction* , int> ALAP(BasicBlock &BB, int csteps);
    void printCycleTimeLS(std::map<Instruction*, int> opMap, std::ofstream& myfile, int max_value);
    std::map<Instruction*, int> ListSchedule(BasicBlock &BB, int* curr_time, int csteps, std::map<Instruction*, int> asapList,  std::map<Instruction*, int> alapList);

    virtual bool runOnBasicBlock(BasicBlock &BB) {
	std::map <Instruction*, int> asapSchedList = ASAP(BB);
	int csteps = getTotalCSteps(asapSchedList);
        std::map <Instruction* , int> alapSchedList = ALAP(BB, csteps);
	int curr_time = 0;
 	std::map <Instruction*, int> listschedule = ListSchedule(BB, &curr_time,csteps,asapSchedList,alapSchedList);

	char* cwd = get_current_dir_name();	
	const char* filename = "/schedule.txt";
	int len = strlen(cwd) + strlen(filename);
	char* path = (char*)malloc(len);
	sprintf(path, "%s%s", cwd, filename);
	std::ofstream myfile;
	myfile.open (path);	
	myfile << "***** List Scheduling ***** \n\n";
	printCycleTimeLS(listschedule, myfile, curr_time);
	myfile.close();
	if (NULL != cwd) free (cwd);
	if (NULL != path) free(path);
	return true;
    }
  };
}

// Function returns a map of scheduled operations
std::map<Instruction* , int>l_s::ASAP(BasicBlock &BB)
{
	std::map <Instruction* , int> asapList ;
	std::vector <llvm::StringRef> instList;
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		instList.push_back(Inst->getName());
		asapList[Inst] = 0;
	}
	
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
		if ((i > 0) && (isPresent == false )) asapList[Inst] = 1;
	}

	unsigned i = 0;
	while (i < asapList.size()) {
		Instruction* Inst;
		for (std::map<Instruction*, int>::iterator mapIter=asapList.begin(); mapIter != asapList.end(); ++mapIter) {
			Inst = mapIter->first;
			if ((unsigned)mapIter->second == i) {
				for (Value::use_iterator UI = Inst->use_begin(); UI != Inst->use_end(); UI++) {
					Instruction *User = static_cast<Instruction*>(*UI);
					asapList[User] = asapList[Inst] + 1;
				}
			}
		}
		i++;
	}
	int c = 0;
	for(std::map<Instruction*, int>::iterator i = asapList.begin(); i!= asapList.end(); i++)
	{
		if( i->second== 0) c++;
		if (c > 2) break;
	}
	if(c>2) {
	for(std::map<Instruction*, int>::iterator i = asapList.begin(); i!= asapList.end(); ++i)
	{	asapList[i->first] = asapList[i->first]+1;
	}} 
	instList.clear();
	return asapList;
}

// Function write instructions scheduled in each cycle for all scheduling algorithms in a file
void l_s::printCycleTimeLS(std::map<Instruction*, int> opMap, std::ofstream& myfile, int max_value) {
	int cycle_num = 1;
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
	for (Value::use_iterator UI = Inst->use_begin(); UI != Inst->use_end(); UI++) {
		Instruction *User = static_cast<Instruction*>(*UI);
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
std::map<Instruction* , int>l_s::ListSchedule(BasicBlock &BB, int* curr_time, int csteps, std::map<Instruction*, int> asapList,  std::map<Instruction*, int> alapList)
{
	int mul_units;
	int add_units;

	char* cwd = get_current_dir_name();
	const char* fname = "/Resource_constraints.txt";
	int length = strlen(cwd) + strlen(fname);
	char* constraints_path = (char*)malloc(length);
	sprintf(constraints_path, "%s%s", cwd, fname);
	std::ifstream myfile;
	myfile.open (constraints_path);
	if (myfile.is_open())
  	{
  		myfile >>mul_units;
		myfile >>add_units;
	        myfile.close();
	}
	else
	{
	        errs() << "Unable to open file"<<'\n';
	}
	if (NULL != cwd) free(cwd);
	if (NULL != constraints_path) free(constraints_path);

	std::map <Instruction* , int> List_p ;
	std::map <Instruction* , int> List_ls ;
        std::map<Instruction*, int> predset;
	BasicBlock::iterator DI;
	int total_mul =0;
	int total_add =0;
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
               	List_p[Inst] =  csteps - alapList[Inst] +1;
		List_ls[Inst] = 0;	}
	int c_m; 
	int p;
	int c_a;
	int time=0;
	unsigned t=0;
	bool canSchedule;
	while(t!=List_ls.size()-1)
	{
		time++;		
		p = csteps;
		c_m = mul_units;
		c_a = add_units;
		
		while(p>0)
		{    
		  std::map<Instruction*, int> predSet;
		  Instruction* It;
		  for(std::map <Instruction* , int>::iterator j = List_p.begin(); j!=List_p.end(); j++)
		  {
					
			canSchedule=true; 	         	
			It = j->first;	
			if(j->second == p)
			{	
			predSet = getPredecessors(It, List_ls);	
			int max_time=time;
			if(predSet.size() > 0) 
			{
			  for(std::map<Instruction*, int>::iterator j1=predSet.begin(); j1!=predSet.end(); j1++)
			  {
				
				if((predSet[j1->first]==0) || (predSet[j1->first] == time)) canSchedule = false;
				if(predSet[j1->first] > max_time) max_time = predSet[j1->first];
			  }
			 }
		
			 if((asapList[It]<=time) && (List_ls[It]==0) && canSchedule)
			  {
			    if(It->getOpcode() == Instruction::Mul)
			      {
				if(c_m){total_mul++;List_ls[It] = time;c_m--;t++;}
                              }
			    else if((It->getOpcode()== Instruction::Add) || (It->getOpcode() == Instruction::Sub))
			      {
				   if(c_a){total_add++;List_ls[It] = time;c_a--;t++;}
			      }
			    else if(It->getOpcode() == Instruction::Store)
			      { 
				 List_ls[It] = max_time;t++;
			      }
			    else
			     {				
				List_ls[It] = max_time;t++;
			     }  					
			   }
			}		          
                   }
   		   p--;
	           if((c_m==0) && (c_a==0)) break;
		}
	}
	
	*curr_time = time;
	List_p.clear();
	return List_ls;
}			        
std::map<Instruction*,int> l_s::getPredecessors(Instruction* Inst, std::map<Instruction*, int> instList)
{
	std::map<Instruction*,int> predSet;
	for (std::map<Instruction*, int>::iterator iter=instList.begin(); iter != instList.end(); ++iter) {
	    for (Value::use_iterator UI = iter->first->use_begin(); UI != iter->first->use_end(); UI++) {
		Instruction *User = static_cast<Instruction*>(*UI);
		if (User == Inst) {
			predSet[iter->first] = instList[iter->first];
		}
	    }
	}
	return predSet;
}

char l_s::ID = 0;
static RegisterPass<l_s> X("l_s", "List Scheduler");

