#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <sys/types.h>

// avoid infinite forks with provided function
void limit_fork(rlim_t max_procs) {
        struct rlimit rl;
        if (getrlimit(RLIMIT_NPROC, &rl))
        {
                perror("getrlimit");
                exit(-1);
        }
        rl.rlim_cur = max_procs;
        if (setrlimit(RLIMIT_NPROC, &rl))
        {
                perror("setrlimit");
                exit(-1);
        }
}

int main(int argc, char *argv[]) {
	limit_fork(50);

	// check that a file name and # processes have been entered as command line arguments
	if (argc != 3) {
		perror("Please enter both the file name and maximum number of processes to run at a time\n");
		exit(1);
	}
	
	// assign command line arguments to vars for easier access later
	char *filename = argv[1];
	char *end = NULL;
	int maxprocesses = strtol(argv[2], &end, 10);

	// open command line filename and ensure it opens successfully
	FILE *readfp = fopen(filename, "r");
	if (readfp == NULL) {
		perror("Could not open file\n");
		exit(1);
	}

	int processcounter = 0; // increment each fork, used to make sure only the desired number
				// of processes are running at a time

	char *buffer = NULL;
	ssize_t chars_read = 0;
	size_t buffer_size = 0;
	int line = 0; 
	// loop through each line of the file
	while ((chars_read = getline(&buffer, &buffer_size, readfp)) != -1) {
		line++; // increment line counter for printing in parent process
		char *output = strtok(buffer, " ");
		char *url = strtok(NULL, " \n");
		char *numseconds = strtok(NULL, " \n"); // will be NULL if only two items per line 
							// this is expected and handled later
		// if we cannot create another process yet, have the parent wait until we can
		// decrement process counter to represent that we waited
		if (processcounter >= maxprocesses) { 	// this only applies to parent because of exit()
							// in child process code if execvp() fails
						// execvp never returns if succeeds, so will not make it to here either
			wait(NULL);
			processcounter--;
		} else {
			pid_t pid = fork();
			processcounter++; 	// create a child process and increment process counter
			if (pid < 0) { 		// check that fork() worked
				perror("fork error\n");
				fclose(readfp);	// close file pointers and free from heap
				free(buffer);
				exit(1);	
			} else if (pid == 0) { // child process
				// depending on number of items in each line, call curl correctly using execvp()
				if (numseconds != NULL) {
					char *args[] = {"curl", "-m", numseconds, "-o", output, "-s", url, NULL};
					execvp("curl", args); 
				} else {
					char *args[] = {"curl", "-o", output, "-s", url, NULL};
					execvp("curl", args);
				}
				perror("execvp failed\n");
				fclose(readfp);
				free(buffer);
				exit(1);    // if execvp() fails, close file pointer and free from heap before exiting
			} else {
				//  print when a download request is starting
				printf("process %d processing line %d\n", pid, line);
				// if too many, wait until another process can start
				while (processcounter >= maxprocesses) {
                                	wait(NULL);
					processcounter--;
                        	}
			}
		}

	}

	fclose(readfp);
	free(buffer);	
}
