/* 
NAME: William Chen
EMAIL: billy.lj.chen@gmail.com
ID: 405131881
*/

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void catchFunc(int signal)
{
  if (signal == SIGSEGV)
    {
      fprintf(stderr, "SIGSEGV has been caught and recieved\n %s\n", strerror(errno));
      exit(4);
    }
}

void segFault()
{
  char* pointer = NULL;
  pointer[0] = 'w';
}

int main(int argc, char *argv[]){
    struct option ops[] = {
    { "input", required_argument, NULL, 'i'},
    { "output", required_argument, NULL, 'o'},
    { "segfault", no_argument, NULL, 's'},
    { "catch", no_argument, NULL, 'c'},
    { 0, 0, 0, 0 }
    };

    int inputFlag = 0;
    int outputFlag = 0;
    int segmentFlag = 0;
    int catchFlag = 0;
    int currentOpt;
    int optIndex;
    char* inputFile;
    char* outputFile;
    int inputFD, outputFD;
    
    while(1){
        optIndex = 0;
        currentOpt = getopt_long(argc, argv, "i:o:sc", ops, &optIndex);
        if(currentOpt == -1)
            break;
        switch(currentOpt){
            case 'i':
                inputFlag = 1;
                inputFile = optarg;
                inputFD = open(inputFile, O_RDONLY);
                if(inputFD < 0){
                    fprintf(stderr, "Error opening --input file %s\n %s\n", inputFile, strerror(errno));
                    exit(2);
                }
                break;
            case 'o':
                outputFlag = 1;
                outputFile = optarg;
                outputFD = creat(outputFile, 0666);
                if(outputFD < 0){
                    fprintf(stderr, "Error creating --output file %s\n %s\n", outputFile, strerror(errno));
                    exit(3);
                }
                break;
            case 's':
                segmentFlag = 1;
                break;
            case 'c':
                catchFlag = 1;
                break;
            default:
                fprintf(stderr, "Error: Invalid Argument. The argument must follow: [--input filename] [--output filename] [--segfault] [--catch] \n");
                exit(1);
                break;
        }
    }

    //do any file redirection
    if (inputFlag)
    {
        if(inputFD >= 0){
            close(0);
            dup(inputFD);
            close(inputFD);
        }
        
    }

    if(outputFlag)
    {
        if(outputFD >= 0){
            close(1);
            dup(outputFD);
            close(outputFD);
        }
    }

    //register the signal handler
    if(catchFlag)
    {
        signal(SIGSEGV, catchFunc);
    }
    //cause the segfault
    if(segmentFlag)
    {
        segFault();
    }
    //if no segfault was caused, copy stdin to stdout
    char* buf = (char*) malloc(sizeof(char));
    while(read(0, buf, 1) != 0)
    {
        write (1, buf, 1);
    }
    free(buf);
    exit(0);

}
