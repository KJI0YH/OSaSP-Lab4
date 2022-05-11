#define failure(str) {perror(str); exit(-1);}
#define SLEEP_TIME 100 * 1000
#define CHILD_COUNT 2
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>

pid_t childs[CHILD_COUNT];
int msgCount = 0;

//get time in millisecinds
suseconds_t getTime() {
	struct timeval tv;
	
	if (gettimeofday(&tv, NULL) == -1) {
		perror("Can not get current time\n");
		return -1;
	}	
		
	return tv.tv_sec * 1000 + tv.tv_usec / 1000; 
}

//parent get SIGUSR2
void parentCatch(int sig, siginfo_t *siginfo, void *code) {

	//signal processing
	printf("%d PID: %d PPID: %d Time: %ld PARENT get SIGUSR2 from %d\n", msgCount++, getpid(), getppid(), getTime(), siginfo->si_pid);
	
	//sleep
	usleep(SLEEP_TIME);
	
	//sent to every process in the process group
	//printf("%d PID: %d PPID: %d Time: %ld PARENT put SIGUSR1\n", msgCount++, getpid(), getppid(), getTime());
	kill(0, SIGUSR1);
}

//child get SIGUSR1
void childCatch(int sig, siginfo_t *siginfo, void *code) {

	//signal processing
	pid_t pid = getpid();
	pid_t ppid = getppid();
	
	printf("%d PID: %d PPID: %d Time: %ld CHILD%d get SIGUSR1\n", msgCount++, pid, ppid, getTime(), pid == childs[0] ? 1 : 2);
	
	//sent to parent process
	//printf("%d PID: %d PPID: %d Time: %ld CHILD%d put SIGUSR2\n", msgCount++, pid, ppid, getTime(), pid == childs[0] ? 1 : 2);
	kill(ppid, SIGUSR2);
}

void main(void){

	//masks for blocking and ignoring signals
	sigset_t maskUSR1, maskUSR2;

	if (sigemptyset(&maskUSR1) == -1) {
		failure("Can not create mask set for Child\n");
	}
	if (sigaddset(&maskUSR1, SIGUSR1) == -1) {
		failure("Can not add mask to set for Child\n");
	}
	
	if (sigemptyset(&maskUSR2) == -1) {
		failure("Can not create mask set for Parent\n");
	}
	if (sigaddset(&maskUSR2, SIGUSR2) == -1) {
		failure("Can not add mask to set for Parent\n");
	}
	
	//signal action for parent
	struct sigaction actParent;
	memset(&actParent, 0, sizeof(actParent));
	actParent.sa_sigaction = parentCatch;	
	actParent.sa_mask = maskUSR2;
	actParent.sa_flags = SA_SIGINFO;

	if (sigaction(SIGUSR2, &actParent, 0) == -1) {
		failure("Can not change the action for Parent\n");
	}
	
	//signal action for child
	struct sigaction actChild;
	memset(&actChild, 0, sizeof(actChild));
	actChild.sa_sigaction = childCatch;
	actChild.sa_mask = maskUSR1;
	actChild.sa_flags = SA_SIGINFO;
		
	//create childs
	for (int i = 0; i < CHILD_COUNT; i++) {
		childs[i] = fork();
		
		switch (childs[i]) {
		
			//child process
			case 0:
				childs[i] = getpid();
				if (sigprocmask(SIG_SETMASK, &maskUSR2, 0) == -1) {
					failure("Can not change the signal mask for Child\n");
				}
				if (sigaction(SIGUSR1, &actChild, 0) == -1) {
					failure("Can not change the action for Child\n");
				}
				printf("PID: %d PPID: %d Time: %ld CHILD%d\n", getpid(), getppid(), getTime(), i + 1);
				while (1) {}
			
			//can not create a child
			case -1:
				failure("Can not create a child process\n")	;
		}
	}
	
	//parent process
	printf("PID: %d PPID: %d Time: %ld PARENT\n", getpid(), getppid(), getTime());
	if (sigprocmask(SIG_SETMASK, &maskUSR1, 0) == -1) {
		failure("Can not change the signal mask for Parent\n");
	}
	
	sleep(1);
	
	if (kill(0, SIGUSR1) == -1) {
		perror("Can not send any signal to any process\n");
	}
	
	while (1) {}

	return;
}
