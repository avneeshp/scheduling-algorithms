#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/STLExtras.h"
#include <vector>
#include <fstream>
#include <algorithm>
#include <map>
#include <set>
#include <list>
#include <string.h>

#define MAX_VAL 100

using namespace llvm;

#define DEBUG_TYPE "fds"

namespace {

typedef enum ResType {
	NOT_SET = 0,
	MUL,
	ALU
} resource_type;

class Operation
{
    private:
	Instruction* m_inst;
	int 	m_latency;
	bool 	m_fixed;
	int 	m_earliest;
	int 	m_latest;
	resource_type m_type;

    public:
	Operation (Instruction* inst, int latency)
	{
		m_inst	   = inst;	
		m_latency  = latency;
		m_fixed    = false;
		m_earliest = 0;
		m_latest   = MAX_VAL;
		m_type	   = NOT_SET;
	}
	~Operation();

	// Member function to get operation name/id
	Instruction* op_id()
	{
		return m_inst;
	}
	
	// Member function to get operation type
	resource_type getOpType() const
	{
		return m_type;
	}

	// Member function to check whether operation is fixed to one cycle or can be scheduled in multiple cycles
	bool isFixed ()
	{
		return m_fixed;
	}

	// Member function to set variable m_fixed to true if an operation has same ASAP and ALAP cycles otherwise false
	void setFixed ()
	{
		m_fixed = true;
	}

	// Member function to get operation latency
	int getLatency ()
	{
		return m_latency;
	}

	// Member function to get earliest cycle to which an operation can be scheduled
	int getEarliest () const
	{
		return m_earliest;
	}
	
	// Member function to set earliest cycle to which an operation can be scheduled
	void setEarliest (int earliest)
	{
		m_earliest = earliest;
	}

	// Member function to get latest cycle to which an operation can be scheduled
	int getLatest () const
	{
		return m_latest;
	}

	// Member function to set latest cycle to which an operation can be scheduled
	void setLatest (int latest)
	{
		m_latest = latest;
	}
	
	// Member function to set resource type / functional unit required for operation
	void setResourceType(resource_type rtype)
	{
		m_type = rtype;
	}

	// Member function to get resource type / functional unit required for operation
	resource_type getResourceType()	const
	{
		return m_type;
	}

	// Member function returns the range or interval in which an operation can be scheduled
	int getMobility ()	const
	{
		return m_latest - m_earliest + 1;
	}
	
	// Member function returns the probability of scheduling an operation o in control step s
	double getProbability (int s)	const
	{
		if (s >= m_latest + m_latency) return 0;
		if (s < m_earliest) return 0;

		double result = 1.0 / (double) (getMobility());

		return result;	//considering latency of each operation as 1 for now
	}
};	
   
struct comp
{
  bool operator()(Operation* s1, Operation* s2) const
  {
    return ((s1->getEarliest() + s2->getEarliest()) > 0);
  }
};
 
class DistGraph {
    private:
	// Member represents the type of function unit
	resource_type m_rtype;
	std::set <Operation*, comp>* m_operations;
	std::map <Operation*, double, comp>* m_avg_resource_usage;

    public:
	DistGraph (resource_type rtype)
	{
		m_rtype = rtype;
		m_operations = new std::set <Operation*, comp>();
		m_avg_resource_usage = new std::map <Operation*, double, comp>();
	}
	~DistGraph()
	{
		delete m_operations;
		delete m_avg_resource_usage;
	}

	resource_type getDGType() const
	{
		return m_rtype;
	}
	
	void insertOp(Operation* op) {
		m_operations->insert(op);
		return;
	}

	std::set<Operation*, comp>* getOp() 
	{
		return m_operations;
	}

	// Member function to calculate the input function unit usage in input time step
	double getResourceUsage(resource_type rtype, int cstep) {
		if (m_rtype != rtype) return 0.0;
		double r_usage = 0.0;
		for (std::set<Operation*>::iterator opIter = m_operations->begin(); opIter != m_operations->end(); ++opIter) {
			r_usage += (*opIter)->getProbability (cstep);
		}
		return r_usage;
		
	}
	void avgResourceUsage(resource_type rtype, Operation* op) {
		double avg_usage = 0.0;
		int start = op->getEarliest();
		int end = op->getLatest();
		for (int i = start; (start != end) && (i <= end); i++) {
			avg_usage += getResourceUsage(rtype, i);
		}
		avg_usage /= ((double) (end - start + 1));
		(*m_avg_resource_usage)[op] = avg_usage;
		return;
	}

