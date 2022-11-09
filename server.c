#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections*/
#define READ 0
#define WRITE 1

bool hasData = false;

void readFromPipe(int fd[], int connfd);

int main (int argc, char **argv)
{
 int listenfd, connfd, n;
 pid_t childpid;
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;
 int count = 0;

 //Pipes
 int fd1[2];
 int fd2[2];
 pipe(fd1);
 pipe(fd2);

 //Create a socket for the server
 //If sockfd<0 there was an error in the creation of the socket
 if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }


 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(SERV_PORT);

 //bind the socket
 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 //listen to the socket by creating a connection queue, then wait for clients
 listen (listenfd, LISTENQ);

 printf("%s\n","Server running...waiting for connections.");

 for ( ; ; ) {

  clilen = sizeof(cliaddr);
  //accept a connection
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

  printf("%s\n","Received request...");
  count++;
  if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process
    int childPID;
    printf ("%s\n","Child created for dealing with client requests");

    //close listening socket
    close (listenfd);

    //Creating a new child for reading data from the pipe
    childPID = fork();
    if(childPID == 0)//Child
    {
      if(count == 1)
      {
        readFromPipe(fd2, connfd);
      }
      else
      {
        readFromPipe(fd1, connfd);
      }
      exit(0);
    }

    while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)   {

      printf("%s","String received from the client:");
      puts(buf);

      if(count == 1)
      {
        //Writing the data to the pipe
        close(fd1[READ]);
        write(fd1[WRITE], buf, strlen(buf) + 1);

        //send(connfd, buf, n, 0); //Send message stating string recieved
      }
      else if(count == 2)
      {
        //Writing the data to the pipe
        close(fd2[READ]);
        write(fd2[WRITE], buf, strlen(buf) + 1);
        
        //send(connfd, buf, n, 0); //Send message stating string recieved
      }
      
      strcpy(buf,"");
    }

    if (n < 0)
      printf("%s\n", "Read error");
    exit(0);
  }
 //close socket of the server
 close(connfd);
 
 }

}

void readFromPipe(int fd[], int connfd)
{
  while(1)
  {
    char str[MAXLINE] = "";
    close(fd[WRITE]);
    int n = read(fd[READ], &str, sizeof(str));
    send(connfd, str, n, 0); //Send message stating string recieved
  }
}
