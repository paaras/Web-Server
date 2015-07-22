// Paaras Chand
// 904 052 608

/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/

#include <sys/types.h>   // definitions of a number of data types used in 
			 // socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, 
			 // e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet 
			 // domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void dostuff(int); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     struct sigaction sa; // for signal SIGCHLD

      /*
        struct sockaddr {
                usigned short sa_family; // address family, AF_XXX
                char sa_data[14]; // 14 bytes of protocol address
        };

        //IPv4 AF_INET sockets
        struct sockaddr_in {
                short sin_family; // AF_INET
                unsigned short sin_port; // Port Number
                struct in_addr sin_addr; // A seperate address struct
                char sin_zero[8]; // 0
        }

        struct in_addr {
                unsigned long s_addr; // Address
        };
        */

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     // Create Socket
     sockfd = socket(AF_INET, SOCK_STREAM, 0); //IPv4, TCP, protocol
     if (sockfd < 0) 
        error("ERROR opening socket");
     
     // Sockaddr structure values
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     
     // Bind
     // int bind(int sockfd, struct sockaddr *my_addr, int addrlent)
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     // Listen
     // int listen(int sockfd, int backlog); // backlog = max connections
     listen(sockfd,5);
     
     clilen = sizeof(cli_addr);
     
     /****** Kill Zombie Processes ******/
     sa.sa_handler = sigchld_handler; // reap all dead processes
     sigemptyset(&sa.sa_mask);
     sa.sa_flags = SA_RESTART;
     if (sigaction(SIGCHLD, &sa, NULL) == -1) {
         perror("sigaction");
         exit(1);
     }
     /*********************************/
    
     // Accept
     // Three way handshake -  the client will get its very own sockfd
     // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) 
     while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         
         if (newsockfd < 0) 
             error("ERROR on accept");
         
         pid = fork(); //create a new process
         if (pid < 0)
             error("ERROR on fork");
         
         if (pid == 0)  { // fork() returns a value of 0 to the child process
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else //returns the process ID of the child process to the parent
             close(newsockfd); // parent doesn't need this 
     } /* end of while */
     return 0; /* we never get here */
}

// 404 Message, File Not Found
static char *ok_404 = 
  "HTTP/1.1 404 Bad Request\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  "<body>\n"
  "<h1>404: Not Found</h1>\n"
  "<p>The requested URL  was not found.</p>\n"
  "</body>\n"
  "</html>\n";

// Server Response for HTML/HTM content
static char *ok_200text =
 "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

// Server Response for GIF content
static char *ok_200gif = 
 "HTTP/1.1 200 OK\r\nContent-type: image/gif\r\n\r\n";

// Server Response for JPG/JPEG content
static char *ok_200jpeg = 
 "HTTP/1.1 200 OK\r\nContent-type: image/jpeg\r\n\r\n";

void dostuff (int sock)
{
   int n;
   char buffer[2048];
   
   // Read the message from the browser and put it into buffer
   bzero(buffer, 2048);
   n = read(sock, buffer,2048);
   if (n < 0) error("ERROR reading from socket");
   printf("CLIENT MESSAGE:\n%s\n",buffer);
   
   // Extract the URL from the message
   char method[140], url[140];
   sscanf(buffer, "%s%s", method, url);
   int s = strlen(url);
   char trUrl [140];
   int c;
   int d = 0;
   for (c = 1; c < strlen(url); c++)
   {
        trUrl[d]=url[c];
        d++;
   }
   
   // Then check if that file actually exists on our server
   // If it doesn't then display a 404
   char *message = ok_404;
   FILE *fp;
   fp = fopen(trUrl,"r");
   if (fp == NULL)
	n = write(sock, ok_404, strlen(ok_404));
   else 
   {
	// If the file does exist than check if it's of type
        // html, htm, jpeg, jpg, or gif
        // This will help us choose the apporoprate server response message
 	char *index1, *index2, *index3, *index4, *index5;
        index1 = (char *) strstr(trUrl, "html");
        if (index1 != NULL)
            message = ok_200text;

	index2 = (char *) strstr(trUrl, "htm");
        if (index2 != NULL)
            message = ok_200text;

        index3 = (char *) strstr(trUrl, "jpeg");
        if (index3 != NULL)
            message = ok_200jpeg;

	index4 = (char *) strstr(trUrl, "jpg");
        if (index4 != NULL)
            message = ok_200jpeg;

	index5 = (char *) strstr(trUrl, "gif");
        if (index5 != NULL)
            message = ok_200gif;

	// Send our the header of the response message
	n = write(sock, message, strlen(message));
        if (n < 0) error("ERROR writing to socket");
   }

   // If the file is an image file (JPEG, JPG, or gif)
   // then we need to use the fread method to create a 
   // buffer
   if (message == ok_200gif || message == ok_200jpeg)
   {
   size_t size;
   for(;;)
   {
	size = fread((char *)buffer, sizeof(char), 2048, fp);
	n = write(sock, buffer, size);
        if (n < 0) error("ERROR writing to socket");
        if (size < 2048)
            break;
   }
   }
   
   // If the file is a text file (HTML, HTM) then we can use 
   // getc to create a buffer (get one character from the file
   // put it into the buffer then get the next character...)
   else
   {
   int filesize = 0;
   int curr = getc(fp);
   for (curr; curr != EOF; curr = getc(fp)) {
          buffer[filesize] = (char)curr;
          filesize++;
   }
   buffer[filesize] = '\0';
   n = write(sock, buffer, strlen(buffer));
   if (n < 0) error("ERROR writing to socket");
   }
}
