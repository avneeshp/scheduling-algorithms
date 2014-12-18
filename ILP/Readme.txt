---------------------------------------------------------------------------------------------------------
To run the TIME-constrained ILP scheduling,
  python schedule.py dfgs/benchmark
Eg:
  python schedule.py dfgs/pr.ll

Different time constraints can be set by changing the variable "FACTOR" in schedule.py.
Time constraint is then set to FACTOR * ASAP_LATENCY

The timeout for LP solver can also be changed by changing the variable "TIMEOUT" in schedule.py.
This timeout value is in seconds.

The scheduled instructions are printed in the file "schedule.txt". 
Number of resources required, cycles taken, and time to execute are printed on the prompt.


---------------------------------------------------------------------------------------------------------
To run the Resource-constrained ILP scheduling,
  python schedulercs.py dfgs/benchmark
Eg:
  python schedulercs.py dfgs/pr.ll

Different resource constraints can be set by modifying "MUL_C" and "ADD_C" in schedulercs.py

The time constraint to verify can be set by modifying "TCS_TO_TEST" in schedulercs.py

The lpsolver returns the scheduled result in schedule.txt if the solution is feasible or returns
infeasible if the instructions cannot be scheduled in TCS_TO_TEST cycles given the resource constraint
