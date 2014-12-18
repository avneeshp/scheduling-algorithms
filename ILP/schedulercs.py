#!/usr/bin/env python

# Tests accessing of instruction operands.
import sys
import os
import operator
import llvm 
import time
# top-level, for common stuff
from llvm.core import *
from copy import deepcopy
from lpsolve55 import *

#===----------------------------------------------------------------------===
# Get the name of an LLVM value
def get_name(val) :
    if (not isinstance(val, Value)):
        return ''
    if isinstance(val, Argument):
        #return val.name + " [in]" 
        return val.name
    if isinstance(val, GlobalVariable):
        #return val.name + " [out]" 
        return val.name 
    if isinstance(val, Instruction):
        if val.opcode_name == 'store':
           return "[store]" 
    if isinstance(val, ConstantInt):
        return str(val.z_ext_value)
    return val.name

# Convenience functions
def keywithmaxval(d):
     v=list(d.values())
     k=list(d.keys())
     return v.index(max(v))

# Is the value a store?
def is_store(val) :
    if not isinstance(val, Instruction):
        return 0
    return val.opcode_name == 'store'

# Is the value a return?
def is_return(val) :
    if not isinstance(val, Instruction):
        return 0 
    return val.opcode_name == 'ret'

# Is the value a multiplication?
def is_mul(val) :
    if not isinstance(val, Instruction):
        return 0
    if not val.is_binary_op:
        return 0
    return val.opcode_name == 'mul'

# Is the value an addition or a subtraction?
def is_addsub(val) :
    if not isinstance(val, Instruction):
        return 0
    if not val.is_binary_op:
        return 0
    opc = val.opcode_name 
    return opc == 'add' or opc == 'sub' 

# Get the pretty string in C-like syntax of an LLVM value
def to_string(val) :
    # Is an Instruction?
    if isinstance(val, Instruction): 
        opc = val.opcode_name
        # Get the first operand if there is any
        if val.operand_count > 0:
            op0 = val.operands[0]
        # Get the second operand if there is any
        if val.operand_count > 1:
            op1 = val.operands[1]
        # Binary operation
        if val.is_binary_op:
            opc_map = {'add':'+', 'sub':'-', 'mul':'*'}
            if (opc in opc_map): opc = opc_map[opc]
            # Generate string in C-like syntax
            return get_name(val) + ' = ' + get_name(op0) + ' ' + opc + ' ' + get_name(op1)
        # Store operation
        elif opc == 'store':
            # Generate string in C-like syntax
            return '*' + get_name(op1) + ' = ' + get_name(op0)
        # Store operation
        elif opc == 'ret':
            # Generate string in C-like syntax
            return 'return'
        else:
            return opc
    # Is a Constant?
    elif isinstance(val, ConstantInt):
        return get_name(val)
    return '' 

