/*
============================================================================
Name : client.c
Author : Himarishitha Kakunuri
Description : Client Side Program
============================================================================
*/



#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>  
#include<arpa/inet.h>  
#include<stdbool.h>
#include<strings.h>
#include<string.h>


int connection_handler(int sfd)
{
    char rd[1024], wr[1024],temp[1024]; 
    ssize_t rdb, wrb;           

    do
    {
        bzero(rd, sizeof(rd)); 
        bzero(temp, sizeof(temp));
        
        rdb = read(sfd, rd, sizeof(rd));
        if (rdb == -1)
            perror("Error while reading");
        else if (rd== 0){
            return 0;
        }
        else if (strchr(rd, '^') != NULL)
        {
            strncpy(temp, rd, strlen(rd) - 1);
            printf("%s\n", temp);
            wrb = write(sfd, "^", strlen("^"));
            if (wrb == -1)
            {
                perror("Error while writing");
                break;
            }
        }
        else if (strchr(rd, '$') != NULL)
        {
            strncpy(temp, rd, strlen(rd) - 2);
            printf("%s\n", temp);
            printf("Closing the connection");
            break;
        }
        else
        {
            bzero(wr, sizeof(wr));
            if (strchr(rd, '#') != NULL)
                strcpy(wr, getpass(rd));
            else{
               printf("%s", rd);
               scanf("%[^\n]%*c", wr); 
            }
            wrb = write(sfd, wr, strlen(wr));
            if (wrb == -1){
                 perror("Error while writing");
                 printf("Closing the connection\n");
                 break;
            }
        }
    } while(rdb > 0);

    close(sfd);
}

int main(){
	struct sockaddr_in server;    
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(8009);
	int cs=connect(sfd,(struct sockaddr *)&server,sizeof(server));
	if(cs==-1)
	{
		perror("Error while Connecting");
		exit(0);
	}     
	printf("Connection established successfully");
	connection_handler(sfd);
	close(sfd);
	return 0;
}
