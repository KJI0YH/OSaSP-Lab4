#define failure(str) {perror(str); exit(-1);}
#define CHILDS_COUNT 8
#define SIG_COUNT 101
#define FILE_NAME "tmpPID"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

//child item
typedef struct _child_t {
	int _parent;
	int *_childs;
	int _childs_count;
	struct sigaction _act;
	int _recvSig;
	sigset_t _ignoreSig;
} child_t;

//all childs
child_t childs[CHILDS_COUNT + 1];

//counters for USR signals
int usr1Count = 0, usr2Count = 0;

//signal action for SIGTERM 
struct sigaction actTerm;

//initialization of a file with PIDs
void initFile(void) {
	FILE* fp = fopen(FILE_NAME, "w+b");
	if (fp == NULL)
		failure("Can not open temp file");
	int initial = 0;
	for (int i = 0; i <= CHILDS_COUNT; i++)
		fwrite(&initial, sizeof(int), 1, fp);
	
	if (fclose(fp) == EOF)
		failure("Can not close temp file");
	return;
}

//read all PIDs from a file
int* readAllPID(void) {
	FILE* fp = fopen(FILE_NAME, "rb");
	if (fp == NULL)
		failure("Can not open temp file\n");
	int* pids = (int*)malloc((CHILDS_COUNT + 1) * sizeof(int));
	if (pids == NULL)
		failure("Can not allocate memory");
	
	fread(pids, sizeof(int), CHILDS_COUNT + 1, fp);
	
	if (fclose(fp) == EOF)
		failure("Can not close temp file");
	
	return pids;
}

//write new PID to a file
void writePID(int ordChild, pid_t newPID) {
	
	int *pids = readAllPID();
	pids[ordChild] = newPID;
	
	FILE* fp = fopen(FILE_NAME, "w+b");
	if (fp == NULL)
		failure("Can not open temp file");
	fwrite(pids, sizeof(int), CHILDS_COUNT + 1, fp);
	if (fclose(fp) == EOF)
		failure("Can not close temp file");
	free(pids);
	return;
}

//read child PID from a file
pid_t readPID(int ordChild) {
	int *pids = readAllPID();
	pid_t pid = pids[ordChild];
	free(pids);
	return pid;
}

//get time in microseconds
long long getTime() {
	struct timeval tv;
	
	if (gettimeofday(&tv, NULL) == -1) {
		perror("Can not get current time");
		return -1;
	}	
		
	return tv.tv_sec * 1000000 + tv.tv_usec; 
}

//terminate work of the child process 
void terminateCatch(int sig, siginfo_t *siginfo, void *code) {
	int* pids = readAllPID();
	
	pid_t pid = getpid();
	
	int i = 0;
	while (i <= CHILDS_COUNT && pid != pids[i])
		i++;
		
	free(pids);
	
	printf("%d PID: %d PPID: %d finish the work after %ds SIGUSR1 and %ds SIGUSR2\n", i, pid, getppid(), usr1Count, usr2Count);
	
	for (int j = 0; j < childs[i]._childs_count; j++) {
		if (waitpid(readPID(childs[i]._childs[j]), NULL, 0) == -1) 
			perror("Can not wait PID");
	}
	
	exit(0);	
}

//count signals
void countSIG(int sig, int ordChild) {
	printf("%d PID: %d PPID: %d receive SIGUSR%d Time: %lld\n", ordChild, getpid(), getppid(), sig == SIGUSR1 ? 1 : 2, getTime());
	
	if (sig == SIGUSR1)
		usr1Count++;
	else if (sig == SIGUSR2)
		usr2Count++;
}

//child 1 process signal
void child1Catch(int sig, siginfo_t *siginfo, void *code) {

	countSIG(sig, 1);
	
	//wait all childs 2-8
	if (usr1Count + usr2Count == SIG_COUNT) {
		
		kill(0, SIGTERM);
		
		if (waitpid(readPID(2), NULL, 0) == -1)
			perror("Can not wait PID");

		printf("1 PID: %d PPID: %d finish the work after %ds SIGUSR1 and %ds SIGUSR2\n", getpid(), getppid(), usr1Count, usr2Count);		
		exit(0);
	}
	
	kill(0, SIGUSR2);	
	printf("%d PID: %d PPID: %d sent SIGUSR2 Time: %lld\n", 1, getpid(), getppid(), getTime());
}

//child 2 process signal
void child2Catch(int sig, siginfo_t *siginfo, void *code) {
	countSIG(sig, 2);
	
	if (kill(readPID(6), SIGUSR1) == -1)
		failure("Can not sent a signal\n");
	printf("%d PID: %d PPID: %d sent SIGUSR1 Time: %lld\n", 2, getpid(), getppid(), getTime());
}

