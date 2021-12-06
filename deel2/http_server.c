#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h> // for getnameinfo()

// Usual socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include "PJ_RPI.h"

#define SIZE 1024
#define BACKLOG 10  // Passed to listen()

void report(struct sockaddr_in *serverAddress);

void setHttpHeader(char httpHeader[])
{
    // File object to return
    FILE *htmlData = fopen("index.html", "r");

    char line[100];
    char responseData[8000];
    while (fgets(line, 100, htmlData) != 0) {
        strcat(responseData, line);
    }
    // char httpHeader[8000] = "HTTP/1.1 200 OK\r\n\n";
    strcat(httpHeader, responseData);
}

void SendDataToClient (int clientSocket){
        int outputs[3];
        int inputs[3];

        outputs[0] = 1;
        outputs[1] = 0;
        outputs[2] = 1;

        inputs[0] = GPIO_READ(27);
        inputs[1] = GPIO_READ(22);
        inputs[2] = GPIO_READ(26);

        char test[20];

        for(int i = 0; i < 6 ; i++ ){
            if(i == 0){
                sprintf(test, "<p>Input GPIO 27: %d </p>", inputs[0]);
            }
            if(i == 1){
                sprintf(test, "<p>Input GPIO 22: %d </p>", inputs[1]);
            }
            if(i == 2){
                sprintf(test, "<p>Input GPIO 26: %d </p>", inputs[2]);
            }
            if(i == 3){
                sprintf(test, "<p>Output GPIO 17: %d </p>", outputs[0]);
            }
            if(i == 4){
                sprintf(test, "<p>Output GPIO 13: %d </p>", outputs[1]);
            }
            if(i == 5){
                sprintf(test, "<p>Output GPIO 19: %d </p>", outputs[2]);
            }
            
            send(clientSocket, test, sizeof(test), 0);
        }
        send(clientSocket, "</body>", sizeof("</body>"), 0);
        send(clientSocket, "</html>", sizeof("</html>"), 0);

}
void error(const char *msg)
{
    perror(msg);
    exit(1);
}


int main(void)
{
    if(map_peripheral(&gpio) == -1) 
	{
       	 	printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        	return -1;
    }

    // Define gpio 27,22,26 as input
	INP_GPIO(27); // input
	INP_GPIO(22); // input
	INP_GPIO(26); // input

    OUT_GPIO(17); // output
	OUT_GPIO(13); // output
	OUT_GPIO(19);  // output

    char httpHeader[8000] = "HTTP/1.1 200 OK\r\n\n"; // HTTP header

    int pid;

    // Socket setup: creates an endpoint for communication, returns a descriptor
    // -----------------------------------------------------------------------------------------------------------------
    int serverSocket = socket(
        AF_INET,      // Domain: specifies protocol family
        SOCK_STREAM,  // Type: specifies communication semantics
        0             // Protocol: 0 because there is a single protocol for the specified family
    );

    // Construct local address structure
    // -----------------------------------------------------------------------------------------------------------------
    struct sockaddr_in serverAddress; // Local address
    serverAddress.sin_family = AF_INET; // Protocol family
    serverAddress.sin_port = htons(8001); // Port number
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//inet_addr("127.0.0.1");

    // Bind socket to local address
    // -----------------------------------------------------------------------------------------------------------------
    // bind() assigns the address specified by serverAddress to the socket
    // referred to by the file descriptor serverSocket.
    bind(
        serverSocket,                         // file descriptor referring to a socket
        (struct sockaddr *) &serverAddress,   // Address to be assigned to the socket
        sizeof(serverAddress)                 // Size (bytes) of the address structure
    );

    // Mark socket to listen for incoming connections
    // -----------------------------------------------------------------------------------------------------------------
    int listening = listen(serverSocket, BACKLOG);
    if (listening < 0) {
        printf("Error: The server is not listening.\n");
        return 1;
    }
    report(&serverAddress);     // Custom report function
    setHttpHeader(httpHeader);  // Custom function to set header
    int clientSocket;

    // Wait for a connection, create a connected socket if a connection is pending
    // -----------------------------------------------------------------------------------------------------------------
    while(1) {
        clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             SendDataToClient (clientSocket);
             close(clientSocket);

             exit(0);
            
         }
         else{
            send(clientSocket, httpHeader, sizeof(httpHeader), 0);
            close(clientSocket);
         }
         
    }
    close(clientSocket);
    return 0; /* we never get here */
}

void report(struct sockaddr_in *serverAddress)
{
    char hostBuffer[INET6_ADDRSTRLEN];
    char serviceBuffer[NI_MAXSERV]; // defined in `<netdb.h>`
    socklen_t addr_len = sizeof(*serverAddress);
    int err = getnameinfo(
        (struct sockaddr *) serverAddress,
        addr_len,
        hostBuffer,
        sizeof(hostBuffer),
        serviceBuffer,
        sizeof(serviceBuffer),
        NI_NUMERICHOST
    );
    if (err != 0) {
        printf("It's not working!!\n");
    }
    printf("\n\n\tServer listening on http://%s:%s\n", hostBuffer, serviceBuffer);
}