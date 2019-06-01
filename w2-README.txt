Documentation for Warmup Assignment 2
=====================================

+------------------------+
| BUILD & RUN (Required) |
+------------------------+

Replace "(Comments?)" below with the command the grader should use to compile
your program (it should simply be "make" or "make warmup2"; minor variation is
also fine).

    To compile your code, the grader should type: (Comments?)

If you have additional instruction for the grader, replace "(Comments?)" with your
instruction (or with the word "none" if you don't have additional instructions):

    Additional instructions for building/running this assignment: (Comments?)

+-------------------------+
| SELF-GRADING (Required) |
+-------------------------+

Replace each "?" below with a numeric value:

(A) Basic running of the code : ? out of 90 pts

(B) Basic handling of <Cntrl+C> : ? out of 10 pts

Missing required section(s) in README file : -? pts
Submitted binary file : -? pts
Cannot compile : -? pts
Compiler warnings : -? pts
"make clean" : -? pts
Segmentation faults : -? pts
Program never terminates : -? pts
Separate compilation : -? pts

Delay trace printing : -? pts
Using busy-wait : -? pts
Handling of commandline arguments:
    1) -n : -? pts
    2) -lambda : -? pts
    3) -mu : -? pts
    4) -r : -? pts
    6) -P and -B : -? pts
Trace output :
    1) regular packets: -? pts
    2) dropped packets: -? pts
    3) removed packets: -? pts
    4) token arrival (dropped or not dropped): -? pts
    5) monotonically non-decreasing timestamps: -? pts
Statistics output :
    1) inter-arrival time : -? pts
    2) service time : -? pts
    3) number of customers in Q1 : -? pts
    4) number of customers in Q2 : -? pts
    5) number of customers at a server : -? pts
    6) time in system : -? pts
    7) standard deviation for time in system : -? pts
    8) drop probability : -? pts
Output bad format : -? pts
Output wrong precision for statistics (should be 6-8 significant digits) : -? pts
Tiny inter-arrival time test : -? pts
Tiny service time test : -? pts
Large total number of customers test : -? pts
Large total number of customers with high arrival rate test : -? pts
Dropped tokens test : -? pts
Cannot handle <Cntrl+C> at all (ignored or no statistics) : -? pts
<Cntrl+C> is handled but statistics are off : -? pts
Cannot stop packet arrival thread when required : -? pts
Cannot stop token depositing thread when required : -? pts
Cannot stop server thread when required : -? pts
Not using condition variables and do some kind of busy-wait : -? pts
Synchronization check : -? pts
Deadlocks : -? pts
Only one server : -? pts
Bad commandline or command : -? pts

+---------------------------------+
| BUGS / TESTS TO SKIP (Required) |
+---------------------------------+

Are there are any tests mentioned in the grading guidelines test suite that you
know that it's not working and you don't want the grader to run it at all so you
won't get extra deductions, please replace "(Comments?)" below with your list.
(Of course, if the grader won't run such tests in the plus points section, you
will not get plus points for them; if the garder won't run such tests in the
minus points section, you will lose all the points there.)  If there's nothing
the grader should skip, please replace "(Comments?)" with "none".

Please skip the following tests: (Comments?)

+---------------------------------------------------------------------------------------+
| ADDITIONAL INFORMATION FOR GRADER (Optional, but the grader should what you add here) |
+---------------------------------------------------------------------------------------+

+-----------------------------------------------+
| OTHER (Optional) - Not considered for grading |
+-----------------------------------------------+

Comments on design decisions: (Comments?)
