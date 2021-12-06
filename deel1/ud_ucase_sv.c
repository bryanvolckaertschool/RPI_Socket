#include "PJ_RPI.h"
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include "tlpi_hdr.h"
#include <pthread.h>
#include "ud_ucase.h"

int state = 0;  // 0 = waiting for start, 1 = running, 2 = done
int received_data_int; // received data from server
int IO; // IO = 1 if the client is the IO, 0 otherwise
int period; // period of the client
struct sockaddr_un svaddr, claddr;  // server address and client address
int sfd, j; // socket file descriptor and j
ssize_t numBytes; // number of bytes received
socklen_t len; // length of the client address
char buf[BUF_SIZE]; // buffer for the received data
t_data received_data; // received data from server



struct itimerval itv; // timer structure
struct sigaction sa; // signal action structure

static void Datahandler(int sig)
{
    t_data send_data; // data to be sent to server
	if (received_data_int == 1)
    {
          
        if(state == 0){
            GPIO_SET = (1 << IO) ^ GPIO_SET; // set IO to 1
            state = 1;  // change state to running
            send_data.IO = IO;  // set IO to send_data
            send_data.period = state;   // set state to send_data
            if (sendto(sfd, &send_data, sizeof(send_data), 0, (struct sockaddr *) &claddr, len) != sizeof(send_data))   // send data to server
                fatal("sendto");  // error
        }else{
            GPIO_CLR = 1 << IO; // set IO to 0
            state = 0; // change state to waiting
            send_data.IO = IO; // set IO to send_data
            send_data.period = state; // set state to send_data
            if (sendto(sfd, &send_data, sizeof(send_data), 0, (struct sockaddr *) &claddr, len) != sizeof(send_data))  // send data to server
                fatal("sendto");    // error
        }
    }
    
   		
}

static void *threadFunc(void *arg) 
{
     
    len = sizeof(struct sockaddr_un);  // Length of address data structure
    numBytes = recvfrom(sfd, &received_data, sizeof(received_data), 0, (struct sockaddr *) &claddr, &len); // Receive message from client

    if (numBytes == -1)
        errExit("recvfrom"); // Error in receiving


    received_data_int = 1; // Received data
    period = received_data.period; // Period
    IO = received_data.IO; // IO
    printf("%d", IO); // Print IO
    OUT_GPIO(received_data.IO); // Set IO

    itv.it_value.tv_sec=received_data.period; // Set timer
    itv.it_value.tv_usec =  0; // Set timer
    itv.it_interval.tv_sec = received_data.period; // Set timer
    itv.it_interval.tv_usec = 0; // Set timer

    return "";
}




int main()
{
	sigemptyset(&sa.sa_mask); // Clear signal mask
    sa.sa_flags = 0; // No flags
    sa.sa_handler = Datahandler;    // Handler function
    if (sigaction(SIGALRM, &sa, NULL) == -1) // Set signal handler
        errExit("sigaction");

	if(map_peripheral(&gpio) == -1)     // Map GPIO peripheral
	{
       	 	printf("Failed to map the physical GPIO registers into the virtual memory space.\n"); 
        	return -1;
    }

    
    /* Create server socket */
    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);   // Create socket     
    if (sfd == -1)
        errExit("socket"); // Error in creating socket

    if (strlen(SV_SOCK_PATH) > sizeof(svaddr.sun_path) - 1)
        fatal("Server socket path too long: %s", SV_SOCK_PATH); 

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
        errExit("remove-%s", SV_SOCK_PATH);
 
    memset(&svaddr, 0, sizeof(struct sockaddr_un)); // Clear address structure
    svaddr.sun_family = AF_UNIX; // Set address family
    strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1); // Set socket path

    if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind"); // Error in binding


    pthread_t t1; // Thread
    void *res; // Thread result
    int s; // Thread return value

    s = pthread_create(&t1, NULL, threadFunc, "GPIO thread\n"); // Create thread
    if (s != 0)
        errExitEN(s, "Thread creation"); // Error in thread creation

    s = pthread_join(t1, &res); // Join thread
   
    if (s != 0)
        errExitEN(s, "Thread joining"); // Error in thread joining

    if (setitimer(ITIMER_REAL, &itv, NULL) == -1)
        errExit("Setting timer"); // Error in setting timer

    while (1){

    }
    
	return 0;	

}