	// Member function to get self force for a resource at a particular time step
	double selfForce(Operation* op, int cstep) {
		double sForce = 0.0;
		resource_type rtype = op->getResourceType();
		int start = op->getEarliest();
		int end = op->getLatest();
		if ((cstep >= start) && (cstep <= end)) return 0.0;
		double rusage_at_time = getResourceUsage(rtype, cstep);
		double avg_rusage = ((*m_avg_resource_usage)[op]);
		sForce = rusage_at_time - avg_rusage;
		return sForce;
	}
	
	// Member function to get successor force for a resource
	double succForce(Operation* succ, int cstep) {
		int start_time = cstep + 1;

		int prev_start_time = succ->getEarliest();
		succ->setEarliest(start_time);
		
		double succForce = 0.0;
		int range = succ->getMobility();
		resource_type rtype = succ->getResourceType();
		double rusage_wrt_time = getResourceUsage(rtype, cstep);
		double avg_rusage = ((*m_avg_resource_usage)[succ]) / range;
		succForce = rusage_wrt_time - avg_rusage;

		succ->setEarliest(prev_start_time);
		return succForce;
	}

	// Member function to get predecessor force for a resource
	double predForce(Operation* pred, int cstep) {
		int end_time   = cstep - 1;

		int prev_latest_time = pred->getLatest();
		pred->setLatest(end_time);
		
		double predForce = 0.0;
		int range = pred->getMobility();
		resource_type rtype = pred->getResourceType();
		double rusage_at_time = getResourceUsage(rtype, cstep);
		double avg_rusage = ((*m_avg_resource_usage)[pred]) / range;
		predForce = rusage_at_time - avg_rusage;
		pred->setLatest(prev_latest_time);
		return predForce;
	}


};

  struct fds : public BasicBlockPass {
    static char ID; // Pass identification, replacement for typeid
    fds() : BasicBlockPass(ID) {}
    
    std::map<Instruction* , int>* ASAP(BasicBlock &BB);
    std::map<Instruction* , int>* ALAP(BasicBlock &BB, int csteps);
    std::set<Instruction*>* getPredecessors(Instruction*, std::map<Instruction*, int>* );
    std::set<Instruction*>* getSuccessors(Instruction*);
    int getTotalCSteps(std::map<Instruction*, int> asapList);
    void printCycleTime(std::map<Instruction*, int>, std::ofstream& );
    resource_type fuType(Instruction*);
    //void calculateFDScheduling();
    
    bool runOnBasicBlock(BasicBlock &BB) override {
	std::map <Instruction* , int>* asapSchedList = ASAP(BB);
	int csteps = getTotalCSteps(*asapSchedList);
	std::map <Instruction* , int>* alapSchedList = ALAP(BB, csteps);
	std::list <Operation*>* OpList = new std::list<Operation*>();
	std::map <Operation*, std::list<Operation*>*>* succMap = new std::map<Operation*, std::list<Operation*>*>();
	std::map <Operation*, std::list<Operation*>*>* predMap = new std::map<Operation*, std::list<Operation*>*>();
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		int i = Inst->getNumOperands();
		if (i > 0) {
			Operation* Op = new Operation(Inst, 1);
			Op->setEarliest((*asapSchedList)[Inst]);
			Op->setLatest((*alapSchedList)[Inst]);
			if ((*asapSchedList)[Inst] == (*alapSchedList)[Inst])
				Op->setFixed();
			resource_type rtype = fuType(Inst);
			Op->setResourceType(rtype);
			OpList->push_back(Op);
		}
	}
	