//child 3 process signal
void child3Catch(int sig, siginfo_t *siginfo, void *code) {
	countSIG(sig, 3);
	
	if (kill(readPID(7), SIGUSR1) == -1)
		failure("Can not sent a signal\n");
	printf("%d PID: %d PPID: %d sent SIGUSR1 Time: %lld\n", 3, getpid(), getppid(), getTime());
}

//child 4 process signal
void child4Catch(int sig, siginfo_t *siginfo, void *code) {
	countSIG(sig, 4);
	
	if (kill(readPID(8), SIGUSR1) == -1)	
		failure("Can not sent a signal\n");
	printf("%d PID: %d PPID: %d sent SIGUSR1 Time: %lld\n", 4, getpid(), getppid(), getTime());
}

//child 5 process signal
void child5Catch(int sig, siginfo_t *siginfo, void *code) {
	countSIG(sig, 5);	
}

//child 6 process signal
void child6Catch(int sig, siginfo_t *siginfo, void *code) {
	countSIG(sig, 6);
}

//child 7 process signal
void child7Catch(int sig, siginfo_t *siginfo, void *code) {
	countSIG(sig, 7);	
}

//child 8 process signal
void child8Catch(int sig, siginfo_t *siginfo, void *code) {
	countSIG(sig, 8);

	if (kill(readPID(1), SIGUSR1) == -1)
		failure("Can not sent a signal\n");
	printf("%d PID: %d PPID: %d sent SIGUSR1 Time: %lld\n", 8, getpid(), getppid(), getTime());
}

//initialize child items
void assignChilds(child_t *node) {
	for (int i = 0; i <= CHILDS_COUNT; i++) {
		memset(&node[i], 0, sizeof(node[i]));
		node[i]._parent = i;
		node[i]._act.sa_flags = SA_SIGINFO;
	}
	
	sigset_t maskUSR1, maskUSR2, maskUSRAll;

	//create ingnore masks
	if (sigemptyset(&maskUSR1) == -1) 
		failure("Can not create maskUSR1 ignore mask\n");
	if (sigaddset(&maskUSR1, SIGUSR1) == -1) 
		failure("Can not add SIGUSR1 to ignore mask\n");
	
	if (sigemptyset(&maskUSR2) == -1) 
		failure("Can not create maskUSR2 ignore mask\n");
	if (sigaddset(&maskUSR2, SIGUSR2) == -1) 
		failure("Can not add SIGUSR2 to ignore all mask\n");
	
	if (sigemptyset(&maskUSRAll) == -1) 
		failure("Can not create ignore mask\n");
	if (sigaddset(&maskUSRAll, SIGUSR1) == -1) 
		failure("Can not add SIGUSR1 to ignore all mask\n");
	if (sigaddset(&maskUSRAll, SIGUSR2) == -1) 
		failure("Can not add SIGUSR1 to ignore all mask\n");
	if (sigaddset(&maskUSRAll, SIGTERM) == -1) 
		failure("Can not add SIGTERM to ignore all mask\n");
	
	
	node[0]._childs_count = 1;
	node[0]._childs = (int*)malloc(sizeof(int) * node[0]._childs_count);
	if (node[0]._childs == NULL)
		failure("Can not allocate a memory");
	node[0]._childs[0] = 1;
	node[0]._ignoreSig = maskUSRAll;
	
	node[1]._childs_count = 1;
	node[1]._childs = (int*)malloc(sizeof(int) * node[1]._childs_count);
	if (node[1]._childs == NULL)
		failure("Can not allocate a memory");
	node[1]._childs[0] = 2;
	node[1]._recvSig = SIGUSR1;
	node[1]._act.sa_mask = maskUSR1;
	node[1]._ignoreSig = maskUSR2;
	if (sigaddset(&node[1]._ignoreSig, SIGTERM) == -1)
		failure("Can not add SIGTERM to ignore child1 mask\n")	
	node[1]._act.sa_sigaction = child1Catch;
	
	node[2]._childs_count = 2;
	node[2]._childs = (int*)malloc(sizeof(int) * node[2]._childs_count);
	if (node[2]._childs == NULL)
		failure("Can not allocate a memory");
	node[2]._childs[0] = 3;
	node[2]._childs[1] = 4;
	node[2]._recvSig = SIGUSR2;
	node[2]._act.sa_mask = maskUSR2;
	node[2]._ignoreSig = maskUSR1;
	node[2]._act.sa_sigaction = child2Catch;
	
	node[3]._childs_count = 1;
	node[3]._childs = (int*)malloc(sizeof(int) * node[3]._childs_count);
	if (node[3]._childs == NULL)
		failure("Can not allocate a memory");
	node[3]._childs[0] = 6;
	node[3]._recvSig = SIGUSR2;
	node[3]._act.sa_mask = maskUSR2;
	node[3]._ignoreSig = maskUSR1;
	node[3]._act.sa_sigaction = child3Catch;
	
	node[4]._childs_count = 1;
	node[4]._childs = (int*)malloc(sizeof(int) * node[4]._childs_count);
	if (node[4]._childs == NULL)
		failure("Can not allocate a memory");
	node[4]._childs[0] = 5;
	node[4]._recvSig = SIGUSR2;
	node[4]._act.sa_mask = maskUSR2;
	node[4]._ignoreSig = maskUSR1;
	node[4]._act.sa_sigaction = child4Catch;
	
	node[5]._childs_count = 0;	
	node[5]._recvSig = SIGUSR2;
	node[5]._act.sa_mask = maskUSR2;
	node[5]._ignoreSig = maskUSR1;
	node[5]._act.sa_sigaction = child5Catch;
	
	node[6]._childs_count = 1;
	node[6]._childs = (int*)malloc(sizeof(int) * node[6]._childs_count);
	if (node[6]._childs == NULL)
		failure("Can not allocate a memory");
	node[6]._childs[0] = 7;
	node[6]._recvSig = SIGUSR1;
	node[6]._act.sa_mask = maskUSR1;
	node[6]._ignoreSig = maskUSR2;
	node[6]._act.sa_sigaction = child6Catch;
	
	node[7]._childs_count = 1;
	node[7]._childs = (int*)malloc(sizeof(int) * node[7]._childs_count);
	if (node[7]._childs == NULL)
		failure("Can not allocate a memory");
	node[7]._childs[0] = 8;
	node[7]._recvSig = SIGUSR1;
	node[7]._act.sa_mask = maskUSR1;
	node[7]._ignoreSig = maskUSR2;
	node[7]._act.sa_sigaction = child7Catch;
	
	node[8]._childs_count = 0;
	node[8]._recvSig = SIGUSR1;
	node[8]._act.sa_mask = maskUSR1;
	node[8]._ignoreSig = maskUSR2;
	node[8]._act.sa_sigaction = child8Catch;
}

