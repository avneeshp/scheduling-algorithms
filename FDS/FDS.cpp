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
#include <unistd.h>
#include <chrono>

#define MAX_VAL 100

using namespace llvm;

#define DEBUG_TYPE "fds"

namespace {

typedef enum ResType {
	NOT_SET = 0,
	MUL,
	ALU
} resource_type;

/**
 * @brief Declaration of operation class 
 * */
class Operation
{
    private:
	Instruction* m_inst;
	bool 	m_fixed;
	int 	m_earliest;
	int 	m_latest;
	resource_type m_type;

    public:
	Operation (Instruction* inst)
	{
		m_inst	   = inst;	
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
		if (s > m_latest ) return 0;
		if (s < m_earliest) return 0;

		double result = 1.0 / (double) (getMobility());

		return result;	//considering latency of each operation as 1 for now
	}

	bool equals (Operation* op)
	{
		if  (op == NULL) return false;
		if (op->op_id() != m_inst) return false;
		return true;
	}
};	
  
/**
 * @brief Declaration of Operation Graph class 
 * */
class OpGraph
{
    private:
	std::list <Operation*>* m_oplist;
	std::map <Operation*, std::list<Operation*>>* m_predmap;
	std::map <Operation*, std::list<Operation*>>* m_succmap;

    public:
	OpGraph ()
	{
	    m_oplist = new std::list <Operation*> ();
	    m_predmap = new std::map <Operation*, std::list<Operation*>> ();
	    m_succmap = new std::map <Operation*, std::list<Operation*>> ();
	}
	~OpGraph ()
	{
	    delete m_predmap;
	    delete m_succmap;
	    delete m_oplist;
	}

	// Member function to insert an operation in the operation list
	void insertOperation(Operation* op)
	{
	    m_oplist->push_back(op);
	}
	
	// Member function to get operation list
	std::list <Operation*>* getOperationList()
	{
	    return m_oplist;
	}
	
	// Member function to insert an operation and its predecessor list in the map
	void insertPredecessor(Operation* op, std::list<Operation*> predList)
	{
	    (*m_predmap)[op] = predList;
	}
	
	// Member function to insert an operation and its successor list in the map
	void insertSuccessor(Operation* op, std::list<Operation*> succList)
	{
	    (*m_succmap)[op] = succList;
	}

	// Member function to get a map of operations and their predecessors
	std::map <Operation*, std::list<Operation*>>* getPredecessorsMap ()
	{
	     return m_predmap;
	}
	
	// Member function to get a map of operations and their successors
	std::map <Operation*, std::list<Operation*>>* getSuccessorsMap ()
	{
	     return m_succmap;
	}
	