	for (std::list<Operation*>::iterator opIter = OpList->begin(); opIter != OpList->end(); ++opIter) {
		Instruction* inst = (*opIter)->op_id();
		std::set<Instruction*>* succ = getSuccessors(inst);
		std::list <Operation*>* succList = new std::list<Operation*>();
		for (std::set<Instruction*>::iterator it = succ->begin(); it != succ->end(); it++) {
			Instruction *Inst = *it;
			int i = Inst->getNumOperands();
			if (i > 0) {
				Operation* Op = new Operation(Inst, 1);
				Op->setEarliest((*asapSchedList)[Inst]);
				Op->setLatest((*alapSchedList)[Inst]);
				if ((*asapSchedList)[Inst] == (*alapSchedList)[Inst])
					Op->setFixed();
				resource_type rtype = fuType(Inst);
				Op->setResourceType(rtype);
				succList->push_back(Op);
			}

		}
		(*succMap)[*opIter] = succList;
	}
	for (std::list<Operation*>::iterator opIter = OpList->begin(); opIter != OpList->end(); ++opIter) {
		Instruction* inst = (*opIter)->op_id();
		std::set<Instruction*>* pred = getPredecessors(inst, asapSchedList);
		std::list <Operation*>* predList = new std::list<Operation*>();
		for (std::set<Instruction*>::iterator it = pred->begin(); it != pred->end(); it++) {
			Instruction *Inst = *it;
			int i = Inst->getNumOperands();
			if (i > 0) {
				Operation* Op = new Operation(Inst, 1);
				Op->setEarliest((*asapSchedList)[Inst]);
				Op->setLatest((*alapSchedList)[Inst]);
				if ((*asapSchedList)[Inst] == (*alapSchedList)[Inst])
					Op->setFixed();
				resource_type rtype = fuType(Inst);
				Op->setResourceType(rtype);
				predList->push_back(Op);
			}

		}
		(*predMap)[*opIter] = predList;
	}
	

	//Distribution graph for multiply
	resource_type multiplier = MUL;
	DistGraph* dGraphMul = new DistGraph(multiplier);
	for (std::list<Operation*>::iterator opIter = OpList->begin(); opIter != OpList->end(); ++opIter) {
		if ((*opIter)->getOpType() == MUL) {
		   dGraphMul->insertOp(*opIter);
		   dGraphMul->avgResourceUsage(multiplier, *opIter);
		}
	}
	
	//Distribution graph for ALU
	resource_type add_sub = ALU;
	DistGraph* dGraphALU = new DistGraph(add_sub);
	for (std::list<Operation*>::iterator opIter = OpList->begin(); opIter != OpList->end(); ++opIter) {
		if ((*opIter)->getOpType() == ALU) {
		   dGraphALU->insertOp(*opIter);
		   dGraphALU->avgResourceUsage(add_sub, *opIter);
		}
	}
	
