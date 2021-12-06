/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 57-7 */

/* ud_ucase_cl.c

   A UNIX domain client that communicates with the server in ud_ucase_sv.c.
   This client sends each command-line argument as a datagram to the server,
   and then displays the contents of the server's response datagram.
*/
#include "ud_ucase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

socklen_t len;

int
main(int argc, char *argv[])
{
    struct sockaddr_un svaddr, claddr; 
    int sfd, j;
    size_t msgLen;
    ssize_t numBytes;
    char resp[BUF_SIZE];

    if (argc < 3 || strcmp(argv[1], "--help") == 0) 
        usageErr("%s msg...\n", argv[0]); /* client usage error */

    /* Create client socket; bind to unique pathname (based on PID) */

    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1) 
        errExit("socket");  /* Socket failed */

    memset(&claddr, 0, sizeof(struct sockaddr_un)); /* Clear structure */
    claddr.sun_family = AF_UNIX;                  /* Local address */
    snprintf(claddr.sun_path, sizeof(claddr.sun_path), "/tmp/ud_ucase_cl.%ld", (long) getpid()); 

    if (bind(sfd, (struct sockaddr *) &claddr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind"); 

    /* Construct address of server */

    memset(&svaddr, 0, sizeof(struct sockaddr_un)); /* Clear structure */
    svaddr.sun_family = AF_UNIX;                 /* Local address */
    strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);    /* Server pathname */

    int io = atoi(argv[1]); //io = 1 for read, 2 for write
    int period = atoi(argv[2]); //period = 1 for 1 second, 2 for 2 seconds, etc.
    /* Send messages to server; echo responses on stdout */
    t_data data={io,period}; //data to be sent to server
    
    if (sendto(sfd, &data, sizeof(data), 0, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) != sizeof(data)) 
        fatal("sendto"); /* Send failed */

    t_data response; //data received from server

     while(1)  {
        len = sizeof(struct sockaddr_un); /* Length of address */
        numBytes = recvfrom(sfd, &response, sizeof(response), 0, (struct sockaddr *) &svaddr, &len);    /* Receive datagram */
        if (numBytes == -1)  
            errExit("recvfrom"); /* Receive failed */

        printf("IO: %d, state = %d \n", response.IO, response.period); //print response from server
        
    }
    remove(claddr.sun_path);            /* Remove client socket pathname */
    exit(EXIT_SUCCESS); 
}