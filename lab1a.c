//NAME: William Chen
//EMAIL: billy.lj.chen@gmail.com
//ID: 405131881

#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>

struct termios restoreAttr;
int pipeToShell[2];
int pipeFromShell[2];
int terminate;
int shellFile;
pid_t pid;
void createPipe(int pipe1[2]);
void setTerminal();
void restoreTerminal();
void writeCheck();
void noShell();
void processsInput();

void createPipe(int pipe1[2]){
    if ((pipe(pipe1)) < 0) 
    {
		fprintf(stderr, "Failure to create pipe.\n");
		exit(1);
	}
}

void setTerminal(){
    struct termios attr;
    if((tcgetattr(STDIN_FILENO, &attr)) < 0){
        fprintf(stderr, "Failure to obtain the initial attributes: %s\n", strerror(errno));
		exit(1);
    }
    restoreAttr = attr;;
    attr.c_iflag = ISTRIP;
    attr.c_oflag = 0;
    attr.c_lflag = 0;
    if ((tcsetattr(0, TCSANOW, &attr)) < 0){
		fprintf(stderr, "Failure to set terminal to character-at-a-time, no-echo mode: %s\n", strerror(errno));
		exit(1);
	}
    atexit(restoreTerminal);
}

void restoreTerminal(){
    if(tcsetattr(STDIN_FILENO, TCSANOW, &restoreAttr) < 0)
	{
		fprintf(stderr, "Failure to restore original terminal: %s\n", strerror(errno));
		exit(1);
	}
}

void signalHandler(int signal){
    if(signal == SIGPIPE){
        terminate = 1;
        fprintf(stderr, "The signal handler is called but failed, exiting with signal %d\n", signal);
    }
}

void noShell(){
    char buf[8];
    int charRead;
    while(1){
        charRead = read(STDIN_FILENO, &buf, 8);
        if(charRead < 0){
            fprintf(stderr, "Failure to read characters from terminal: %s\n", strerror(errno));
			exit(1);
        }
        int i;
        char c;
        for(i = 0; i < charRead; i++){
			if (buf[i] == '\r' || buf[i] == '\n') 
			{
                char combine1[2];
				combine1[0] = '\r';
				combine1[1] = '\n';
				write(STDOUT_FILENO, combine1, 2);
			}
			else if (buf[i] == 0x04){
                char combine2[2];
                combine2[0] = '^';
				combine2[1] = 'D';
				write(STDOUT_FILENO, combine2, 2);
				exit(0);
            }
            else
			{
                c = buf[i];
				write(STDOUT_FILENO, &c, 1);
			}   
        }
    }
}

