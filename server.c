/*
============================================================================
Name : server.c
Author : Himarishitha Kakunuri
Description : Server Side Program
============================================================================
*/


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h> 
#include<stdbool.h>
#include<errno.h> 


#include "./handlers/admin.h"
#include "./handlers/faculty.h"



#define INITIAL_PROMPT "\t.......... Welcome Back To Academia :: Course Registration..........\nLogin Type\nEnter Your Choice {1.Admin, 2.Faculty, 3.Student}:"


int handle_client(int csfd) {
    int ch;
    char rd[1024],wr[1024];
    
    write(csfd,INITIAL_PROMPT,strlen(INITIAL_PROMPT));
    bzero(rd,sizeof(rd));
    int rdb = read(csfd,rd,sizeof(rd));
    if(rdb==-1)
      perror("Error while reading from client");
    else if(rdb==0)
      printf("No data sent from client");
    else{
       ch = atoi(rd);
       switch(ch){
        case 1:
                admin_handler(csfd);
                break;
        case 2:
                faculty_handler(csfd);
                break;
        case 3:
                //student_handler(csfd);
                break;         
        default:
                write(csfd,"Invalid Choice ^",14);
                char rd[100];
                int rdb = read(csfd,read,sizeof(read));
                break;
       }
    }    
    close(csfd);
    return 1;
}


int main(){

	struct sockaddr_in server,client;
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	server.sin_family=AF_INET;    
	server.sin_addr.s_addr=INADDR_ANY; 
	server.sin_port=htons(8009);
	int bs = bind(sfd,(struct sockaddr *)&server,sizeof(server));
	if(bs==-1)
	{
		perror("Error while binding");
		exit(0);
	}
	int ls = listen(sfd,5);	
	if(ls==-1)
	{
		perror("Error while Listening");
		exit(0);
	}     
	printf("Waiting for Client to connect\n");
	while(1){
		int clisize = sizeof(client);
		int cfd=accept(sfd,(struct sockaddr *)&client,&clisize);
		printf("Connected with client\n");
		if(!fork()){
			close(sfd);
			handle_client(cfd);
			exit(0);
		}
		else{
			close(cfd);
		}
	}
	return 0;
}
