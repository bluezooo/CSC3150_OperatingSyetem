#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	/* fork a child process */
	pid_t pid;
	int status;
	printf("Process start to fork\n");
	pid = fork();

	if (pid < 0) {
		perror("fork");
		exit(1);
	} else {
		//child process
		if (pid == 0) {
			int i;
			char *arg[argc];
			for (i = 0; i < argc - 1; i++) {
				arg[i] = argv[i + 1];
			}
			arg[argc - 1] = NULL;
			printf("I'm the Child Process, my pid = %d\n",
			       getpid());

			/* execute test program */
			printf("Chlid process start to execute test program:\n");
			execve(arg[0], arg, NULL);
			printf("Continue to run original child process!\n");
			perror("execve");
			exit(EXIT_FAILURE);
		}

		//Parent process
		else {
			printf("I'm the Parent Process, my pid = %d\n",
			       getpid());

			/* wait for child process terminates */
			pid_t wait_child = waitpid(pid, &status, WUNTRACED);
			if (wait_child == -1) {
				perror("Wait chlid");
				exit(EXIT_FAILURE);
			}
			printf("Parent process receives SIGCHLD signal\n");

			/* check child process'  termination status */
			if (WIFEXITED(status)) {
				printf("Normal termination with EXIT STATUS = %d\n",
				       WEXITSTATUS(status));
			} else if (WIFSIGNALED(status)) {
				switch (WTERMSIG(status)) {
				case 1:
					printf("child process get SIGHUP signal\n");
					break;
				case 2:
					printf("child process get SIGINT signal\n");
					break;
				case 3:
					printf("child process get SIGQUIT signal\n");
					break;
				case 4:
					printf("child process get SIGILL signal\n");
					break;
				case 5:
					printf("child process get SIGTRAP signal\n");
					break;
				case 6:
					printf("child process get SIGABRT signal\n");
					break;
				case 7:
					printf("child process get SIGBUS signal\n");
					break;
				case 8:
					printf("child process get SIGFPE signal\n");
					break;
				case 9:
					printf("child process get SIGKILL signal\n");
					break;
				case 11:
					printf("child process get SIGSEGV signal\n");
					break;
				case 13:
					printf("child process get SIGPIPE signal\n");
					break;
				case 14:
					printf("child process get SIGALRM signal\n");
					break;
				case 15:
					printf("child process get SIGTERM signal\n");
					break;
				case 19:
					printf("child process get SIGSTOP signal\n");
					break;
				default:
					printf("CHILD EXECUTION FAILED:\n");
					break;
				}
			}
			exit(0);
		}
	}
	return 0;
}
