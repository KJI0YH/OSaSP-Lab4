# OSaSP-Lab4
Using signals in OS UNIX

task1.c:
  Parent simultaneously sends a SIGUSR1 to the two childs. The childs received the signal and send a SIGUSR2 to the parent. Parent received the signal and after 100 ms send SIGUSR1...
  
task2.c:
  Processes continuously exchange signals. Each process, when receiving or sending a signal, outputs information to the console in the following form: N PID PPID send/receive USR1/2 TIME (mcs). Process 1, after receiving the 101st USR signal, sends a SIGTERM and waits for all childs to complete, after which it completes itself. Process 0 waits for the completion of process 1, after which it terminates itself. Childs, will get SIGTERM quits with the output of a message like PID PPID finish after Xst SIGUSR1 and Yst SIGUSR2 to the console
  
  Process tree:
    1->2 2->(3,4) 4->5 3->6 6->7 7->8
  Signal exchange:
    1->(2,3,4,5) SIGUSR2 2->6 SIGUSR1 3->7 SIGUSR1 4->8 SIGUSR1 8->1 SIGUSR1
