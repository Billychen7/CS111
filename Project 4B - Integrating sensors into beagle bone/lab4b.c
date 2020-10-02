//NAME: William Chen
//EMAIL: billy.lj.chen@gmail.com
//ID: 405131881


#include <stdlib.h>
#include <mraa.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <sys/errno.h>
#include <mraa/aio.h>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <poll.h>

int period = 1;
char scaleOp = 'F';
int logFlag = 0;
int scaleFlag = 0;
char* logName; 
int logFile;
int stopCommand = 0;
const int B = 4275;      
const int R0 = 100000;      
mraa_aio_context tempSensor;
mraa_gpio_context button;

void initializeSensors();
void buttonHandler();
void printTemp(float temp);
void inputCommand();
void closeSensors();

void initializeSensors(){
    button = mraa_gpio_init(60);
    if(button == NULL){
        fprintf(stderr, "Error: Initializing button: %s\n", strerror(errno));
        exit(1);
    }
    tempSensor = mraa_aio_init(1);
    if(tempSensor == NULL){
        fprintf(stderr, "Error: Initializing tempSensor: %s\n", strerror(errno));
        exit(1);
    }
    if(mraa_gpio_dir(button, MRAA_GPIO_IN) != MRAA_SUCCESS){
        fprintf(stderr, "Error: Setting up input direction for button: %s\n", strerror(errno));
        exit(1);
    }
	if(mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &buttonHandler, NULL) != MRAA_SUCCESS){
        fprintf(stderr, "Error: Failure to connect interrupt to handler: %s\n", strerror(errno));
        exit(1);
    }
}

void buttonHandler(){
    time_t currTime;
    time(&currTime);
    struct tm * timeVal;
    timeVal = localtime(&currTime);

    if(logFlag == 1){
        dprintf(logFile, "%02d:%02d:%02d SHUTDOWN\n", timeVal->tm_hour, timeVal->tm_min, timeVal->tm_sec);
    }
    fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", timeVal->tm_hour, timeVal->tm_min, timeVal->tm_sec);
    exit(0);
}

void printTemp(float temp){
    time_t currTime;
    time(&currTime);
    struct tm * timeVal;
    timeVal = localtime(&currTime);

    if(logFlag == 1){
        dprintf(logFile, "%02d:%02d:%02d %.1f\n", timeVal->tm_hour, timeVal->tm_min, timeVal->tm_sec, temp);
    }
    fprintf(stdout, "%02d:%02d:%02d %.1f\n", timeVal->tm_hour, timeVal->tm_min, timeVal->tm_sec, temp);
}

void inputCommand(char* input){
    if(strcmp(input, "SCALE=F\n") == 0){
        scaleOp = 'F';
        if(logFlag){
            dprintf(logFile, "%s", input);
        }
    }
    else if(strcmp(input, "SCALE=C\n") == 0){
        scaleOp = 'C';
        if(logFlag){
            dprintf(logFile, "%s", input);
        }
    }
    else if(strstr(input, "PERIOD=") != NULL){
        period = atoi(input + 7);
        if(logFlag){
            dprintf(logFile, "%s", input);
        }
    }
    else if(strcmp(input, "STOP\n") == 0){
        stopCommand = 1;
        if(logFlag){
            dprintf(logFile, "%s", input);
        }
    }
    else if(strcmp(input, "START\n") == 0){
        stopCommand = 0;
        if(logFlag){
            dprintf(logFile, "%s", input);
        }
    }
    else if(strstr(input, "LOG") != NULL){
        if(logFlag){
            dprintf(logFile, "%s", input);
        }
    }
    else if(strcmp(input, "OFF\n") == 0){
        if(logFlag){
            dprintf(logFile, "%s", input);
        }
        buttonHandler();
    }
    else{
        if(logFlag){
            dprintf(logFile, "%s", input);
        }
        fprintf(stderr, "invalid command\n");
    }
}

void closeSensors(){
    mraa_aio_close(tempSensor);
    mraa_gpio_close(button);
}

int main(int argc, char* argv[]) {
	struct option ops[] = {
		{"period", required_argument, NULL, 'p'},
   		{"scale", required_argument, NULL, 's'},
    	{"log", required_argument, NULL, 'l'},
    	{0, 0, 0, 0}
	};
    int c;
    while(1){
        c = getopt_long(argc, argv, "", ops, NULL);
        if(c == -1)
        break;
        switch(c){
            case 'p':
                period = atoi(optarg);
                break;
            case 'l':
                logFlag = 1;
                logName = optarg;
                logFile = creat(logName, 0666);
                if(logFile < 0){
                    fprintf(stderr, "Error: Creating Log File: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            case 's':
                switch(optarg[0]){
                    case 'F':
                        scaleOp = 'F';
                        break;
                    case 'f':
                        scaleOp = 'F';
                        break;
                    case 'C':
                        scaleOp = 'C';
                        break;
                    case 'c':
                        scaleOp = 'C';
                        break;
                    default:
                        fprintf(stderr, "Error: Invalid scale option: %s\n", strerror(errno));
                        exit(1);
                        break;
                }
                break;
            default:
                fprintf(stderr, "Error: Invalid Argument: %s\n", strerror(errno));
                exit(1);
                break;
        }
    }
    initializeSensors();
    struct pollfd polls;
    polls.fd = STDIN_FILENO;
    polls.events = POLLIN | POLLERR | POLLHUP;
    time_t start;
    time(&start);
    time_t end;
    time(&end);

    char* input = malloc(256 * sizeof(char));
    while(1){
        if(stopCommand == 0){
            if(difftime(end, start) >= period){
                time(&start);
                int tempRead = mraa_aio_read(tempSensor);
                float R = (1023.0/tempRead) - 1.0;
                R = R0*R;
                float temp;
                temp = 1.0/(log(R/R0)/B+1/298.15)-273.15;  //returns Celsius
                if(scaleOp == 'F')
                    temp = (temp * 1.8) + 32;
                printTemp(temp);
            }
        }
        int in = poll(&polls, 1, 0);
        if(in == 1){
            fgets(input, 256, stdin);
            inputCommand(input);
        }
        time(&end);
    }
    closeSensors();
    exit(0);
}
