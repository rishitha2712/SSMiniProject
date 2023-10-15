/*
============================================================================
Name : faculty.h
Author : Himarishitha Kakunuri
Description : To implement faculty functionalities.     
============================================================================
*/



#include <sys/ipc.h>


struct Faculty loggedInFaculty;

int faculty_handler(int cfd);
int add_course(int cfd);
int view_offering_course(int cfd);
int remove_course(int cfd);
int modify_course(int cfd);
int change_password(int cfd);
int logout(int cfd);


int faculty_handler(int cfd){

    if(login_handler(2,cfd,&loggedInFaculty,NULL)){

        ssize_t wrb, rdb;            
        char rd[1000], wr[1000];
        bzero(wr, sizeof(wr));
        strcpy(wr, "\nLogged in successfully\n");
        
        while(1){
            strcat(wr, "\n");
            strcat(wr, "......Welcome to Faculty Menu......\n\t1.View offering courses\n\t2.Add new course\n\t3.Remove course from catalog\n\t4.Update course details\n\t5.Change password\n\t6.Logout\n\nEnter your choice:");
            
            wrb = write(cfd, wr, strlen(wr));
            if (wrb == -1)
            {
                perror("Error while writing");
                return 0;
            }
            bzero(wr, sizeof(wr));

            rdb = read(cfd, rd, sizeof(rd));
            if (rdb == -1)
            {
                perror("Error while reading ch");
                return 0;
            }

            int ch = atoi(rd);
            switch (ch)
            {
            case 1:
                view_offering_course(cfd);
                break;
            case 2:
                add_course(cfd);
                 break;
            case 3: 
                remove_course(cfd);
                break;
            case 4:
                //modify_course(cfd);
                break;
            case 5:
                //change_password(cfd);
                break;
            case 6:
                logout(cfd);
                break;    
            default:
                wrb = write(cfd,"wrong choice ^",sizeof("wrong choice ^"));
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


int add_course(int cfd){
    ssize_t rdb, wrb;
    char rd[1000], wr[1000];
    struct Course newCourse;
    struct Course prevCourse;


    int coursefd = open("course", O_RDONLY);
    if (coursefd == -1 && errno == ENOENT)
    {
        newCourse.id =1;
    }
    else if (coursefd == -1)
    {
        perror("Error while opening course file");
        return 0;
    }
    else
    {
        int offset = lseek(coursefd, -sizeof(struct Course), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last Course record!");
            return 0;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
        int status = fcntl(coursefd, F_SETLKW, &lock);
        if (status == -1)
        {
            perror("Error obtaining read lock");
            return 0;
        }

        rdb = read(coursefd, &prevCourse, sizeof(struct Course));
        if (rdb == -1)
        {
            perror("Error while reading Course");
            return 0;
        }

        lock.l_type = F_UNLCK;
        fcntl(coursefd, F_SETLK, &lock);

        close(coursefd);

        newCourse.id = prevCourse.id+1;
    }

    
    wrb = write(cfd,"Enter course name:",strlen("Enter course name:"));
    if (wrb == -1)
    {
        perror("Error writing message to client");
        return 0;
    }

    bzero(rd, sizeof(rd));
    rdb = read(cfd, &rd, sizeof(rd));
    if (rdb == -1)
    {
        perror("Error reading course name");
        return 0;
    }

    strcpy(newCourse.name,rd);


    wrb = write(cfd,"Enter no.of total seats:",strlen("Enter no.of total seats:"));
    if (wrb == -1)
    {
        perror("Error writing message to client");
        return 0;
    }

    bzero(rd, sizeof(rd));
    rdb = read(cfd, &rd, sizeof(rd));
    if (rdb == -1)
    {
        perror("Error reading no of seats");
        return 0;
    }
    int seats = atoi(rd);
    newCourse.no_of_seats= seats;


    
    wrb = write(cfd,"Enter credits:",strlen("Enter Credits:"));
    if (wrb == -1)
    {
        perror("Error writing message to client");
        return 0;
    }

    bzero(rd, sizeof(rd));
    rdb = read(cfd, &rd, sizeof(rd));
    if (rdb == -1)
    {
        perror("Error reading credits");
        return 0;
    }
    int credits = atoi(rd);
    newCourse.credits= credits;
     
    newCourse.no_of_available_seats=seats;

    char code[10];
    strcpy(newCourse.courseid,"C-");
    sprintf(code ,"%d",newCourse.id);
    strcat(newCourse.courseid, code);

    strcpy(newCourse.facultyid,loggedInFaculty.login_id);

    strcpy(newCourse.status,"active");
    
    coursefd = open("course", O_CREAT|O_APPEND|O_WRONLY,S_IRWXU);
    if (coursefd == -1)
    {
        perror("Error while creating course file!");
        return 0;
    }

    wrb = write(coursefd, &newCourse, sizeof(struct Course));
    if (wrb == -1)
    {
        perror("Error while writing Course");
        return 0;
    }

    close(coursefd);
    bzero(wr, sizeof(wr));
    
    sprintf(wr, "%s%s%d", "^ Course added successfully\n The courseid is: ", newCourse.courseid,newCourse.id);
    wrb = write(cfd, wr, sizeof(wr));
    
    rdb = read(cfd,rd,sizeof(rd));
    return 1;
}



int remove_course(int cfd){
    
    ssize_t rdb, wrb;
    char rd[1000], wr[1000];
    struct Course course;
    int courseID;
    off_t offset;
    int status;

    wrb = write(cfd, "Enter course id to delete:", strlen("Enter course id to delete:"));
    if (wrb == -1)
    {
        perror("Error while writing message to client");
        return 0;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb == -1)
    {
        perror("Error while reading course ID");
        return 0;
    }

    char *position = strstr(rd, "C-");
    char *start = NULL;
    if(position!=NULL) {
        start = position + strlen("C-");
        courseID = atoi(start);
    }
    else{
        write(cfd,"wrong course id",16);
        return 0;
    }

    int coursefd = open("course", O_RDONLY);
    if (coursefd == -1)
    {
        bzero(wr, sizeof(wr));
        strcpy(wr,"Course file id doesn't exists");
        wrb = write(cfd, wr, strlen(wr));
        if (wrb == -1)
        {
            perror("Error while writing message to client");
            return 0;
        }
        return 0;
    }
    
    offset = lseek(coursefd, (courseID-1)* sizeof(struct Course), SEEK_SET);
    if (errno == EINVAL)
    {
        bzero(wr, sizeof(wr));
        strcpy(wr,"Course id doesn't exists");
        wrb = write(cfd, wr, strlen(wr));
        if (wrb == -1)
        {
            perror("Error while writing message to client");
            return 0;
        }
        return 0;
    }
    else if (offset == -1)
    {
        perror("Error while seeking");
        return 0;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Course), getpid()};
    struct flock lock1 = {F_RDLCK,SEEK_SET,offset,sizeof(struct Enrollment),getpid()}; 
   
    status = fcntl(coursefd, F_SETLKW, &lock);
    if (status == -1)
    {
        perror("Error while obtaining lock on course record");
        return 0;
    }

    rdb = read(coursefd, &course, sizeof(struct Course));
    if (rdb == -1)
    {
        perror("Error while reading course record");
        return 0;
    }


    lock.l_type = F_UNLCK;
    fcntl(coursefd, F_SETLK, &lock);

    close(coursefd);

    
    if(strcmp(loggedInFaculty.login_id,course.facultyid)!=0){
        write(cfd,"Not your course to remove ^",27);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }


    else if(strcmp(course.status,"notactive")==0){
        write(cfd,"Course not found to remove ^",29);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }
   

    coursefd = open("course", O_WRONLY);
    if (coursefd == -1)
    {
        perror("Error while opening course file");
        return 0;
    }
    offset = lseek(coursefd, (courseID-1) * sizeof(struct Course), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking");
        return 0;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    status = fcntl(coursefd, F_SETLKW, &lock);
    if (status == -1)
    {
        perror("Error while obtaining write lock");
        return 0;
    }

    strcpy(course.status,"notactive");

    wrb = write(coursefd, &course, sizeof(struct Course));
    if (wrb == -1)
    {
        perror("Error while updating");
    }

    lock.l_type = F_UNLCK;
    fcntl(coursefd, F_SETLKW, &lock);
    close(coursefd);
 
    
    struct Enrollment enroll;
    int enrollfd;
    bool flag=false;
    int temp1[15][10];
    int i,n;
    int count=0;

    enrollfd = open("enroll",O_RDONLY);
    while((n = read(enrollfd, &enroll, sizeof(struct Enrollment))) > 0) {
       if((strcmp(enroll.status,"enrolled")==0)  && (strcmp(enroll.courseid,rd)==0)){              
         temp1[i][10]= enroll.id;
         i++;
         count++;
         flag=true;  
       }
    }
    
    close(enrollfd);

    if(flag==true){
       for(int i=0;i<count;i++){
            enrollfd = open("enroll",O_RDONLY);
            int offset = lseek(enrollfd,(temp1[i][10]-1)*sizeof(struct Enrollment),SEEK_SET);
            if(offset == -1){
                perror("Error while seeking");
                return 0;
            }
            lock1.l_type = F_RDLCK;
            lock1.l_start = offset;
            int status = fcntl(enrollfd, F_SETLKW, &lock1);
            if (status == -1){
                perror("Error while obtaining read lock");
                return 0;
            }
            rdb = read(enrollfd, &enroll, sizeof(struct Enrollment));
            if(rdb == -1){
                perror("Error while reading enrollment file");
                return 0;
            }
            lock1.l_type = F_UNLCK;
            status = fcntl(enrollfd, F_SETLK, &lock1);   
            close(enrollfd);
            strcpy(enroll.status,"unenrolled");
  
            enrollfd = open("enroll",O_WRONLY);
            if (enrollfd == -1){
                perror("Error while opening enrollment file");
                return 0;
            }
            offset = lseek(enrollfd, (temp1[i][10]-1) * sizeof(struct Enrollment), SEEK_SET);
            if(offset == -1){
                perror("Error while seeking to required enrollment record");
                return 0;
            }
            lock1.l_type = F_WRLCK;
            lock1.l_start = offset;
            status = fcntl(enrollfd, F_SETLKW, &lock);
            if(status == -1){
                perror("Error while obtaining write lock");
                return 0;
            }
            wrb = write(enrollfd, &enroll, sizeof(struct Enrollment));
            if(wrb == -1){
                perror("Error while writing update enrollment info into file");
            }
            lock1.l_type = F_UNLCK;
            fcntl(enrollfd, F_SETLKW, &lock1);
            close(enrollfd);
       }
    }

  
    wrb = write(cfd, "Course successfully deleted ^", strlen("Course successfully deleted ^"));
    if (wrb == -1)
    {
        perror("Error while writing message to client");
        return 0;
    }
    rdb = read(cfd,rd,sizeof(rd));
    return 1;
}

int view_offering_course(int cfd){
    ssize_t rdb, wrb;             
    char rd[1000], wr[10000]; 
    char temp[1000];
    struct Course getcourse;
    int coursefd;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Course), getpid()};

    wrb = write(cfd, "Enter Course Id:", strlen("Enter course id:"));
    if (wrb == -1){
        perror("Error while writing message to client");
        return 0;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb == -1){
        perror("Error reading course ID");
        return 0;
    }

    char *position = strstr(rd, "C-");
    char *start = NULL;
    int courseID;
    if(position!=NULL) {
        start = position + strlen("C-");
        courseID = atoi(start);
        
    }
    else{
        write(cfd,"Wrong course id ^",18);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    } 


    coursefd = open("course", O_RDONLY);
    if (coursefd == -1)
    {
        bzero(wr, sizeof(wr));
        strcpy(wr, "Course file doesn't exists ^");
        wrb = write(cfd, wr, strlen(wr));
        if (wrb ==-1)
        {
            perror("Error while writing message");
            return 0;
        }
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }


    coursefd = open("course",O_RDONLY);

    off_t offset = lseek(coursefd, (courseID-1) * sizeof(struct Course), SEEK_SET);
    if (errno == EINVAL)
    {
        bzero(wr, sizeof(wr));
        strcpy(wr, "Course id doesn't exists $");
        wrb = write(cfd, wr, strlen(wr));
        if (wrb == -1)
        {
            perror("Error while writing message to client");
            return 0;
        }
        return 0;
    }
    else if (offset == -1)
    {
        perror("Error while seeking");
        return false;
    }
    lock.l_start = offset;

    int status = fcntl(coursefd, F_SETLKW, &lock);
    if (status == -1)
    {
        perror("Error while obtaining read lock");
        return false;
    }

    rdb = read(coursefd, &getcourse, sizeof(struct Course));
    if (rdb == -1)
    {
        perror("Error reading course");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(coursefd, F_SETLK, &lock);

    bzero(wr, sizeof(wr));
    if(strcmp(getcourse.facultyid,loggedInFaculty.login_id)!=0){
        write(cfd,"This course is not offered by you ^",strlen("This course is not offered by you ^"));
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }

    else if(strcmp(getcourse.status,"notactive")==0){
        write(cfd,"No course found ^",18);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }

    sprintf(wr, "********* Course Details *********  \n\tName: %s\n\tNo of Seats: %d\n\tCredits : %d\n\tNo of available seats: %d\n\tCourse-id: %s", getcourse.name, getcourse.no_of_seats,getcourse.credits,getcourse.no_of_available_seats,getcourse.courseid);
    strcat(wr, "^");

    wrb = write(cfd, wr, strlen(wr));
    if (wrb == -1)
    {
        perror("Error writing");
        return 0;
    }
    rdb = read(cfd,rd,sizeof(rd));
    return true;
}

int logout(int cfd){
    ssize_t rdb, wrb;             
    char rd[1000], wr[1000]; 
    write(cfd,"You have been logged out successfully ^",strlen("You have been logged out successfully ^"));
    close(cfd);
    return 0;
}



