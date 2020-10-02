//NAME: William Chen
//EMAIL: billy.lj.chen@gmail.com
//UID: 405131881

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <math.h>
#include <netdb.h>
#include <mraa.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

char* id;
char* hostID;
int period = 1;
int scaleFlag = 0;
int logFlag = 0;
int logFile;
char scaleOp = 'F';
int portNum;
int continueCheck = 1;
int client;
int ifError;
int sslCheck;

mraa_aio_context sensor;
SSL* ssl;

void writeFunction(char* buf);

void turnOff(){
    struct tm* time;
    struct timeval currTime;
    
    int ifError;
    ifError = gettimeofday(&currTime, NULL);
    if (ifError < 0){
        fprintf(stderr, "Error: Failure getting time: %s \n", strerror(errno));
        exit(2);
    }
    time = localtime(&currTime.tv_sec);
    if (!time){
        fprintf(stderr, "Error: Failure getting time: %s \n", strerror(errno));
        exit(2);
    }

    char buf[128];
    ifError = sprintf(buf, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
    if (ifError < 0){
        fprintf(stderr, "Error: Failure writing data to buffer: %s\n", strerror(errno));
        exit(2);
    }
    writeFunction(buf);
    exit(0);
}

void checkCommands(const char* commandGiven){
    if (logFlag == 1)
	{	
		char buf[128];
		strcpy(buf, commandGiven);
        int lastIndex = strlen(buf);
		buf[lastIndex] = '\n';
        ifError = write(logFile, buf, strlen(commandGiven) + 1);
		if (ifError < 0){
            fprintf(stderr, "Error: Failure writing buffer to log: %s\n", strerror(errno));
            exit(2);
        }	
	}
    int checkOff = strcmp(commandGiven, "OFF");
    int checkStart = strcmp(commandGiven, "START");
    int checkStop = strcmp(commandGiven, "STOP");
    int checkScaleF = strncmp(commandGiven, "SCALE=F", 7);
    int checkScaleC = strncmp(commandGiven, "SCALE=C", 7);
    int checkPeriod = strncmp(commandGiven, "PERIOD=", 7);
    int checkLog = strncmp(commandGiven, "LOG", 3);
    if (checkOff == 0){
        turnOff();
    }
    else if (checkStart == 0){
        continueCheck = 1;
    }
    else if (checkStop == 0){
        continueCheck = 0;
    }
    else if (checkScaleF == 0){
        scaleOp = 'F';
    }
    else if (checkScaleC == 0){
        scaleOp = 'C';
    }
    else if (checkPeriod == 0){
        period = atoi(commandGiven + 7);
    }
    else{
        if (checkLog != 0){
            fprintf(stderr, "Error: Invalid command. \n");
            exit(1);
        }
    }
}

double checkTemp(){
    int tempRead = mraa_aio_read(sensor);
    if (tempRead == -1){
        fprintf(stderr, "Error: Failure reading temperature: %s\n", strerror(errno));
        exit(2);
    }
    float R = (1023.0 /tempRead) - 1.0;
    R = R*100000;
    float tempC;
    tempC = 1.0/(log(R/100000)/4275+1/298.15)-273.15;
    if(scaleOp == 'F'){
        return (double)((tempC*1.8)+32);
    }
    else{
        return (double)tempC;
    }
}

void close_sensor()
{
    if (mraa_aio_close(sensor) != MRAA_SUCCESS){
        fprintf(stderr, "Error: Failure closing sensor: %s\n", strerror(errno));
        exit(2);
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client);
}

void writeFunction(char* buf){
    size_t bufSize = strlen(buf);
    sslCheck = SSL_write(ssl, buf, bufSize);
    if (sslCheck <= 0){
        fprintf(stderr, "Error: Failed with SSL error %d. Failure to write to SSL connection %s \n", SSL_get_error(ssl, sslCheck), strerror(errno));
        exit(2);
    }
    if(logFlag == 1){
        if (write(logFile, buf, bufSize) < 0){
            fprintf(stderr, "Error: Failure writing buffer to log: %s\n", strerror(errno));
            exit(2);
        }
    }
}

int main(int argc, char *argv[])
{
    struct option ops[] = {
		{"period", required_argument, NULL, 'p'},
   		{"scale", required_argument, NULL, 's'},
    	{"log", required_argument, NULL, 'l'},
        {"id", required_argument, NULL, 'i'},
        {"host", required_argument, NULL, 'h'},
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
                char* logName = optarg;
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
                        scaleFlag = 1;
                        break;
                    case 'C':
                        scaleOp = 'C';
                        scaleFlag = 1;
                        break;
                    default:
                        fprintf(stderr, "Error: Invalid scale option: %s\n", strerror(errno));
                        exit(1);
                        break;
                }
                break;
            case 'i':
                id = optarg;
                break;
            case 'h':
                hostID = optarg;
                break;
            default:
                fprintf(stderr, "Error: Invalid Argument: %s\n", strerror(errno));
                exit(1);
                break;
        }
    }
    if (!hostID){
        fprintf(stderr, "Error: Host name or address is required: %s\n", strerror(errno));
        exit(1);
    }
    struct sockaddr_in address;
    struct hostent* host = gethostbyname(hostID);
    if(!host){
        fprintf(stderr, "Error: Failure to set up host: %s\n", strerror(errno));
        exit(2);
    }
    if (!id || strlen(id) != 9){
        fprintf(stderr, "Error: ID must be included and id must be 9 digits long: %s\n", strerror(errno));
        exit(1);
    }

    int lastIndex = optind;
    if (lastIndex < argc){
        portNum = atoi(argv[lastIndex]);
        if (portNum <= 0){
            fprintf(stderr, "Error: Port Number must be greater than zero: %s\n", strerror(errno));
            exit(1);
        }
    }
    else{
        fprintf(stderr, "Error: Port number is mandatory: %s\n", strerror(errno));
        exit(1);
    }
    client = socket(2, 1, 0);
    if (client < 0){
        fprintf(stderr, "Error: Failure to set up client: %s\n", strerror(errno));
        exit(2);
    }
    bzero((char*)&address, sizeof(struct sockaddr_in));
    bcopy(host->h_addr, (char*)(&address.sin_addr.s_addr), host->h_length);
    address.sin_family = 2;
    address.sin_port = htons(portNum);
    ifError = connect(client, (struct sockaddr*)&address, sizeof(address));
    if (ifError < 0){
        fprintf(stderr, "Error: Failure connecting client and host: %s\n", strerror(errno));
        exit(2);
    }
    sensor = mraa_aio_init(1);
    if (!sensor){
        fprintf(stderr, "Error: Failure to initialize temperature sensor: %s \n", strerror(errno));
        exit(2);
    }
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    const SSL_METHOD* method = SSLv23_client_method();
    SSL_CTX* context = SSL_CTX_new(method);
    if (!context){
        fprintf(stderr, "Error: Failure to create SSL_CTX context: %s \n", strerror(errno));
        exit(2);
    }
    ssl = SSL_new(context);
    if (!ssl){
        fprintf(stderr, "Error: Failure to create SSL: %s \n", strerror(errno));
        exit(2);
    }
    if (SSL_set_fd(ssl, client) == 0){
        fprintf(stderr, "Error: Failure to set FD for SSL: %s \n", strerror(errno));
        exit(2);
    }
    if (SSL_connect(ssl) <= 0)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        fprintf(stderr, "Error: Failure to connect with server: %s \n", strerror(errno));
        exit(2);
    }
    atexit(close_sensor);

    char buf[28];
    ifError = sprintf(buf, "ID=%d\n", atoi(id));
    if (ifError < 0){
        fprintf(stderr, "Error: Failure writing to buffer: %s\n", strerror(errno));
        exit(2);
    }
    writeFunction(buf);
    
    int j = 0;
    char buffer[128];
    char inputArg[128];
    struct pollfd in[1];
    in[0].fd = client;
    in[0].events = POLLIN;
    struct timeval currTime;
    struct timeval nextTime;
    nextTime.tv_sec = 0;
    struct tm* now;

    while(1){
        ifError = gettimeofday(&currTime, NULL);
        if (ifError < 0){
            fprintf(stderr, "Error: Failure getting current time: %s\n", strerror(errno));
            exit(2);
        }
        double temp = checkTemp();
        if (nextTime.tv_sec <= currTime.tv_sec && continueCheck == 1)
        {
            now = localtime(&currTime.tv_sec);
            if (!now){
                fprintf(stderr, "Error: Failure getting time: %s\n", strerror(errno));
                exit(2);
            }
            ifError = sprintf(buffer, "%02d:%02d:%02d %.1f\n", now->tm_hour, now->tm_min, now->tm_sec, temp);
            if (ifError < 0){
                fprintf(stderr, "Error: Failure writing data to buffer: %s\n", strerror(errno));
                exit(2);
            }
            writeFunction(buffer);
            nextTime = currTime;
            nextTime.tv_sec += period;
        }
        int check = poll(in, 1 , 0);
        if(check == -1){
            fprintf(stderr, "Error: Failure polling: %s\n", strerror(errno));
            exit(2);
        }
        else if(check > 0){
            if (in[0].revents & POLLERR){
                fprintf(stderr, "Error: Failure reading stdin: %s\n", strerror(errno));
                exit(2);
            }  
            if (in[0].revents & POLLIN)
            {
                int input = SSL_read(ssl, buffer, 128);
                if (input <= 0){
                    sslCheck = input;
                    fprintf(stderr, "Error: Failed with SSL error %d. Failure to read from SSL connection: %s \n", SSL_get_error(ssl, sslCheck), strerror(errno));
                    exit(2);
                }
                int i;
                for (i = 0; i < input; i++){
                    char in = buffer[i];
                    switch (in){
                        case '\n':
                            inputArg[j] = '\0';
                            checkCommands(inputArg);
                            j = 0;
                            break;
                        default:
                            inputArg[j] = in;
                            j++;
                    }
                }
            }
        }
    }
    exit(0);
}