	// Member function to get list of unscheduled operations
	std::list <Operation*>* getUnscheduledOperations ()
	{
	     std::list <Operation*>* unschedOps = new std::list <Operation*> ();
	     for (std::list<Operation*>::iterator it = m_oplist->begin(); it != m_oplist->end(); it++)
	     {
		Operation *op = *it;
		if (op->isFixed() == true) continue;
	 	if (op->getMobility() <= 1) continue;
		unschedOps->push_back(op);
	     }
	     return unschedOps;
	}
	void updateTimeFrames (int time_constraint)
	{
		std::list<Operation*>* unschedOps = getUnscheduledOperations();
		bool allPredPlanned;
		bool allSuccPlanned;

		//ASAP schedules
		for (std::list<Operation*>::iterator it = unschedOps->begin(); it != unschedOps->end();)
		{
			Operation* op = *it++;
			std::list <Operation*> predlist = (*m_predmap)[op];
			if (predlist.empty()) {
				unschedOps->remove(op);
			}
				
		}
		while (unschedOps->size() > 0)
		{
			for (std::list<Operation*>::iterator it = unschedOps->begin(); it != unschedOps->end();)
			{
				Operation* op = *it++;
				allPredPlanned = true;
				std::list<Operation*> predlist = (*m_predmap)[op];
				
				for (std::list<Operation*>::iterator iter = predlist.begin(); iter != predlist.end(); iter++) {
					Operation* pred = *iter;
					std::list<Operation*>::iterator tmpIter = find(unschedOps->begin(), unschedOps->end(), pred);
					if (tmpIter != unschedOps->end())
						allPredPlanned = false;
				}
				if (allPredPlanned)
				{
					int start_time = calculateEarliestStart(op) + 1;
					op->setEarliest(start_time);
					unschedOps->remove(op);
				}
			}
		}

		//ALAP schedules
		std::list<Operation*>* unplannedOps = getUnscheduledOperations();
		for (std::list<Operation*>::iterator it = unplannedOps->begin(); it != unplannedOps->end();)
		{
			Operation* op = *it++;
			std::list <Operation*> succlist = (*m_succmap)[op];
			if (succlist.empty()) {
				unplannedOps->remove(op);
			}
				
		}
		while (unplannedOps->size() > 0)
		{
			for (std::list<Operation*>::iterator it = unplannedOps->begin(); it != unplannedOps->end();)
			{
				Operation* op = *it++;
				allSuccPlanned = true;
				std::list<Operation*> succlist = (*m_succmap)[op];
				
				for (std::list<Operation*>::iterator iter = succlist.begin(); iter != succlist.end(); iter++) {
					Operation* succ = *iter;
					std::list<Operation*>::iterator tmpIter = find(unplannedOps->begin(), unplannedOps->end(), succ);
					if (tmpIter != unplannedOps->end())
						allSuccPlanned = false;
				}
				if (allSuccPlanned)
				{
					int end_time = calculateLatestEnd(op) - 1;
					op->setLatest(end_time);
					unplannedOps->remove(op);
				}
			}
		}	
	}
	int calculateLatestEnd(Operation* op)
	{
		std::list<Operation*> succlist = (*m_succmap)[op];
		int min_t = succlist.front()->getLatest();
		
		for (std::list<Operation*>::iterator it = succlist.begin(); it != succlist.end(); it++) {
			Operation* succ = *it;
			if(succ->getLatest() < min_t){
				min_t = succ->getLatest();
			}
		}
		return min_t;
	}
	
	int calculateEarliestStart( Operation* op)
	{
		std::list<Operation*> predlist = (*m_predmap)[op];
		int max_t = 0;
		
		for (std::list<Operation*>::iterator it = predlist.begin(); it != predlist.end(); it++) {
			Operation* pred = *it;
			if(pred->getEarliest() > max_t){
				max_t = pred->getEarliest();
			}
		}
		return max_t;
	}

};

/**
 * @brief Declaration of Distribution Graph class 
 * */
class DistGraph {
    private:
	// Member represents the type of function unit
	resource_type m_rtype;
	std::list <Operation*>* m_operations;
	std::map <Operation*, double>* m_avg_resource_usage;

    public:
	DistGraph (resource_type rtype)
	{
		m_rtype = rtype;
		m_operations = new std::list <Operation*>();
		m_avg_resource_usage = new std::map <Operation*, double>();
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
		m_operations->push_back(op);
		return;
	}

	std::list<Operation*>* getOp() 
	{
		return m_operations;
	}

	// Member function to calculate the input function unit usage in input time step
	double getResourceUsage(resource_type rtype, int cstep) {
		if (m_rtype != rtype) return 0.0;
		double r_usage = 0.0;
		for (std::list<Operation*>::iterator opIter = m_operations->begin(); opIter != m_operations->end(); ++opIter) {
			r_usage += (*opIter)->getProbability (cstep);
		}
		return r_usage;
		
	}

	void setAvgResourceUsage(resource_type rtype, Operation* op) {
		double avg_usage = 0.0;
		int start = op->getEarliest();
		int end = op->getLatest();
		for (int i = start; (i <= end); i++) {
			avg_usage += getResourceUsage(rtype, i);
		}
		avg_usage /= ((double) (end - start + 1));
		(*m_avg_resource_usage)[op] = avg_usage;
		return;
	}
	
	double getAvgResourceUsage (Operation* op)
	{
		double avg_rusage = (*m_avg_resource_usage)[op];
		return avg_rusage;
	}

