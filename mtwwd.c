#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include "bbuffer.h"
#include <pthread.h>

#define MAXREQ (4096*1024)

char buffer[MAXREQ], body[MAXREQ], msg[MAXREQ];
char *www_path;
int port;
int n_threads;
int n_bufslots;


BNDBUF *bb;

void error(const char* msg){
    perror(msg);
    exit(1);
}


/* wtf is this magic */
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

void parse_args(int argc, char*argv[]) {
    if(argc < 5) {
        // Default args so i dont have to specify every time while programming
        www_path = getenv("PWD");
        strcat(www_path, "/www");
        port = 8028;
        if(argc == 2) {
            port = atoi(argv[1]);
        }
        n_threads = 50;
        n_bufslots = 50;
    } else {
        www_path = argv[1];
        port = atoi(argv[2]);
        n_threads = atoi(argv[3]);
        n_bufslots = atoi(argv[4]);
    }
}

void *handle_request() {
    char address_buffer[256];
    char total_adress[256];
    int err_404 = 0;
    int n;
    int newsockfd;

    FILE *content;
    while(1) {
        newsockfd = bb_get(bb);
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
        
        strcpy(total_adress, www_path);
        strcat(total_adress, address_buffer);
    
        char * file_buffer = 0;
        long length;
        
        
        FILE * f = fopen (total_adress, "r+");
        if (f == NULL ){
            strcpy(total_adress, www_path);
            strcat(total_adress, "/404.html");
            printf("Response adress: %s \n", total_adress);
            f = fopen (total_adress, "rb");
            err_404 = 1;
        }
        err_404 = 0;
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        file_buffer = malloc (length);
        if (file_buffer)
            {
                fread (file_buffer, 1, length, f);
            }
        fclose (f);

        bzero(total_adress, sizeof(total_adress));
        if(err_404 == 0) {
            snprintf(msg, sizeof(msg),
            "HTTP/1.0 200 OK\n Content-Type: text/html\n Content-Length: %lu\n\n%s", strlen(file_buffer), file_buffer);
        } else {
            snprintf(msg, sizeof(msg),
            "HTTP/1.0 404 Not found\n Content-Type: text/html\n Content-Length: %lu\n\n%s", strlen(file_buffer), file_buffer);
        }

        n = write(newsockfd, msg, strlen(msg));
        

        if (n<0){
            error("ERROR writing to socket");
        }

        close(newsockfd);
        
    }       
}

int main(int argc, char *argv[]){
    
    parse_args(argc, argv);

    pthread_t threads[n_threads];
    int temp_arg[n_threads];
    bb = bb_init(n_bufslots);

    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd<0) error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding");
    }
    for(int i = 0; i<n_threads;i++) {
        temp_arg[i] = i;
        int res = pthread_create(&threads[i], NULL, &handle_request, &temp_arg[i]);
    }
    listen(sockfd, 5);
    int cccc = 0;
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        bb_add(bb, newsockfd);
        cccc+=1;
        printf("%d\n",cccc);
    }
}

