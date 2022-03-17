#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include "bbuffer.h"

#define PORT 8028
#define MAXREQ (4096*1024)

char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];
char *root_path;

FILE *content;

void error(const char* msg){
    perror(msg);
    exit(1);
}

void get_destination(char *buffer, char *address_buffer){
    int i = 0;
    int condition = 0;
    int j = 0;
    while(1){
        if (buffer[j] == '\n' || buffer[j] == '\r'){
            address_buffer[i+1] = '\0';
            break;
        }

        if(condition == 0 && buffer[j] == ' '){
            j++;
            condition = 1;
        }
        if(condition == 1){
            if (buffer[j]== ' '){
                address_buffer[i+1] = '\0';
                break;
            }
            address_buffer[i] = buffer[j];
            i++;
        }
        j++;
    }

    return;
}

int main(int argc, char *argv[]){
    

    char address_buffer[256];
    char total_adress[256];

    root_path = getenv("PWD");

    strcpy(total_adress, root_path);

    if(argc >= 2){
        root_path = argv[1];
    }


    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd<0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding");
    }
    listen(sockfd, 5);

    while(1){
        strcpy(total_adress, root_path);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if(newsockfd < 0){
            error("ERROR on accept");
        }
        bzero(buffer, sizeof(buffer));
        n = read(newsockfd, buffer, sizeof(buffer)-1);
        
        if(n < 0){
            error("ERROR reading from socket");
        }

        snprintf(body, sizeof(body),
        "<html>\n<body>\n<h1>Hello web browser</h1>Your request was\n<pre>%s</pre>\n</body>\n</html>\n", buffer);
        

        

    

        bzero(address_buffer, sizeof(address_buffer));
        get_destination(buffer, address_buffer);
        
        strcat(total_adress, address_buffer);

        printf("The adress is %s \n", total_adress);

        char * file_buffer = 0;
        long length;
        FILE * f = fopen (total_adress, "rb");
         bzero(total_adress, sizeof(total_adress));
        if (f){
            fseek (f, 0, SEEK_END);
            length = ftell (f);
            fseek (f, 0, SEEK_SET);
            file_buffer = malloc (length);
            if (file_buffer)
                {
                    fread (file_buffer, 1, length, f);
                }
            fclose (f);
        }

        snprintf(msg, sizeof(msg),
        "HTTP/1.0 200 OK\n Content-Type: text/html\n Content-Length: %lu\n\n%s", strlen(file_buffer), file_buffer);

        n = write(newsockfd, msg, strlen(msg));
        

        if (n<0){
            error("ERROR writing to socket");
        }

        close(newsockfd);
        
    }
}