//create process tree
void createProcTree(child_t node) {
	
	printf("PID: %d PPID: %d Time: %lld CHILD%d\n", getpid(), getppid(), getTime(), node._parent);
	if (sigprocmask(SIG_SETMASK, &node._ignoreSig, 0) == -1) 
		failure("Can not change the signal mask\n");

	//add action for user signal
	if (node._parent != 0) 
		if (sigaction(node._recvSig, &node._act, 0) == -1)
			failure("Can not change the action for child\n");
		
	//add action for terminate signal
	if (node._parent > 1) 
		if (sigaction(SIGTERM, &actTerm, 0) == -1) 
			failure("Ca not change the action for child\n");
				
	//write created PID to a file
	writePID(node._parent, getpid());
	
	//create all childs for this parent
	for (int i = 0; i < node._childs_count; i++) {
		
		//wait while previous pid created
		while (readPID(node._childs[i] - 1) == 0) {}
		
		pid_t child = fork();
		switch(child) {

			//child process
			case 0:				
				createProcTree(childs[node._childs[i]]);
				while (1) {}
				break;
				
			//child create failure
			case -1:
				failure("Can not create a child\n");
				break;
				
			//parent 
			default:
			
				//child 1 process
				if (node._parent == 1 && i + 1 == node._childs_count) {
					
					//wait all pids
					for (int i = 0; i <= CHILDS_COUNT; i++) {
						while (readPID(i) == 0) {}
					}					
					
					//start signal exchange
					if (kill(0, SIGUSR2) == -1)
						failure("Can not send a signal\n");
					while (1) {}
				}				
				break;
		}
	}
	return;
}

void main(void) {
	
	//initialize file with PIDs
	initFile();
	
	//initialize child items
	assignChilds(childs);	

	//set ingnoring signals
	sigprocmask(SIG_SETMASK, &childs[0]._ignoreSig, 0);
	
	//create terminate action
	memset(&actTerm, 0, sizeof(actTerm));
	actTerm.sa_sigaction = terminateCatch;
	
	sigset_t termMask;
	sigemptyset(&termMask);
	sigaddset(&termMask, SIGTERM);
	
	actTerm.sa_mask = termMask;
	actTerm.sa_flags = SA_SIGINFO;

	//create process tree
	createProcTree(childs[0]);
	
	//wait for first child
	while (readPID(1) == 0) {}
	if (waitpid(readPID(1), NULL, 0) == -1)
		failure("Can not wait PID\n");
	
	printf("0 PID: %d PPID: %d finish the work after %ds SIGUSR1 and %ds SIGUSR2\n", getpid(), getppid(), usr1Count, usr2Count);
	
	return;	
}