	std::ofstream myfile;
	myfile.open ("/home/avneesh/Desktop/sched/sched_v2/schedule.txt");
	myfile << "\n***** ASAP Scheduling ***** \n\n";
	printCycleTime(*asapSchedList, myfile);
	myfile << "\n\n***** ALAP Scheduling ***** \n\n";
	printCycleTime(*alapSchedList, myfile);
	myfile << "\n\n***** Time Frames ***** \n\n";
	for (std::list<Operation*>::iterator opIter = OpList->begin(); opIter != OpList->end(); ++opIter) {
		Operation* Op = *opIter;
		Instruction* inst = Op->op_id();
		std::string str;
		llvm::raw_string_ostream rso(str);
		inst->print(rso);	
		myfile << str << "\n";
		myfile << "Start Cycle: " << Op->getEarliest() << "\n";
		myfile << "End Cycle: " << Op->getLatest() << "\n";
	}
	myfile << "\n\n***** Distribution Graph for Multiply ***** \n\n";
	for (int step = 1; step < csteps; step++) {
		myfile << "Time Step : " << step << "\t" << dGraphMul->getResourceUsage(multiplier, step) << "\n";
		std::set<Operation*, comp>* mulOpSet = dGraphMul->getOp();
		for (std::set<Operation*, comp>::iterator opIter = mulOpSet->begin(); opIter != mulOpSet->end(); ++opIter) {
			Operation* Op = *opIter;
			Instruction* inst = Op->op_id();
			std::string str;
			llvm::raw_string_ostream rso(str);
			inst->print(rso);
			double avg_usage = 0.0;
			int start = (*opIter)->getEarliest();
			int end = (*opIter)->getLatest();
			double rusage = dGraphMul->getResourceUsage(multiplier, step);
			if ((step < start) || (step > end)) continue;
			for (int i = start; i <= end; i++) {
				avg_usage += dGraphMul->getResourceUsage(multiplier, i);
			}
			avg_usage /= ((double) (end - start + 1));
			myfile << "Self Force : " << str << " : " << step <<" => " << (rusage - avg_usage) << "\n";
		}
	}	
	myfile << "\n\n***** Distribution Graph for ALU ***** \n\n";
	for (int step = 1; step < csteps; step++) {
		myfile << "Time Step : " << step << "\t" << dGraphALU->getResourceUsage(add_sub, step) << "\n";
		std::set<Operation*, comp>* aluOpSet = dGraphALU->getOp();
		for (std::set<Operation*, comp>::iterator opIter = aluOpSet->begin(); opIter != aluOpSet->end(); ++opIter) {
			Operation* Op = *opIter;
			Instruction* inst = Op->op_id();
			std::string str;
			llvm::raw_string_ostream rso(str);
			inst->print(rso);
			double avg_usage = 0.0;
			int start = (*opIter)->getEarliest();
			int end = (*opIter)->getLatest();
			double rusage = dGraphALU->getResourceUsage(add_sub, step);
			if ((step < start) || (step > end)) continue;
			for (int i = start; i <= end; i++) {
				avg_usage += dGraphALU->getResourceUsage(add_sub, i);
			}
			avg_usage /= ((double) (end - start + 1));
			myfile << "Self Force : " << str << " : " << step <<" => " << (rusage - avg_usage) << "\n";
			if (Op->isFixed() == false) {
				std::list<Operation*>* sList = (*succMap)[Op];
				for (std::list<Operation*>::iterator it = sList->begin(); it != sList->end(); it++) {
					Operation* successor = *opIter;
					Instruction* Inst = successor->op_id();
					std::string sstr;
					llvm::raw_string_ostream rso(sstr);
					Inst->print(rso);
					int start_time = step + 1;
					double avg_succ_usage = 0.0;

					int prev_start_time = successor->getEarliest();
					successor->setEarliest(start_time);
					int end_time = successor->getLatest();
					double succ_rusage = dGraphALU->getResourceUsage(add_sub, step);
					for (int j = start_time; j <= end_time; j++) {
						avg_succ_usage += dGraphALU->getResourceUsage(add_sub, j);
					}
					avg_succ_usage /= ((double) (end_time - start_time + 1));
					myfile << "Succ Force : " << sstr << " : " << step <<" => " << (succ_rusage - avg_succ_usage) << "\n";
					successor->setEarliest(prev_start_time);
				}
				std::list<Operation*>* pList = (*predMap)[Op];
				for (std::list<Operation*>::iterator it = pList->begin(); it != pList->end(); it++) {
					Operation* predecessor = *opIter;
					Instruction* Inst = predecessor->op_id();
					std::string pstr;
					llvm::raw_string_ostream rso(pstr);
					Inst->print(rso);
					int end_time = step - 1;
					double avg_pred_usage = 0.0;

					int prev_end_time = predecessor->getLatest();
					predecessor->setLatest(end_time);
					int start_time = predecessor->getEarliest();
					double pred_rusage = dGraphALU->getResourceUsage(add_sub, step);
					for (int j = start_time; j <= end_time; j++) {
						avg_pred_usage += dGraphALU->getResourceUsage(add_sub, j);
					}
					avg_pred_usage /= ((double) (end_time - start_time + 1));
					myfile << "Pred Force : " << pstr << " : " << step <<" => " << (pred_rusage - avg_pred_usage) << "\n";
					predecessor->setLatest(prev_end_time);
				}
			}

		}
	}	
	myfile.close();
	return true;
    }
  };
}

// Function returns the functional unit required for the input instruction
resource_type fds::fuType (Instruction* Inst)
{
	resource_type rtype = NOT_SET;	
	switch (Inst->getOpcode())
	{
		case Instruction::Add :
			rtype = ALU;
			break;
		case Instruction::Sub :
			rtype = ALU;
			break;
		case Instruction::Mul :
			rtype = MUL;
			break;
		default:
			break;
	}
	return rtype;
}