# Main function
def run(testcase):
    f = open(testcase)
    m = Module.from_assembly(f)
    dut_name = os.path.splitext(os.path.basename(testcase))[0]
    dut = m.get_function_named(dut_name)

    MUL_C                = 3
    ADDSUB_C             = 3
    DEBUG_PRINT          = 0
    FACTOR               = 2.5
    TIMEOUT              = 60           # lpsolve timeout in secs
    TCS_TO_TEST          = 11           # verify whether the operations can be scheduled in these many cycles given res constraint
    #===------------------------------ASAP SCHEDULING-----------------------------===
    # Print instructions in DFG under test 
    idx = 0
    maxi = 0
    no_of_inst = 0
    cycles_asap = {to_string(inst):0 for inst in dut.basic_blocks[0].instructions}
    for inst in dut.basic_blocks[0].instructions:
        # Print its operands in short name 
        no_of_inst = no_of_inst + 1
        operands = "  - Operands: "
        temp1 = 1
        for o in inst.operands:
			operands += get_name(o) + "; "
			for inst1 in dut.basic_blocks[0].instructions:
				x = to_string(inst1).find(" =")
				x = to_string(inst1)[0:x]
				if x == get_name(o):temp1=0
        # Zero operands?
        if len(inst.operands) <= 0:
            operands += "NONE"
        if temp1 == 1:cycles_asap[to_string(inst)]=1
        # Print its uses in pretty form
        uses = "  - Uses: "
        for u in inst.uses:
            uses += to_string(u) + "; "
            if cycles_asap[to_string(inst)] >= cycles_asap[to_string(u)]:
                cycles_asap[to_string(u)]=cycles_asap[to_string(inst)]+1
        # Zero uses?
        if len(inst.uses) <= 0:
            uses += "NONE"
        idx += 1
        maxi = max(maxi, cycles_asap[to_string(inst)])
        if to_string(inst).find("ret") != -1:cycles_asap[to_string(inst)]=maxi
    f.close()
    fp = open('schedule.txt', 'w')
    if DEBUG_PRINT == 1:
      fp.write('------------------------ASAP-------------------')
      fp.write('\n')
    #===------------------------------ASAP -----------------------------===
      for i in range(1, maxi+1):
        cyc = "-----Cycle " + str(i) + " ------"
        fp.write('\n')
        fp.write(cyc)
        fp.write('\n')
        for inst in dut.basic_blocks[0].instructions:
            if cycles_asap[to_string(inst)] == i and to_string(inst)!="return":            
                fp.write(to_string(inst))
                fp.write('\n')
      fp.write('\n')
      fp.write('\n')
    #===------------------------------ALAP SCHEDULING-----------------------------===
    f = open(testcase)
    m = Module.from_assembly(f)
    dut_name = os.path.splitext(os.path.basename(testcase))[0]
    dut = m.get_function_named(dut_name)

    maxi = TCS_TO_TEST;

    cycles_alap = {to_string(inst):(maxi+1) for inst in dut.basic_blocks[0].instructions}
    for inst in reversed(dut.basic_blocks[0].instructions):
        # Print its operands in short name 
        operands = "  - Operands: "
        for o in inst.operands:
			operands += get_name(o) + "; "
        # Zero operands?
        if len(inst.operands) <= 0:
            operands += "NONE"
        # Print its uses in pretty form
        uses = "  - Uses: "
        for u in inst.uses:
            uses += to_string(u) + "; "
        # Zero uses?
        if len(inst.uses) <= 0:
            uses += "NONE"
            cycles_alap[to_string(inst)]=maxi;
        for inst1 in dut.basic_blocks[0].instructions:
            for u in inst1.uses:
                if to_string(u) == to_string(inst):
                   if (cycles_alap[to_string(inst1)]) >  (cycles_alap[to_string(inst)] - 1):
                       cycles_alap[to_string(inst1)] =  cycles_alap[to_string(inst)] - 1
    f.close()
    if DEBUG_PRINT == 1:
      fp.write('------------------------ALAP-------------------')
      fp.write('\n')
    #===------------------------------ALAP --------------------------===
      for i in range(1, maxi+1):
        cyc = "-----Cycle " + str(i) + " ------"
        fp.write('\n')
        fp.write(cyc)
        fp.write('\n')
        for inst in dut.basic_blocks[0].instructions:
            if cycles_alap[to_string(inst)] == i and to_string(inst)!="return":            
                fp.write(to_string(inst))
                fp.write('\n')
      fp.write('\n')
      fp.write('\n')
   
    start_time = time.time();

    unique_start_time = [[0 for x in range(maxi)] for x in range(no_of_inst-1)]
    mul_constraint    = [[0 for x in range(maxi)] for x in range(no_of_inst-1)]
    addsub_constraint = [[0 for x in range(maxi)] for x in range(no_of_inst-1)]

    #===--------------------Unique Start Time Constraint--------------------------===
    if DEBUG_PRINT == 1:
      print '------Start Time Constraint-------'
    counter = 0
    for inst in dut.basic_blocks[0].instructions:
        if to_string(inst)!="return":
          if DEBUG_PRINT == 1:
            print to_string(inst)
          for i in range(cycles_asap[to_string(inst)], cycles_alap[to_string(inst)]+1):
            unique_start_time[counter][i-1] = 1
            mul_constraint[counter][i-1] = 1
            addsub_constraint[counter][i-1] = 1
          counter = counter + 1
    if DEBUG_PRINT == 1:
      print unique_start_time

    #===---------------------Resource Constraint--------------------------===
    counter           = 0
    for inst in dut.basic_blocks[0].instructions:
      if is_mul(inst):
        for i in range(0, maxi):
           addsub_constraint[counter][i] = 0
      elif is_addsub(inst):
        for i in range(0, maxi):
           mul_constraint[counter][i] = 0
      elif to_string(inst) !="return":
        for i in range(0, maxi):
           mul_constraint[counter][i] = 0
           addsub_constraint[counter][i] = 0
      counter = counter + 1

    if DEBUG_PRINT == 1:
      print '------Resource Constraint-------'
      print mul_constraint
      print addsub_constraint
   
    dependence_constraint = [[0 for x in range(maxi)] for x in range(no_of_inst-1)]
    lp = lpsolve('make_lp', 0, (no_of_inst-1) * maxi)
    #===---------------------Dependence Constraint------------------------===
    if DEBUG_PRINT == 1:
      print '------Dependence Constraint-------'
    for inst in dut.basic_blocks[0].instructions:
      if (cycles_asap[to_string(inst)] != cycles_alap[to_string(inst)]) and (len(inst.uses)>0):
          dependence_constraint = [[0 for x in range(maxi)] for x in range(no_of_inst-1)]
          count = 0
          for inst1 in dut.basic_blocks[0].instructions:
            if to_string(inst) == to_string(inst1):
              a1 = count
              break
            count = count+1
          for u in inst.uses:
            dependence_constraint = [[0 for x in range(maxi)] for x in range(no_of_inst-1)]
            for i in range(cycles_asap[to_string(inst)], cycles_alap[to_string(inst)]+1):
              dependence_constraint[a1][i-1]=i
            count = 0
            for inst1 in dut.basic_blocks[0].instructions:
              if to_string(u) == to_string(inst1):
                a2 = count
                break
              count = count+1
            for i in range(cycles_asap[to_string(u)], cycles_alap[to_string(u)]+1):
              dependence_constraint[a2][i-1]=-i
            d_add  = [0] * (no_of_inst-1)*maxi
            for i in range(0, maxi):
              for j in range(0, no_of_inst-1):
                k =(maxi*j + i)
                d_add[k] = dependence_constraint[j][i]
            ret = lpsolve('add_constraint', lp, d_add, LE, -1)
            if DEBUG_PRINT == 1:
              print dependence_constraint

    no_of_inst = no_of_inst-1
    for i in range(1,(no_of_inst*maxi)):
      ret = lpsolve('set_binary', lp, i, 1)
    #===---------------------Add Unique Start Time Constraint------------------------===
    b  = {1 for k in range(0, no_of_inst)}
    for i in range(0, no_of_inst):
      x = [0] * ((i*maxi))
      y = unique_start_time[i]
      v = [1] * maxi
      z = [0] * ((no_of_inst*maxi - maxi - i*maxi))
      w = x+y+z
      if DEBUG_PRINT == 1:
        print w
      ret = lpsolve('add_constraint', lp, w, EQ, 1)
      w = x+v+z
      ret = lpsolve('add_constraint', lp, w, EQ, 1)
      if DEBUG_PRINT == 1:
        print w
        print x
        print y
        print z
    
    d_add  = [0] * no_of_inst*maxi
    if DEBUG_PRINT == 1:
      print 'add res const' 
    #===---------------------Add Resource Constraint------------------------===
    for i in range(0, maxi):
      b_mul  = [0] * no_of_inst*maxi
      b_add  = [0] * no_of_inst*maxi
      for j in range(0, no_of_inst):
        k =(maxi*j + i)
        b_mul[k] = mul_constraint[j][i]
        b_add[k] = addsub_constraint[j][i]
        d_add[k] = dependence_constraint[j][i]
      ret = lpsolve('add_constraint', lp, b_mul, LE, MUL_C)
      ret = lpsolve('add_constraint', lp, b_add, LE, ADDSUB_C)
      if DEBUG_PRINT == 1:
        print b_add
        print b_addd

    obj = lpsolve('set_timeout',lp, TIMEOUT)
    obj = lpsolve('solve',lp)
    var = lpsolve('get_variables', lp)
    
    if DEBUG_PRINT == 1:
        print var
   
    print ' \n \n '
    message = " Time taken to execute = " + str(time.time() - start_time) + " seconds"
    print message
    fp.write('------------------------ILP-------------------')
    fp.write('\n')
    cycle_count = 1
    #===------------------------------ILP--------------------------===
    for i in range(1, maxi+1):
        cyc = "-----Cycle " + str(cycle_count) + " ------"
        fp.write('\n')
        fp.write(cyc)
        fp.write('\n')
        flag = 0
        for j in range(0, no_of_inst):
          if var[0][maxi*j + i-1] != 0.0:
            flag = 1
            k = 0
            for inst in dut.basic_blocks[0].instructions:
              if k == j:
                fp.write(to_string(inst))
                fp.write('\n')
                break
              else:
                k = k + 1
        if flag == 1:
         cycle_count = cycle_count + 1 

    message = " Cycles Required " + str(cycle_count-1)
    print message
    
    fp.write('\n')
    fp.write('\n')
    fp.close()

# Prompt CLI usage
if len(sys.argv) < 2:
    sys.exit('Usage: python %s <test>.ll' % sys.argv[0])
# Test exists?
elif not os.path.exists(sys.argv[1]):
    sys.exit('Cannot locate specified test case %s' % sys.argv[1])

run(sys.argv[1])
