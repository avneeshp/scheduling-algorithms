#!/usr/bin/env python

# Tests accessing of instruction operands.
import sys
import os
import operator
import llvm 
# top-level, for common stuff
from llvm.core import *
from copy import deepcopy
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

    #===------------------------------ASAP SCHEDULING-----------------------------===
    # Print instructions in DFG under test 
    idx = 0
    maxi = 0
    #for inst in dut.basic_blocks[0].instructions:
    cycles_asap = {to_string(inst):0 for inst in dut.basic_blocks[0].instructions}
    for inst in dut.basic_blocks[0].instructions:
        # Print its operands in short name 
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
    maxaddsub = 0
    maxmul = 0
    addsub, mul = [0] * 100, [0] * 100
    fp.write('------------------------ASAP-------------------')
    fp.write('\n')
    #===------------------------------ASAP FUNCTIONAL UNIT CALCULATION-----------------------------===
    for i in range(1, maxi+1):
        cyc = "-----Cycle " + str(i) + " ------"
        fp.write('\n')
        fp.write(cyc)
        fp.write('\n')
        for inst in dut.basic_blocks[0].instructions:
            if cycles_asap[to_string(inst)] == i and to_string(inst)!="return":            
                fp.write(to_string(inst))
                fp.write('\n')
            if is_addsub(inst):
                addsub[cycles_asap[to_string(inst)]] =  addsub[cycles_asap[to_string(inst)]] + 1
            if is_mul(inst):
                mul[cycles_asap[to_string(inst)]] =  mul[cycles_asap[to_string(inst)]] + 1
    for i in range(1, 100):
	if addsub[i] > maxaddsub:
		maxaddsub = addsub[i]

	if mul[i] > maxmul:
		maxmul = mul[i]
    maxaddsub = maxaddsub/maxi;
    maxmul = maxmul/maxi;
    maxmul = str(maxmul);
    maxaddsub = str(maxaddsub);
    fp.write('\n')
    fp.write('\n')
    fp.write('----------RESOURCES---------')
    fp.write('\n')
    fp.write('ADDSUB:')
    fp.write(maxaddsub)
    fp.write('\n')
    fp.write('MUL:')
    fp.write(maxmul)
    fp.write('\n')
    #===------------------------------ASAP REGISTER UNITS CALCULATION-----------------------------===
    f = open(testcase)
    m = Module.from_assembly(f)
    dut_name = os.path.splitext(os.path.basename(testcase))[0]
    dut = m.get_function_named(dut_name)
    birth = {}
    death = {}
    lifetime = dict()
    for inst in dut.basic_blocks[0].instructions:
          x = to_string(inst).find(" =")
          output = to_string(inst)[0:x]
          for inst1 in dut.basic_blocks[0].instructions:
                   x1 = to_string(inst1).find(" =")
                   output1 = to_string(inst1)[0:x1]
                   if output == output1:
                       if output in birth: 
                             if birth[output] > cycles_asap[to_string(inst1)]:                       
                                 birth[output]=cycles_asap[to_string(inst1)]
                                 lifetime[output]=[(cycles_asap[to_string(inst1)])]
                    
                       else: 
                                 birth[output]=cycles_asap[to_string(inst1)]
                                 lifetime[output]=[(cycles_asap[to_string(inst1)])]
                   for o in inst1.operands:
                        if output == get_name(o):
                             goo = 0
                             if output in death: 
                                  if death[output] < cycles_asap[to_string(inst1)]:                       
                                       death[output]=cycles_asap[to_string(inst1)]
                                       lifetime[output][1]=death[output]
                             else: 
                                       death[output]=cycles_asap[to_string(inst1)]
                                       lifetime[output].append(death[output])
    lif = deepcopy(lifetime)
    for key in lif:
           if len(lif[key]) == 1: del lifetime[key]
    sorted_lifetime = sorted(lifetime.items(), key=operator.itemgetter(1))
    resultnew = sorted(lifetime.items(), reverse=True, key=lambda x: (-x[1][0], x[1][1]))
    registers = 0
    while len(resultnew) != 0:
        registers = registers + 1
        curr_var = resultnew[0][0]
        last = 0
        flag = 0
        while flag==0:
              keyss = [item[0] for item in resultnew]
              n = len(keyss)
              for i in range(n):
                   if keyss[i] == curr_var:
                        start = resultnew[i][1][0]
                        break
              if start >= last : 
                    last = resultnew[i][1][1]
                    temp_var = curr_var
                    for i in range(n):
                        if keyss[i] == curr_var:
                              if i == (n-1):
                                  flag = 1
                              else: 
                                     curr_var = resultnew[i + 1][0]
                              break
                    for i in range(n):
                        if keyss[i] == temp_var:
                              del resultnew[i]
                              break
              else:     
                    for i in range(n):
                        if keyss[i] == curr_var:
                              if i == (n-1):
                                  flag = 1
                              else:  curr_var = resultnew[i + 1][0]
                              break
    fp.write('Registers:')
    fp.write(str(registers))
    fp.write('\n')
    fp.write('\n')
    fp.write('\n')
    fp.write('\n')
    #===------------------------------ALAP SCHEDULING-----------------------------===
    f = open(testcase)
    m = Module.from_assembly(f)
    dut_name = os.path.splitext(os.path.basename(testcase))[0]
    dut = m.get_function_named(dut_name)

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
    fp.write('------------------------ALAP-------------------')
    fp.write('\n')
    maxaddsub = 0
    maxmul = 0
    addsub, mul = [0] * 100, [0] * 100
    #===------------------------------ALAP FUNCTIONAL UNIT CALCULATION-----------------------------===
    for i in range(1, maxi+1):
        cyc = "-----Cycle " + str(i) + " ------"
        fp.write('\n')
        fp.write(cyc)
        fp.write('\n')
        for inst in dut.basic_blocks[0].instructions:
            if cycles_alap[to_string(inst)] == i and to_string(inst)!="return":            
                fp.write(to_string(inst))
                fp.write('\n')
            if is_addsub(inst):
                addsub[cycles_alap[to_string(inst)]] =  addsub[cycles_alap[to_string(inst)]] + 1
            if is_mul(inst):
                mul[cycles_alap[to_string(inst)]] =  mul[cycles_alap[to_string(inst)]] + 1
    for i in range(1, 100):
	if addsub[i] > maxaddsub:
		maxaddsub = addsub[i]

	if mul[i] > maxmul:
		maxmul = mul[i]
    maxaddsub = maxaddsub/maxi;
    maxmul = maxmul/maxi;
    maxmul = str(maxmul);
    maxaddsub = str(maxaddsub);
    fp.write('\n')
    fp.write('\n')
    fp.write('----------RESOURCES---------')
    fp.write('\n')
    fp.write('ADDSUB:')
    fp.write(maxaddsub)
    fp.write('\n')
    fp.write('MUL:')
    fp.write(maxmul)
    fp.write('\n')
    #===------------------------------ALAP REGISTER UNITS CALCULATION-----------------------------===
    f = open(testcase)
    m = Module.from_assembly(f)
    dut_name = os.path.splitext(os.path.basename(testcase))[0]
    dut = m.get_function_named(dut_name)
    birth = {}
    death = {}
    lifetime = dict()
    for inst in dut.basic_blocks[0].instructions:
          x = to_string(inst).find(" =")
          output = to_string(inst)[0:x]
          for inst1 in dut.basic_blocks[0].instructions:
                   x1 = to_string(inst1).find(" =")
                   output1 = to_string(inst1)[0:x1]
                   if output == output1:
                       if output in birth: 
                             if birth[output] > cycles_alap[to_string(inst1)]:                       
                                 birth[output]=cycles_alap[to_string(inst1)]
                                 lifetime[output]=[(cycles_alap[to_string(inst1)])]
                    
                       else: 
                                 birth[output]=cycles_alap[to_string(inst1)]
                                 lifetime[output]=[(cycles_alap[to_string(inst1)])]
                   for o in inst1.operands:
                        if output == get_name(o):
                             goo = 0
                             if output in death: 
                                  if death[output] < cycles_alap[to_string(inst1)]:                       
                                       death[output]=cycles_alap[to_string(inst1)]
                                       lifetime[output][1]=death[output]
                             else: 
                                       death[output]=cycles_alap[to_string(inst1)]
                                       lifetime[output].append(death[output])
    lif = deepcopy(lifetime)
    for key in lif:
           if len(lif[key]) == 1: del lifetime[key]
    sorted_lifetime = sorted(lifetime.items(), key=operator.itemgetter(1))
    resultnew = sorted(lifetime.items(), reverse=True, key=lambda x: (-x[1][0], x[1][1]))
    registers = 0
    while len(resultnew) != 0:
        registers = registers + 1
        curr_var = resultnew[0][0]
        last = 0
        flag = 0
        while flag==0:
              keyss = [item[0] for item in resultnew]
              n = len(keyss)
              for i in range(n):
                   if keyss[i] == curr_var:
                        start = resultnew[i][1][0]
                        break
              if start >= last : 
                    last = resultnew[i][1][1]
                    temp_var = curr_var
                    for i in range(n):
                        if keyss[i] == curr_var:
                              if i == (n-1):
                                  flag = 1
                              else: 
                                     curr_var = resultnew[i + 1][0]
                              break
                    for i in range(n):
                        if keyss[i] == temp_var:
                              del resultnew[i]
                              break
              else:     
                    for i in range(n):
                        if keyss[i] == curr_var:
                              if i == (n-1):
                                  flag = 1
                              else:  curr_var = resultnew[i + 1][0]
                              break
    fp.write('Registers:')
    fp.write(str(registers))
    fp.write('\n')
    fp.write('\n')
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
