/*
============================================================================
Name : login.h
Author : Himarishitha Kakunuri
Description : To check credentials   
============================================================================
*/


#include <stdio.h>     
#include <unistd.h>    
#include <string.h>    
#include <stdbool.h>   
#include <sys/types.h> 
#include <sys/stat.h>  
#include <fcntl.h>     
#include <stdlib.h>    
#include <errno.h>     


#include "../student.h"
#include "../faculty.h"
#include "../course.h"
#include "../enrollment.h"


bool login_handler(int user, int cfd, struct Faculty *f, struct Student *s);

bool login_handler(int user, int cfd, struct Faculty *f,struct Student *s)
{
    ssize_t rdb, wrb;            
    char rd[1024], wr[1024],temp[1000];
    struct Faculty faculty;
    struct Student student;
    int id;

    bzero(rd, sizeof(rd));
    bzero(wr, sizeof(wr));
    strcat(wr, "\n");
    strcat(wr, "Enter your Login ID:");

    
    wrb = write(cfd, wr, strlen(wr));
    if (wrb == -1)
    {
        perror("Error while writing");
        return false;
    }

    rdb = read(cfd, rd, sizeof(rd));
    if (rdb == -1)
    {
        perror("Error while reading login ID");
        return false;
    }

    bool userfound = false;

    if (user==1)
    {
        if (strcmp(rd, "iiitb") == 0)
            userfound = true;    
    }

  
    else if(user==2)
    {

        bzero(temp, sizeof(temp));
        strcpy(temp, rd);
        int id;

        char *start = NULL;
        char *position = strstr(temp,"F-");
        if (position != NULL) {
           start = position + strlen("F-");
           id = atoi(start);
        } 

        int ffd = open("faculty", O_RDONLY);
        if (ffd == -1)
        {
            perror("Error opening faculty file.");
            return false;
        }

        
        off_t offset = lseek(ffd,(id-1) * sizeof(struct Faculty), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, (id-1) * sizeof(struct Faculty), sizeof(struct Faculty), getpid()};

            int status = fcntl(ffd, F_SETLKW, &lock);
            if (status == -1)
            {
                perror("Error obtaining read lock on faculty record");
                return false;
            }

            rdb = read(ffd, &faculty, sizeof(struct Faculty));
            if (rdb == -1)
            {
                perror("Error reading faculty record from faculty file");
            }

            lock.l_type = F_UNLCK;
            fcntl(ffd, F_SETLK, &lock);

            if (strcmp(faculty.login_id, rd) == 0)
                userfound = true;

            close(ffd);
        }
        else
        {
            wrb = write(cfd, "Faculty ID does not exists", strlen("Faculty ID does not exists"));
        }
    }


    else if(user==3){

        bzero(temp, sizeof(temp));
        strcpy(temp, rd);
        int id;

        char *start = NULL;
        
        char *position = strstr(temp,"MT-");
        if (position != NULL) {
           start = position + strlen("MT-");
           id = atoi(start);
        } 

        int sfd = open("student", O_RDONLY);
        if (sfd == -1)
        {
            perror("Error opening student file");
            return 0;
        }

        
        off_t offset = lseek(sfd,(id-1) * sizeof(struct Student), SEEK_SET);
        if (offset >= 0)
        {
            struct flock lock = {F_RDLCK, SEEK_SET, (id-1) * sizeof(struct Student), sizeof(struct Student), getpid()};

            int status = fcntl(sfd, F_SETLKW, &lock);
            if(status == -1)
            {
                perror("Error obtaining read lock");
                return false;
            }

            rdb = read(sfd, &student, sizeof(struct Student));
            if (rdb == -1)
            {
                perror("Error reading student record from student file");
            }

            lock.l_type = F_UNLCK;
            fcntl(sfd, F_SETLK, &lock);

            if (strcmp(student.login_id, rd) == 0)
                userfound = true;

            close(sfd);
        }
        else
        {
            wrb = write(cfd,"Student Id does not exist", strlen("Student Id does not exist"));
        }
    }

    if(userfound)
    {
        bzero(wr, sizeof(wr));
        wrb = write(cfd, "Enter Password:",15);
        if (wrb == -1)
        {
            perror("Error while writing message");
            return false;
        }
        rdb = read(cfd, rd, sizeof(rd));
        if (rdb == -1)
        {
            perror("Error reading password");
            return false;
        }
          
        
        if (user==1)
        {
            if (strcmp(rd,"iiitb") == 0)
                return true;
        }
        else if(user==2)
        {
            if (strcmp(rd, "fac") == 0)
            {
                *f = faculty;
                return true;
            }
        }
        else{
            if (strcmp(rd, "stu") == 0)
            {
                *s = student;
                return true;
            }
        }
         
        bzero(wr, sizeof(wr));        
        wrb = write(cfd, "Invalid Credentials ^", 21);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }
    else
    {
        bzero(wr, sizeof(wr));
        wrb = write(cfd,"Invalid Login ID ^",18);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }

    return false;
}