// Function returns a map of operations scheduled ASAP
std::map<Instruction* , int>* fds::ASAP(BasicBlock &BB)
{
	std::map <Instruction* , int>* asapList = new std::map<Instruction*, int>();
	std::set<Instruction*>* instList = new std::set<Instruction*>();
	
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		unsigned i = Inst->getNumOperands();
		if (i > 0) { 
			(*asapList)[Inst] = 0;
			instList->insert(Inst);
		}
	}
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		unsigned i = Inst->getNumOperands();
		std::set<Instruction*>*pred = getPredecessors(Inst, asapList);
		if ((i > 0) && (pred->empty())) {
			(*asapList)[Inst] = 1;
	    		instList->erase(Inst);
		}
		delete pred;
	}
	while (instList->size() > 0) {
	    Instruction* Inst;
	    for (std::map<Instruction*, int>::iterator sIter=asapList->begin(); sIter != asapList->end(); ++sIter) {
		Inst = sIter->first;
            	if (instList->find(sIter->first) == instList->end()) continue;
		std::set<Instruction*>* pred = getPredecessors(Inst, asapList);
		bool allPredSched = true;
		int max_pred_val = 0;
		for (std::set<Instruction*>::iterator iter=pred->begin(); iter != pred->end(); ++iter) {
		    if (instList->find(*iter) != instList->end()) {
			allPredSched = false;
			break;
		    } else if ((*asapList)[*iter] > max_pred_val) {
			max_pred_val = (*asapList)[*iter];
		    }
		}
		if (allPredSched == true) {
			(*asapList)[Inst] = max_pred_val + 1;
	    		instList->erase(Inst);
		}
		delete pred;
	    }
	}
	
	delete instList;
	return asapList;
}

// Function write instructions scheduled in each cycle for all scheduling algorithms in a file
void fds::printCycleTime(std::map<Instruction*, int> opMap, std::ofstream& myfile) {
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

// Function returns a map of operations scheduled ALAP
std::map<Instruction* , int>* fds::ALAP(BasicBlock &BB, int csteps)
{
	std::map <Instruction* , int>* alapList = new std::map<Instruction*, int>();
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		unsigned i = Inst->getNumOperands();
		if (( i > 0) && (true == Inst->hasNUses(0))) 
			(*alapList)[Inst] = csteps;
		else if (i > 0)
			(*alapList)[Inst] = csteps + 1;
	}
	
	int j = csteps;
	while (j > 0) {
		Instruction* Inst;
		for (std::map<Instruction*, int>::iterator mapIter=alapList->begin(); mapIter != alapList->end(); ++mapIter) {
			Inst = mapIter->first;
			std::set<Instruction*>* succ = getSuccessors(Inst);
			bool allSuccSched = true;
			int min_succ_val = csteps + 1;
			for (std::set<Instruction*>::iterator iter=succ->begin(); iter != succ->end(); ++iter) {
				if ((*alapList)[*iter] == (csteps + 1)) {
					allSuccSched = false;
					break;
				} else if ((*alapList)[*iter] < min_succ_val) {
					min_succ_val = (*alapList)[*iter];
				}
			}
			if (allSuccSched == true) (*alapList)[Inst] = min_succ_val - 1;
		}
		j--;
	}

	return alapList;
}

// Function returns total number of cycles/control-steps used in ASAP scheduling
int fds::getTotalCSteps(std::map<Instruction*, int> asapList)
{
	int max_value = 0;
	for (std::map<Instruction*, int>::iterator mapIter=asapList.begin(); mapIter != asapList.end(); mapIter++) {
	    if (mapIter->second > max_value)
		max_value = mapIter->second; 
	}
	return max_value;
}

// Function returns a set of predecessors
std::set<Instruction*>* fds::getPredecessors(Instruction* Inst, std::map<Instruction*, int>* instList)
{
	std::set<Instruction*>* predSet = new std::set<Instruction*>();
	for (std::map<Instruction*, int>::iterator iter=instList->begin(); iter != instList->end(); ++iter) {
	    for (Use &U : (iter->first->uses())) {
		Instruction *User = cast<Instruction>(U.getUser());
		if (User->getName() == Inst->getName()) {
			predSet->insert(iter->first);
		}
	    }
	}
	return predSet;
}

// Function returns a set of successors
std::set<Instruction*>* fds::getSuccessors(Instruction* Inst)
{
	std::set<Instruction*>* succSet = new std::set<Instruction*>();
	for (Use &U : Inst->uses()) {
		Instruction *User = cast<Instruction>(U.getUser());
		succSet->insert(User);
	}
	return succSet;
}

char fds::ID = 0;
static RegisterPass<fds> X("fds", "FDS Scheduler");