	// Member function to get self force for a resource at a particular time step
	double selfForce(Operation* op, int cstep) {
		double sForce = 0.0;
		resource_type rtype = op->getResourceType();
		int start = op->getEarliest();
		int end = op->getLatest();
		if ((cstep < start) || (cstep > end)) return 0.0;
		double rusage_at_time = getResourceUsage(rtype, cstep);
		double avg_rusage = getAvgResourceUsage(op);
		sForce = rusage_at_time - avg_rusage;
		return sForce;
	}
	
	// Member function to get successor force for an operation at a particular time step 
	double succForce(Operation* succ, int cstep) {
		if (succ->getMobility() == 1) return 0.0;
		int start_time = cstep + 1;

		int prev_start_time = succ->getEarliest();
		succ->setEarliest(start_time);
		
		double succForce = 0.0;
		resource_type rtype = succ->getResourceType();
		double rusage_wrt_time = getResourceUsage(rtype, cstep);
		double avg_rusage = getAvgResourceUsage(succ);
		succForce = rusage_wrt_time - avg_rusage;

		succ->setEarliest(prev_start_time);
		return succForce;
	}

	// Member function to get predecessor force for an operation at a particular time step
	double predForce(Operation* pred, int cstep) {
		if (pred->getMobility() == 1) return 0.0;
		int end_time   = cstep - 1;

		int prev_latest_time = pred->getLatest();
		pred->setLatest(end_time);
		
		double predForce = 0.0;
		resource_type rtype = pred->getResourceType();
		double rusage_at_time = getResourceUsage(rtype, cstep);
		double avg_rusage = getAvgResourceUsage(pred);
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
    void printFDSCycleTime(std::list <Operation*>*, int, std::ofstream&);
    
    bool runOnBasicBlock(BasicBlock &BB) override {
	float in;
	auto start = std::chrono::high_resolution_clock::now();
	char* cwd = get_current_dir_name();
	const char* fname = "/cycles.txt";
	int length = strlen(cwd) + strlen(fname);
	char* cyc_path = (char*)malloc(length);
	sprintf(cyc_path, "%s%s", cwd, fname);
	std::ifstream input_file;
	input_file.open (cyc_path);
	input_file >> in;
	input_file.close();
	std::map <Instruction* , int>* asapSchedList = ASAP(BB);
	int asap_csteps = getTotalCSteps(*asapSchedList);
	int csteps = in * asap_csteps;
	std::map <Instruction* , int>* alapSchedList = ALAP(BB, csteps);
	std::list <Operation*> OpList;
	OpGraph* operationGraph = new OpGraph();
	for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
		Instruction *Inst = DI++;
		int i = Inst->getNumOperands();
		if (i > 0) {
			Operation* Op = new Operation(Inst);
			resource_type rtype = fuType(Inst);
			if ((rtype == MUL) || (rtype == ALU)) {
				Op->setEarliest((*asapSchedList)[Inst]);
				Op->setLatest((*alapSchedList)[Inst]);
				if ((*asapSchedList)[Inst] == (*alapSchedList)[Inst])
					Op->setFixed();
			} else {
				//Scheduling all stores instructions in same cycle as ALAP for now
				Op->setEarliest((*alapSchedList)[Inst]);
				Op->setLatest((*alapSchedList)[Inst]);
				Op->setFixed();
			}	
			Op->setResourceType(rtype);
			OpList.push_back(Op);
			operationGraph->insertOperation(Op);
		}
	}
	std::list <Operation*>* OperationList = operationGraph->getOperationList();
	
	for (std::list<Operation*>::iterator opIter = OpList.begin(); opIter != OpList.end(); ++opIter) {
		Operation* op = *opIter;
		Instruction* inst = op->op_id();
		std::set<Instruction*>* succ = getSuccessors(inst);
		std::list <Operation*> succList;
		for (std::set<Instruction*>::iterator it = succ->begin(); it != succ->end(); it++) {
			Instruction *Inst = *it;
			for (std::list<Operation*>::iterator iter = OperationList->begin(); iter != OperationList->end(); iter++) {
				Operation* oper = *iter;
				if (oper->op_id() != Inst) continue;
				succList.push_back(oper);
			}
		}
		operationGraph->insertSuccessor(op, succList);
	}
	for (std::list<Operation*>::iterator opIter = OpList.begin(); opIter != OpList.end(); ++opIter) {
		Operation* op = *opIter;
		Instruction* inst = op->op_id();
		std::set<Instruction*>* pred = getPredecessors(inst, asapSchedList);
		std::list <Operation*> predList;
		for (std::set<Instruction*>::iterator it = pred->begin(); it != pred->end(); it++) {
			Instruction *Inst = *it;
			for (std::list<Operation*>::iterator iter = OperationList->begin(); iter != OperationList->end(); iter++) {
				Operation* oper = *iter;
				if (oper->op_id() != Inst) continue;
				predList.push_back(oper);
			}
		}
		operationGraph->insertPredecessor(op, predList);
	}
	std::map <Operation*, std::list<Operation*>>* succMap = operationGraph->getSuccessorsMap();
	std::map <Operation*, std::list<Operation*>>* predMap = operationGraph->getPredecessorsMap();
	
	//Distribution graph for multiply
	resource_type multiplier = MUL;
	DistGraph* dGraphMul = new DistGraph(multiplier);
	for (std::list<Operation*>::iterator opIter = OperationList->begin(); opIter != OperationList->end(); ++opIter) {
		if ((*opIter)->getOpType() == MUL) {
		   dGraphMul->insertOp(*opIter);
		   dGraphMul->setAvgResourceUsage(multiplier, *opIter);
		}
	}
	
	//Distribution graph for ALU
	resource_type add_sub = ALU;
	DistGraph* dGraphALU = new DistGraph(add_sub);
	for (std::list<Operation*>::iterator opIter = OperationList->begin(); opIter != OperationList->end(); ++opIter) {
		if ((*opIter)->getOpType() == ALU) {
		   dGraphALU->insertOp(*opIter);
		   dGraphALU->setAvgResourceUsage(add_sub, *opIter);
		}
	}
	
	while (1) {
		operationGraph->updateTimeFrames(csteps);
		std::list <Operation*>* unschedOps  = operationGraph->getUnscheduledOperations();
		if (unschedOps->size() <= 0) {
			delete unschedOps;
			break;
		}
		Operation* minOperation = NULL;
		double min_force = MAX_VAL;
		int scheduled_step = -1;
		for (std::list <Operation*>::iterator it = unschedOps->begin(); it != unschedOps->end(); it++) {
			Operation* Op = *it;
			for (int step = Op->getEarliest(); step <= Op->getLatest(); step++) {
				double selfForce;
				double sumPredForce = 0.0;
				double sumSuccForce = 0.0;
				resource_type rtype = Op->getResourceType();
				if (rtype == MUL) {
					selfForce = dGraphMul->selfForce(Op, step);
					std::list<Operation*> succList = (*succMap)[Op];
					for (std::list<Operation*>::iterator iter = succList.begin(); iter != succList.end(); iter++) {
						Operation* succ = *iter;
						if (succ->getResourceType() == MUL)
							sumSuccForce += dGraphMul->succForce(succ, step);
						else
							sumSuccForce += dGraphALU->succForce(succ, step);
					}
					std::list<Operation*> predList = (*predMap)[Op];
					for (std::list<Operation*>::iterator iter = predList.begin(); iter != predList.end(); iter++) {
						Operation* pred = *iter;
						if (pred->getResourceType() == MUL)
							sumPredForce += dGraphMul->predForce(pred, step);
						else
							sumPredForce += dGraphALU->predForce(pred, step);
					}
				}	
				if (rtype == ALU) {
					selfForce = dGraphALU->selfForce(Op, step);	
					std::list<Operation*> succList = (*succMap)[Op];
					for (std::list<Operation*>::iterator iter = succList.begin(); iter != succList.end(); iter++) {
						Operation* succ = *iter;
						if (succ->getResourceType() == MUL)
							sumSuccForce += dGraphMul->succForce(succ, step);
						else
							sumSuccForce += dGraphALU->succForce(succ, step);
					}
					std::list<Operation*> predList = (*predMap)[Op];
					for (std::list<Operation*>::iterator iter = predList.begin(); iter != predList.end(); iter++) {
						Operation* pred = *iter;
						if (pred->getResourceType() == MUL)
							sumPredForce += dGraphMul->predForce(pred, step);
						else
							sumPredForce += dGraphALU->predForce(pred, step);
					}
				}
				double current_force = selfForce + sumPredForce + sumSuccForce;

				if (current_force < min_force) {
					min_force = current_force;
					scheduled_step = step;
					minOperation = Op;
				}
			}
		}		
		minOperation->setEarliest(scheduled_step);
		minOperation->setLatest(scheduled_step);
		minOperation->setFixed();
		delete unschedOps;
	}
	const char* filename = "/schedule.txt";
	int len = strlen(cwd) + strlen(filename);
	char* path = (char*)malloc(len);
	sprintf(path, "%s%s", cwd, filename);
	std::ofstream myfile;
	myfile.open (path);
	
	myfile << "\n\n***** FDS Scheduling ***** \n\n";
	printFDSCycleTime(OperationList, csteps, myfile); 
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	myfile << "Execution Time: " << microseconds << "\n";
	myfile.close();
	if (NULL != cwd) free (cwd);
	if (NULL != path) free(path);
	if (NULL != cyc_path) free(cyc_path);
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
	int max_addsub = 0;
	int max_mul = 0;
	int max_value = getTotalCSteps(opMap);
	Instruction *Inst;
	while (cycle_num <= max_value) {
		int num_of_addsub = 0;
		int num_of_mul = 0;
		myfile << "[ Cycle Time " << cycle_num << " ]\n";
		for (std::map<Instruction*, int>::iterator mapIter = opMap.begin(); mapIter != opMap.end(); ++mapIter) {
			Inst = mapIter->first;
			std::string str;
			llvm::raw_string_ostream rso(str);
			Inst->print(rso);
			if (mapIter->second == cycle_num) {
				myfile << str << "\n";
				if ((Inst->getOpcode() == Instruction::Add) || (Inst->getOpcode() == Instruction::Sub))
					num_of_addsub += 1;
				if (Inst->getOpcode() == Instruction::Mul)
					num_of_mul += 1;
			}
		}
		if (max_mul < num_of_mul)
			max_mul = num_of_mul;
		if (max_addsub < num_of_addsub)
			max_addsub = num_of_addsub;
		cycle_num = cycle_num + 1;
	}
	myfile << "\nNumber of AddSub Required : " << max_addsub << "\n";
	myfile << "\nNumber of Multipliers Required : " << max_mul << "\n";
	return;
}

void fds::printFDSCycleTime(std::list <Operation*>*opList, int max_value, std::ofstream& myfile) {
	int cycle_num = 1;
	Instruction *Inst;
	int max_addsub = 0;
	int max_mul = 0;
	while (cycle_num <= max_value) {
		int num_of_addsub = 0;
		int num_of_mul = 0;
		myfile << "[ Cycle Time " << cycle_num << " ]\n";
		for (std::list <Operation*>::iterator opIter = opList->begin(); opIter != opList->end(); ++opIter) {
			if ((*opIter)->getEarliest() == cycle_num) {
				if ((*opIter)->getResourceType() == MUL)
					num_of_mul += 1;
				if ((*opIter)->getResourceType() == ALU)
					num_of_addsub += 1;
				Inst = (*opIter)->op_id();
				std::string str;
				llvm::raw_string_ostream rso(str);
				Inst->print(rso);
				myfile << str << "\n";
			}
		}
		if (max_mul < num_of_mul)
			max_mul = num_of_mul;
		if (max_addsub < num_of_addsub)
			max_addsub = num_of_addsub;
		cycle_num = cycle_num + 1;
	}
	myfile << "\nNumber of AddSub Required : " << max_addsub << "\n";
	myfile << "\nNumber of Multipliers Required : " << max_mul << "\n";
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
		if (User == Inst) {
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
