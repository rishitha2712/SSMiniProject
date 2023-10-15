
/*
============================================================================
Name : student.h
Author : Himarishitha Kakunuri
Description : To implement student functionalities.     
============================================================================
*/

#include <sys/ipc.h>


struct Student loggedInStudent;
struct Enrollment enroll;

int student_handler(int cfd);
int view_all_courses(int cfd);
int enroll_course(int cfd);
int view_enrolled_courses(int cfd);
int drop_course(int cfd);
int change_password(int cfd);
int Logout(int cfd);


int student_handler(int cfd){
    

    if(login_handler(3,cfd,NULL,&loggedInStudent)){
        
        ssize_t wrb, rdb;            
        char rd[1000], wr[1000];
        bzero(wr, sizeof(wr));

        strcpy(wr,"\n Login Successful");
 
        while(1){

            strcat(wr, "\n");
            strcat(wr, "......Welcome to Student Menu......\n\t1.View All Courses\n\t2.Enroll new course\n\t3.Drop course\n\t4.View enrolled course details\n\t5.Change password\n\t6.Logout and Exit\n\nEnter your choice:"
);
            wrb = write(cfd, wr, strlen(wr));
            bzero(wr, sizeof(wr));

            rdb = read(cfd, rd, sizeof(rd));
            if (rdb == -1)
            {
                perror("Error while reading choice");
                return 0;
            }

            int ch = atoi(rd);
            switch (ch){
                case 1:
                    view_all_courses(cfd);
                    break;
                case 2:
                    enroll_course(cfd);
                    break;
                case 3: 
                    drop_course(cfd);
                    break;
                case 4:
                    view_enrolled_courses(cfd);
                    break;
                case 5:
                    //change_password(cfd);
                    break;
                case 6:
                    Logout(cfd);
                    break;    
                default:
                    wrb = write(cfd,"Invalid choice ^",14);
                    rdb = read(cfd,rd,sizeof(rd));
                    break;
            }
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

int drop_course(int cfd){

  ssize_t rdb, wrb;             
  char rd[1000], wr[10000];
  int efd, coursefd;
  bool flag;
  struct Enrollment enroll;
  struct Course course;
  int enrollID,n;
  struct flock lock = {F_WRLCK, SEEK_SET, 0, sizeof(struct Enrollment), getpid()};
  
  if(strcmp(loggedInStudent.access,"blocked")==0){
    write(cfd,"You are blocked by admin ^",26);
    rdb = read(cfd,rd,sizeof(rd));
    return 0;
  }

 
  wrb = write(cfd,"Enter the course id you want to drop:",37);
  rdb = read(cfd,rd,sizeof(rd));
  if(rdb==-1){
    perror("Error reading course id");
    return 0;
  }
  
  efd = open("enroll",O_RDONLY);
  while((n = read(efd, &enroll, sizeof(struct Enrollment))) > 0) {
    if((strcmp(enroll.status,"enrolled")==0) && (strcmp(enroll.studentid,loggedInStudent.login_id)==0) && (strcmp(enroll.courseid,rd)==0)){              
        enrollID= enroll.id;
        flag=true;
        break;  
    }
  }
  close(efd);
  
  if(flag==false){
    write(cfd,"Invalid course id ^",19);
    rdb = read(cfd,rd,sizeof(rd));
    return 0;
  } 


  efd = open("enroll",O_RDONLY);
  int offset = lseek(efd,(enrollID-1)*sizeof(struct Enrollment),SEEK_SET);
  if (offset == -1){
    perror("Error while seeking");
    return 0;
  }

  lock.l_type = F_RDLCK;
  lock.l_start = offset;
  int status = fcntl(efd, F_SETLKW, &lock);
  if (status == -1){
      perror("Error while obtaining read lock");
      return 0;
  }
  rdb = read(efd, &enroll, sizeof(struct Enrollment));
  if(rdb == -1){
    perror("Error while reading enrollment");
    return 0;
  }

  if(strcmp(enroll.status,"unenrolled")==0){
      write(cfd,"Not enrolled in course already ^",sizeof("Not enrolled in course already ^"));
      rdb = read(cfd,rd,sizeof(rd));
      return 0;
  }
  lock.l_type = F_UNLCK;
  status = fcntl(coursefd, F_SETLK, &lock);   
  close(efd);

  strcpy(enroll.status,"unenrolled");
  
  efd = open("enroll",O_WRONLY);
  if (efd == -1){
        perror("Error while opening enrollment file");
        return 0;
  }

  offset = lseek(efd, (enrollID-1) * sizeof(struct Enrollment), SEEK_SET);
  if(offset == -1){
    perror("Error while seeking");
    return 0;
  }

  lock.l_type = F_WRLCK;
  lock.l_start = offset;
  status = fcntl(efd, F_SETLKW, &lock);
  if(status == -1){
    perror("Error obtaining write lock");
    return 0;
  }
  wrb = write(efd, &enroll, sizeof(struct Enrollment));
  if (wrb == -1){
    perror("Error updating enroll");
  }

  lock.l_type = F_UNLCK;
  fcntl(efd, F_SETLKW, &lock);
  close(efd);
  
  int courseID;
  coursefd = open("course",O_RDONLY);
  while((n = read(coursefd, &course, sizeof(struct Course))) > 0) {
    if((strcmp(course.status,"notactive")!=0) && (strcmp(course.courseid,rd)==0)){              
        courseID= course.id;
        break;  
    }
  }
  close(coursefd);
  
  coursefd = open("course",O_RDONLY);
  offset = lseek(coursefd,(courseID-1)*sizeof(struct Course),SEEK_SET);
  if (offset == -1){
    perror("Error while seeking");
    return 0;
  }
  lock.l_type = F_RDLCK;
  lock.l_start = offset;
  status = fcntl(coursefd, F_SETLKW, &lock);
  if (status == -1){
      perror("Error while obtaining read lock");
      return 0;
  }
  rdb = read(coursefd, &course, sizeof(struct Course));
  if(rdb == -1){
    perror("Error while reading course");
    return 0;
  }
  lock.l_type = F_UNLCK;
  status = fcntl(coursefd, F_SETLK, &lock);   
  close(efd);


  course.no_of_available_seats=course.no_of_available_seats+1;
  
  coursefd = open("course",O_WRONLY);
  if (coursefd == -1){
        perror("Error while opening course file");
        return 0;
  }
  offset = lseek(coursefd, (courseID-1) * sizeof(struct Course), SEEK_SET);
  if(offset == -1){
    perror("Error while seeking");
    return 0;
  }
  lock.l_type = F_WRLCK;
  lock.l_start = offset;
  status = fcntl(coursefd, F_SETLKW, &lock);
  if(status == -1){
    perror("Error while obtaining write lock");
    return 0;
  }
  wrb = write(coursefd, &course, sizeof(struct Course));
  if (wrb == -1){
    perror("Error while writing");
  }

  lock.l_type = F_UNLCK;
  fcntl(coursefd, F_SETLKW, &lock);
  close(coursefd);

  
  wrb = write(cfd,"You have unenrolled successfully ^",strlen("You have unenrolled successfully ^"));
  if(wrb == -1){
      perror("Error writing message to client");
      return false;
  }
  rdb = read(cfd,rd,sizeof(rd));
  return 0;
}

int view_enrolled_courses(int cfd){
    ssize_t rdb, wrb;             
    char rd[1000], wr[10000]; 
    char temp[1000];
    char temp1[5][10];
    int i,n;
    int efd;
    int coursefd;
    struct Enrollment enroll;
    struct Course course;
    bool flag;
    

    if(strcmp(loggedInStudent.access,"blocked")==0){
       write(cfd,"You are blocked by admin ^",26);
       rdb = read(cfd,rd,sizeof(rd));
       return 0;
    }

    efd = open("enroll",O_RDONLY);
    while((n = read(efd, &enroll, sizeof(struct Enrollment))) > 0) {
        if((strcmp(enroll.status,"enrolled")==0) && (strcmp(enroll.studentid,loggedInStudent.login_id)==0)){              
           flag=true;
           strcpy(temp1[i],enroll.courseid);
           i++;   
        }
    }

    
    if(flag==false){
       write(cfd,"No enrolled courses ^",21);
       rdb = read(cfd,rd,sizeof(rd));  
    }

    bool stat[20]={false};

    for(int i=0;i<5;i++){
        coursefd = open("course",O_RDONLY);
        while((n = read(coursefd, &course, sizeof(struct Course))) > 0) {
            if(stat[course.id]==false && (strcmp(course.status,"notactive")!=0) && (strcmp(course.courseid,temp1[i])==0)){                 
                bzero(wr,sizeof(wr));
                stat[course.id]=true;   
                sprintf(wr, " ^ ********* Course Details *********  \n\tName: %s\n\tNo of Seats: %d\n\tCredits : %d\n\tNo of available seats: %d\n\tCourse-id: %s\n", course.name, course.no_of_seats,course.credits,course.no_of_available_seats,course.courseid);
                wrb = write(cfd, wr, strlen(wr));
                if(wrb == -1){
                    perror("Error writing course");
                    return 0;
                }
                rdb = read(cfd,rd,sizeof(rd));  
                break;
            } 
        }
        close(coursefd);
    }
    return 1;   
}


int enroll_course(int cfd){
    ssize_t rdb, wrb;             
    char rd[1000], wr[10000]; 
    char tempBuffer[1000];
    struct Course getcourse;
    int coursefd;
    struct flock lock = {F_WRLCK, SEEK_SET, 0, sizeof(struct Course), getpid()};
    

    if(strcmp(loggedInStudent.access,"blocked")==0){
       write(cfd,"You are blocked by admin ^",26);
       rdb = read(cfd,rd,sizeof(rd));
       return 0;
    }

    coursefd = open("course", O_RDONLY);
    if (coursefd == -1)
    {
        bzero(wr, sizeof(wr));
        strcpy(wr, "course file doesn't exists");
        wrb = write(cfd, wr, strlen(wr));
        if (wrb ==-1)
        {
            perror("Error while writing message to client");
            return 0;
        }
        return 0;
    }

    int n;
    int i=1;
    char temp1[5][10];

    strcpy(rd,"\n........Available Courses.......\n");
    while((n = read(coursefd, &getcourse, sizeof(struct Course))) > 0) {
        char temp[200];
        if(strcmp(getcourse.status,"notactive")==0)
           continue;
        strcpy(temp1[i],getcourse.courseid);
        i++;   
        strcpy(temp,getcourse.courseid);
        strcat(temp,getcourse.name);
        strcat(temp,"\n");   
        strcat(rd,temp);   
    }
    close(coursefd);

    
    strcat(rd,"\nEnter course id you want to enroll:");
    wrb = write(cfd, rd, strlen(rd));
    if(wrb == -1){
        perror("Error writing message to client");
        return 0;
    }
    bzero(rd,sizeof(rd));
    rdb= read(cfd,rd,sizeof(rd));
    if(rdb==-1){
        perror("Error reading course id");
        return 0;
    }

    int courseID;
    for(int i=0;i<5;i++){
        if(strcmp(rd,temp1[i])==0){
            char *position = strstr(rd, "C-");
            char *start = NULL;
            if(position!=NULL) {
               start = position + strlen("C-");
               courseID = atoi(start);
            }

            struct Course course;
            coursefd = open("course", O_RDONLY);
            int offset = lseek(coursefd, (courseID-1) * sizeof(struct Course), SEEK_SET);
            if (offset == -1){
               perror("Error while seeking");
               return 0;
            }
            
            lock.l_type = F_RDLCK;
            lock.l_start = offset;
            int status = fcntl(coursefd, F_SETLKW, &lock);
            if (status == -1){
              perror("Error while obtaining read lock");
              return 0;
            }

            rdb = read(coursefd, &course, sizeof(struct Course));
            if(rdb == -1){
                perror("Error while reading course record");
                return 0;
            }
            lock.l_type = F_UNLCK;
            status = fcntl(coursefd, F_SETLK, &lock);
            
            if(course.no_of_available_seats==0){
               write(cfd,"No available seats to enroll ^",30);
               close(coursefd);
               rdb = read(cfd,rd,sizeof(rd));
               return 0;
            }
            else{
                int n,enrollID;
                bool flag=false;

                int enrollfd = open("enroll",O_RDONLY);
                struct Enrollment enroll; 
                
                while((n = read(enrollfd,&enroll, sizeof(struct Enrollment)))> 0){

         
                   if((strcmp(rd,enroll.courseid)==0) && (strcmp(loggedInStudent.login_id,enroll.studentid)==0) && (strcmp(enroll.status,"enrolled")==0)){
                       wrb = write(cfd, "Already enrolled ^",18);
                       rdb = read(cfd,rd,sizeof(rd));
                       return 0;                
                   }

                  
                   else if((strcmp(rd,enroll.courseid)==0) && (strcmp(loggedInStudent.login_id,enroll.studentid)==0) && (strcmp(enroll.status,"unenrolled")==0)){
                      flag=true;
                      enrollID= enroll.id;
                      break;                           
                   }       
                }

     
                if(flag==true){

                    close(enrollfd);
                    enrollfd = open("enroll",O_RDONLY);
                    int offset = lseek(enrollfd,(enrollID-1)*sizeof(struct Enrollment),SEEK_SET);
                    if(offset == -1){
                        perror("Error while seeking");
                        return 0;
                    }
                    
                    lock.l_type = F_RDLCK;
                    lock.l_start = offset;
                    int status = fcntl(enrollfd, F_SETLKW, &lock);
                    if(status == -1){
                        perror("Error while obtaining read lock");
                        return 0;
                    }
                    rdb = read(enrollfd, &enroll, sizeof(struct Enrollment));
                    if(rdb == -1){
                        perror("Error while reading enrollment");
                        return 0;
                    }
                    lock.l_type = F_UNLCK;
                    status = fcntl(enrollfd, F_SETLK, &lock);   
                    close(enrollfd);

                
                    strcpy(enroll.status,"enrolled");

                  
                    enrollfd = open("enroll",O_WRONLY);
                    if (enrollfd == -1){
                        perror("Error while opening enrollment file");
                        return 0;
                    }

                    offset = lseek(enrollfd, (enrollID-1) * sizeof(struct Enrollment), SEEK_SET);
                    if(offset == -1){
                      perror("Error while seeking");
                      return 0;
                    }

                    lock.l_type = F_WRLCK;
                    lock.l_start = offset;
                    status = fcntl(enrollfd, F_SETLKW, &lock);
                    if(status == -1){
                        perror("Error while obtaining write lock");
                        return 0;
                    }
                    wrb = write(enrollfd, &enroll, sizeof(struct Enrollment));
                    if (wrb == -1){
                        perror("Error while updating");
                    }

                     lock.l_type = F_UNLCK;
                     fcntl(enrollfd, F_SETLKW, &lock);
                     close(enrollfd);
                } 

      
                else{
                    if(enrollfd== -1){
                     enroll.id = 1;
                    }
                    else{

                        int offset = lseek(enrollfd,-sizeof(struct Enrollment),SEEK_END);
                        if(offset == -1){
                            perror("Error seeking");
                            return 0;
                        }
                        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Enrollment), getpid()};
                        int status = fcntl(enrollfd, F_SETLKW, &lock);
                        if(status == -1){
                            perror("Error obtaining read lock");
                            return 0;
                        }
                        struct Enrollment previousenroll;
                        rdb = read(enrollfd,&previousenroll, sizeof(struct Enrollment));
                        if(rdb == -1){
                            perror("Error while reading enrollment");
                            return 0;
                        }
                        lock.l_type = F_UNLCK;
                        fcntl(enrollfd, F_SETLK, &lock);
                        close(enrollfd);
 
                        enroll.id = previousenroll.id + 1;
                    }

    
                    strcpy(enroll.courseid,rd);
                    strcpy(enroll.studentid,loggedInStudent.login_id);
                    strcpy(enroll.status,"enrolled");
          

      
                    enrollfd = open("enroll",O_CREAT|O_APPEND|O_WRONLY,S_IRWXU);
                    if(enrollfd == -1){
                        perror("Error while opening enroll file");
                        return 0;
                    }
                    wrb = write(enrollfd, &enroll,sizeof(struct Enrollment));
                    if(wrb == -1){
                        perror("Error while writing enroll");
                        return 0;
                    }
                } 
            }
            close(coursefd);
            
           
            course.no_of_available_seats=course.no_of_available_seats-1;                 
            coursefd = open("course", O_WRONLY);
            if(coursefd == -1){
                perror("Error while opening course file");
                return 0;
            }
            offset = lseek(coursefd, (courseID-1) * sizeof(struct Course), SEEK_SET);
            if (offset == -1){
               perror("Error while seeking");
               return false;
            }

            lock.l_type = F_WRLCK;
            lock.l_start = offset;
            status = fcntl(coursefd, F_SETLKW, &lock);
            if (status == -1){
                perror("Error while obtaining write lock");
                return false;
            }
            wrb = write(coursefd, &course, sizeof(struct Course));
            if (wrb == -1){
                 perror("Error while updating");
            }
            lock.l_type = F_UNLCK;
            fcntl(coursefd, F_SETLKW, &lock);
            close(coursefd);

            
            wrb = write(cfd,"Enrolled successfully ^",strlen("Enrolled successfully ^"));
            if(wrb == -1){
                perror("Error writing message to client");
                return false;
            }
            rdb = read(cfd,rd,sizeof(rd));
            return 0;
        }
    }
    wrb= write(cfd,"Invalid course ^",16);
    if(wrb==-1){
        perror("Error writing message to client");
        return 0;
    }
    rdb = read(cfd,rd,sizeof(rd));
    return 1;
}


int view_all_courses(int cfd){
    
    ssize_t rdb, wrb;             
    char rd[1000], wr[10000]; 
    char tempBuffer[1000];
    struct Course getcourse;
    int coursefd;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Course), getpid()};

    if(strcmp(loggedInStudent.access,"blocked")==0){
       write(cfd,"You are blocked by admin ^",26);
       rdb = read(cfd,rd,sizeof(rd));
       return 0;
    }

    coursefd = open("course", O_RDONLY);
    if (coursefd == -1)
    {
        bzero(wr, sizeof(wr));
        strcpy(wr, "course file id doesn't exists");
        wrb = write(cfd, wr, strlen(wr));
        if (wrb ==-1)
        {
            perror("Error while writing message to client");
            return 0;
        }
        return 0;
    }

    int n;
    
    
    while((n = read(coursefd, &getcourse, sizeof(struct Course))) > 0) {
        strcpy(rd,getcourse.status);
        if(strcmp(rd,"notactive")==0)
           continue;
        bzero(wr,sizeof(wr));
        
        sprintf(wr, "  ********* Course Details *********  \n\tName: %s\n\tNo of Seats: %d\n\tCredits : %d\n\tNo of available seats: %d\n\tCourse-id: %s\n", getcourse.name,getcourse.no_of_seats,getcourse.credits,getcourse.no_of_available_seats,getcourse.courseid);
        strcat(wr,"^");
        wrb = write(cfd, wr, strlen(wr));
      
        if(wrb == -1){
          perror("Error writing course");
          return 0;
        }
        rdb = read(cfd,rd,sizeof(rd));
    }
    
    bzero(wr,sizeof(wr));
    strcat(wr, "^");
    wrb = write(cfd, wr, strlen(wr));
    if(wrb == -1){
        perror("Error writing course");
        return 0;
    }
    rdb = read(cfd,rd,sizeof(rd));  
    return 1;   
}


int Logout(int cfd){
    ssize_t rdb, wrb;             
    char rd[1000], wr[1000];
    write(cfd,"You have been logged out successfully ^",strlen("You have been logged out successfully ^"));
    close(cfd);
    return 0;
}