int main (int argc, char* argv[]){
    static const struct option op[] = {
        {"shell", required_argument, NULL, 's'},
        {0, 0, 0, 0}
    };
    int shellFlag = 0;
    int choice;
    char* program; 
    while(1){
        choice = getopt_long(argc, argv, "", op, NULL);
        if(choice == -1)
        break;
        switch(choice){
            case 's':
                shellFlag = 1;
                program = optarg;
                break;
            default:
                fprintf(stderr, "Error: Invalid Argument. The argument must be [--shell=program] \n");
                exit(1);
        }
    }
    if (shellFlag != 1){
        //no shell :p
        setTerminal();
        noShell();
    }
    else{
        setTerminal();
        createPipe(pipeToShell);
        createPipe(pipeFromShell);
        signal(SIGPIPE, signalHandler);
        pid = fork();
        if(pid == 0){ //child
            close(pipeToShell[1]);
		    close(pipeFromShell[0]);
		    dup2(pipeToShell[0], STDIN_FILENO); 
		    close(pipeToShell[0]); 
		    dup2(pipeFromShell[1], STDOUT_FILENO);
		    dup2(pipeFromShell[1], STDERR_FILENO);
		    close(pipeFromShell[1]);
		    execl(program, program, (char*) NULL); 
		    fprintf(stderr, "Failure to execute shell in child process: %s\n", strerror(errno));
		    exit(1);
        }
        else if(pid > 0){ //parent
            close(pipeToShell[0]);
		    close(pipeFromShell[1]);
            terminate = 0;
            struct pollfd input[] =  // define the two input channels we are multiplexing
		    {
			    {STDIN_FILENO, POLLIN, 0},
			    {pipeFromShell[0], POLLIN, 0}
		    };
            int r;
            while(1){
                if((r = poll(input, 2, 0)) < 0){
                    fprintf(stderr, "Failure to call poll()\n");
                    exit(1);
                }
                else if(r > 0){
                    if(input[0].revents & POLLIN){
                        char buf[4];
                        int charRead;
                        charRead = read(STDIN_FILENO, &buf, 4);
                        if(charRead < 0){
                            fprintf(stderr, "Failure to read characters from terminal: %s\n", strerror(errno));
		                    exit(1);
                        }
                        int i;
                        for(i = 0; i < charRead; i++){
                            if (buf[i] == 0x03){
                                char combin[2];
                                combin[0] = '^';
                                combin[1] = 'C';
				                write(STDOUT_FILENO, combin, 2);
                                if ((kill(pid, SIGINT)) < 0)
			                    {
				                    fprintf(stderr, "Error sending SIGINT to shell: %s\n", strerror(errno));
				                    exit(1);
			                    }
			                }
			                else if (buf[i] == '\r' || buf[i] == '\n') {
                                char combine1[2];
                                char lf = '\n';
				                combine1[0] = '\r';
				                combine1[1] = '\n';
				                write(STDOUT_FILENO, combine1, 2);
                                write(pipeToShell[1], &lf, 1);
			                }
			                else if (buf[i] == 0x04){
                                char combine2[2];
                                combine2[0] = '^';
				                combine2[1] = 'D';
                                terminate = 1;
				                write(STDOUT_FILENO, combine2, 2);
                                close(pipeToShell[1]);
                            }
                            else{
                                char x = buf[i];
                                write(STDOUT_FILENO, &x, 1);
                                write(pipeToShell[1], &x, 1);
                            }
                        }
                    }  
                    if(input[1].revents & POLLIN){
                        char buf1[256];
                        int charRead;
                        charRead = read(pipeFromShell[0], &buf1, 256);
                        if(charRead < 0){
                            fprintf(stderr, "Failure to read characters from shell: %s\n", strerror(errno));
		                    exit(1);
                        }
                        int i;
                        for(i = 0; i < charRead; i++){
			                if (buf1[i] == '\r' || buf1[i] == '\n') {
                                char combine1[2];
				                combine1[0] = '\r';
				                combine1[1] = '\n';
				                write(STDOUT_FILENO, combine1, 2);
			                }
			                else if (buf1[i] == '\004'){
                                char combine2[2];
                                combine2[0] = '^';
				                combine2[1] = 'D';
                                terminate = 1;
				                write(STDOUT_FILENO, combine2, 2);
                            }
                            else{
                                char a = buf1[i];
                                write(STDOUT_FILENO, &a, 1);
                            }
                        }
                    }
                    if(input[0].revents & POLLERR || input[0].revents & POLLHUP){
                        terminate = 1;
                    }
                    if(input[0].revents & POLLERR || input[0].revents & POLLHUP){
                        terminate = 1;
                    }
                }
                if(terminate){
                    break;
                }
            }
            close(pipeFromShell[1]);
            close(pipeToShell[0]);
            int stat;
            if (waitpid(pid, &stat, 0) < 0)
		    {
			    fprintf(stderr, "Failure while waiting for child process: %s\n", strerror(errno));
			    exit(1);
		    }
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", stat&0x007f, stat>>8);
		    exit(0);
        }
        else{
            //neither parent process nor child process
            fprintf(stderr, "Failure to fork a new process: %s\n", strerror(errno));
		    exit (1);
        }
    }
}