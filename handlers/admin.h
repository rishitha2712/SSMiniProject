/*
============================================================================
Name : admin.h
Author : Himarishitha Kakunuri
Description : To handle all the admin functionalities.
============================================================================
*/



#include "./login.h"

#include <string.h>
#include <ctype.h>


int admin_handler(int cfd);
int add_student(int cfd);
int get_student_details(int cfd);
int add_faculty(int cfd);
int get_faculty_details(int cfd);
int modify_student_info(int cfd);
int modify_faculty_info(int cfd);
int activate_student(int cfd);
int block_student(int cfd);
int log_out(int cfd);

int admin_handler(int cfd)
{
    if(login_handler(1,cfd,NULL,NULL))
    {
        ssize_t rdb,wrb;            
        char rd[1024], wr[1024]; 
        bzero(wr, sizeof(wr));
        strcpy(wr,"\nLogin Successful \n");
        while (1)
        {
            strcat(wr, "\n......Welcome to Admin Menu......\n1.Add Student\n2.View Student Details\n3.Add Faculty\n4.View Faculty Details\n5.Activate Student\n6.Block Student\n7.Modify Student Details\n8.Modify Faculty  Details\n9.Logout \nEnter your Choice:");
            
            wrb = write(cfd, wr, strlen(wr));
            if (wrb == -1)
            {
                perror("Error while writing Admin Options");
                return 0;
            }
            bzero(wr, sizeof(wr));

            rdb = read(cfd, rd, sizeof(rd));
            if (rdb == -1)
            {
                perror("Error while reading choice");
                return 0;
            }

            int ch = atoi(rd);
            switch (ch)
            {
            case 1:
                add_student(cfd);
                break;
             case 2:
                get_student_details(cfd);
                break;
            case 3: 
                add_faculty(cfd);
                break;
            case 4:
                get_faculty_details(cfd);
                break;
            case 5:
                activate_student(cfd);
                break;
            case 6:
                block_student(cfd);
                break;
            case 7:
                //modify_student_info(cfd);
                break;
            case 8:
                //modify_faculty_info(cfd);
                break;
            case 9:
                log_out(cfd);
                break;            
            default:
                write(cfd,"wrong choice ^",14);
                read(cfd,rd,sizeof(rd));
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

int add_student(int cfd)
{
    ssize_t rdb, wrb;
    char rd[1024], wr[1024];
    struct Student newStudent, previousStudent;

    int sfd = open("student", O_RDONLY);
    if (sfd == -1 && errno == ENOENT)
    {
        newStudent.id = 1;
    }
    else if (sfd == -1)
    {
        perror("Error while opening student file");
        return -1;
    }
    else
    {
        int offset = lseek(sfd, -sizeof(struct Student), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last student record!");
            return 0;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Student), getpid()};
        int lockingStatus = fcntl(sfd, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock on student record!");
            return 0;
        }

        rdb = read(sfd, &previousStudent, sizeof(struct Student));
        if (rdb == -1)
        {
            perror("Error while reading Student record from file!");
            return 0;
        }

        lock.l_type = F_UNLCK;
        fcntl(sfd, F_SETLK, &lock);
        close(sfd);

        newStudent.id = previousStudent.id + 1;
    }

  
    wrb = write(cfd,"Enter Name:",11);
    if (wrb == -1)
    {
        perror("Error writing message to client!");
        return false;
    }

    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb == -1)
    {
        perror("Error reading name response from client!");
        return false;
    }
 
    strcpy(newStudent.name,rd);

    
    wrb = write(cfd,"Enter age:",10);
    if (wrb == -1)
    {
        perror("Error writing message to client!");
        return false;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb == -1)
    {
        perror("Error reading age");
        return false;
    }
    int a=atoi(rd);
    newStudent.age=a;

    
    wrb = write(cfd,"Enter address:",14);
    if (wrb == -1)
    {
        perror("Error writing message to client");
        return false;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb==-1)
    {
        perror("Error reading address");
        return false;
    }
    strcpy(newStudent.address,rd);

    
    wrb = write(cfd,"Enter email:",12);
    if (wrb == -1)
    {
        perror("Error writing message to client");
        return false;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb==-1)
    {
        perror("Error reading email");
        return false;
    }
    
    strcpy(newStudent.email,rd);

     
    strcpy(newStudent.login_id, "MT");
    strcat(newStudent.login_id, "-");
    sprintf(wr, "%d", newStudent.id);
    strcat(newStudent.login_id, wr);

    
    strcpy(newStudent.access,"activated");
    strcpy(newStudent.password, "stu");

    
    sfd = open("student", O_CREAT|O_APPEND|O_WRONLY,S_IRWXU);
     if (sfd == -1)
     {
         perror("Error opening student file!");
         return 0;
     }
    wrb = write(sfd, &newStudent, sizeof(newStudent));
    if (wrb == -1)
    {
        perror("Error while writing Student");
        return 0;
    }
    close(sfd);
    bzero(wr, sizeof(wr));

    
    sprintf(wr, "%s%s\n", "Student added successfully.\nLogin ID for the student is :",newStudent.login_id);
    strcat(wr, "^");
    wrb = write(cfd, wr, strlen(wr));
    if (wrb == -1)
    {
        perror("Error displaying login details");
        return 0;
    }
    rdb = read(cfd,rd,sizeof(rd)); 
    return newStudent.id;
}


int add_faculty(int cfd)
{
    ssize_t rdb, wrb;
    char rd[1024], wr[1024];
    struct Faculty newFaculty, previousFaculty;

    int ffd = open("faculty", O_RDONLY);
    if (ffd == -1 && errno == ENOENT)
    {
        newFaculty.id = 1;
    }
    else if (ffd == -1)
    {
        perror("Error while opening faculty file");
        return -1;
    }
    else{
        int offset = lseek(ffd, -sizeof(struct Faculty), SEEK_END);
        if (offset == -1)
        {
            perror("Error seeking to last faculty record");
            return 0;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Faculty), getpid()};
        int lockingStatus = fcntl(ffd, F_SETLKW, &lock);
        if (lockingStatus == -1)
        {
            perror("Error obtaining read lock");
            return 0;
        }

        rdb = read(ffd, &previousFaculty, sizeof(struct Faculty));
        if (rdb == -1)
        {
            perror("Error while reading faculty record from file!");
            return 0;
        }

        lock.l_type = F_UNLCK;
        fcntl(ffd, F_SETLK, &lock);
        close(ffd);

        newFaculty.id = previousFaculty.id + 1;
    }

  
    wrb = write(cfd, "Enter Name:",11);
    if (wrb == -1)
    {
        perror("Error writing message to client");
        return false;
    }

    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb == -1)
    {
        perror("Error reading name");
        return false;
    }
    
    
    strcpy(newFaculty.name,rd);
    
    
    wrb = write(cfd,"Enter Department:",17);
    if(wrb==-1){
        perror("Error writing message to client");
        return false;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb==-1)
    {
        perror("Error reading department");
        return false;
    }

    strcpy(newFaculty.department,rd);

    wrb = write(cfd, "Enter email:",12);
    if (wrb == -1)
    {
        perror("Error writing message to client");
        return false;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb==-1)
    {
        perror("Error reading email");
        return false;
    }
    strcpy(newFaculty.email,rd);
    
    strcpy(newFaculty.login_id, "F");
    strcat(newFaculty.login_id, "-");
    sprintf(wr, "%d", newFaculty.id);
    strcat(newFaculty.login_id, wr);
    strcpy(newFaculty.password, "fac");


    ffd = open("faculty", O_CREAT|O_APPEND|O_WRONLY,S_IRWXU);
    if (ffd == -1)
     {
         perror("Error while opening file");
         return 0;
     }
    wrb = write(ffd, &newFaculty, sizeof(newFaculty));
    if (wrb == -1)
    {
        perror("Error while writing Faculty record");
        return 0;
    }
    bzero(wr, sizeof(wr));

    sprintf(wr, "%s%s\n","Faculty added successfully.\nFaculty ID is:" ,newFaculty.login_id);
    strcat(wr,"^");
    
    wrb = write(cfd, wr, strlen(wr));
    if (wrb == -1)
    {
        perror("Error displaying login details");
        return 0;
    }
    read(cfd,rd,sizeof(rd)); 
    
    close(ffd);
    return newFaculty.id;
}

int get_student_details(int cfd)
{
    ssize_t rdb, wrb;             
    char rd[1024], wr[1024]; 
    char temp[1024];
    struct Student student;
    int sfd;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Student), getpid()};

    wrb = write(cfd,"Enter student id:" , 17);
    if (wrb == -1){
        perror("Error while writing to client");
        return false;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb == -1){
        perror("Error reading student ID");
        return false;
    }
    
    sfd = open("student", O_RDONLY);
    if (sfd == -1)
    {
        bzero(wr, sizeof(wr));
        strcpy(wr, "Student id doesn't exists ^");
        wrb = write(cfd, wr, strlen(wr));
        if (wrb == -1)
        {
            perror("Error while writing message to client");
            return false;
        }
        rdb = read(cfd,rd,sizeof(rd));  
        return 0;
    }

    char *position = strstr(rd, "MT-");
    char *start = NULL;
    int studentID;
    if(position!=NULL) {
        start = position + strlen("MT-");
        studentID = atoi(start);
        
    }
    
    off_t offset = lseek(sfd, (studentID-1) * sizeof(struct Student), SEEK_SET);
    if (offset == -1)
    {
        perror("Error while seeking to student record ");
        return false;
    }

    lock.l_start = offset;
    int status = fcntl(sfd, F_SETLKW, &lock);
    if (status == -1)
    {
        perror("Error while obtaining read lock on the student");
        return false;
    }

    rdb = read(sfd, &student, sizeof(struct Student));
    if (rdb == -1)
    {
        perror("Error reading student record");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(sfd, F_SETLK, &lock);
    bzero(wr, sizeof(wr));

    sprintf(wr, "********* Student Details *********  \n\tName: %s\n\tAge : %d\n\tEmail : %s\n\tAddress: %s\n\tLogin-id: %s", student.name, student.age,student.email,student.address,student.login_id);
    strcat(wr,"^");

    wrb = write(cfd, wr, strlen(wr));
    if (wrb == -1)
    {
        perror("Error writing student info");
        return false;
    }
    rdb = read(cfd,rd,sizeof(rd)); 

    return true;
}


int get_faculty_details(int cfd)
{
    ssize_t rdb, wrb;             
    char rd[1024], wr[1024]; 
    char temp[1024];
    struct Faculty faculty;
    int ffd;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Faculty), getpid()};

    wrb = write(cfd,"Enter Faculty ID:", strlen("Enter Faculty ID:"));
    if (wrb == -1){
        perror("Error while writing message to client");
        return false;
    }
    bzero(rd, sizeof(rd));
    rdb = read(cfd, rd, sizeof(rd));
    if (rdb == -1){
        perror("Error reading faculty ID");
        return false;
    }
  

    ffd = open("faculty", O_RDONLY);
    if (ffd == -1)
    {
        
        bzero(wr, sizeof(wr));
        strcpy(wr, "Faculty id doesn't exists ^");
        wrb = write(cfd, wr, strlen(wr));
        if (wrb == -1)
        {
            perror("Error while writing message to client");
            return false;
        }
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }
    char *position = strstr(rd, "F-");
    char *start = NULL;
    int facultyID;
    if(position!=NULL) {
        start = position + strlen("F-");
        facultyID = atoi(start);
        
    }
    
    off_t offset = lseek(ffd, (facultyID-1) * sizeof(struct Faculty), SEEK_SET);
    lock.l_start = offset;

    int status = fcntl(ffd, F_SETLKW, &lock);
    if (status == -1)
    {
        perror("Error while obtaining read lock");
        return false;
    }

    rdb = read(ffd, &faculty, sizeof(struct Faculty));
    if (rdb == -1)
    {
        perror("Error reading faculty record");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(ffd, F_SETLK, &lock);
    bzero(wr, sizeof(wr));

    sprintf(wr, "********* Faculty Details *********  \n\tName: %s\n\tDepartment : %s\n\tEmail : %s\n\tLogin-id: %s", faculty.name,faculty.department,faculty.email,faculty.login_id);
    strcat(wr,"^");

    wrb = write(cfd, wr, strlen(wr));
    if (wrb == -1)
    {
        perror("Error writing faculty");
        return 0;
    }
    read(cfd,rd,sizeof(rd)); 
    
    return 1;
}



int block_student(int cfd){
    ssize_t rdb, wrb; 
    int status,sfd;
    char rd[1024], wr[1024],temp[1024];
    struct Student student;
    struct flock lock = {F_WRLCK, SEEK_SET, 0, sizeof(struct Student), getpid()};
    wrb = write(cfd,"Enter student id to block:",strlen("Enter student id to block:"));
    if(wrb==-1){
        perror("Error writing to client");
        return 0;
    }

    rdb = read(cfd,rd,sizeof(rd));
    if(rdb==-1){
        perror("Error reading student id");
        return 0;
    }

    char *position = strstr(rd, "MT-");
    char *start = NULL;
    int studentID;

    if(position!=NULL) {
        start = position + strlen("MT-");
        studentID = atoi(start);
    }
    else{
        write(cfd,"Invalid student ID ^",sizeof("Invalid student ID ^"));
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }

    
    sfd = open("student",O_RDONLY);
    int offset = lseek(sfd,(studentID-1)*sizeof(struct Student),SEEK_SET);
    if(offset == -1){
        perror("Error");
    }
    lock.l_type = F_RDLCK;
    lock.l_start = offset;
    status = fcntl(sfd, F_SETLKW, &lock);
    if(status == -1){
        perror("Error while obtaining read lock");
        return 0;
    }

    rdb = read(sfd, &student, sizeof(struct Student));
    if(rdb == -1){
        perror("Error while reading student record from student file");
        return 0;
    }
    else if(rdb==0){
        write(cfd,"Wrong student ID ^",18);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }
    lock.l_type = F_UNLCK;
    status = fcntl(sfd, F_SETLK, &lock);   
    close(sfd);
    
    if(strcmp(student.access,"blocked")==0){
       write(cfd,"Already blocked ^",17);
       rdb = read(cfd,rd,sizeof(rd));
       return 0;
    } 
    
    
    strcpy(student.access,"blocked");
   
    sfd = open("student",O_WRONLY);
    if(sfd == -1){
        perror("Error while opening student file");
        return 0;
    }
    offset = lseek(sfd, (studentID-1) * sizeof(struct Student), SEEK_SET);
    if(offset == -1){
        perror("Error while seeking to student record");
        return 0;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    status = fcntl(sfd, F_SETLKW, &lock);
    if(status == -1){
        perror("Error while obtaining write lock");
        return 0;
    }
    
    wrb = write(sfd, &student, sizeof(struct Student));
    if(wrb == -1){
        perror("Error while writing into student file");
        return 0;            
    }

    lock.l_type = F_UNLCK;
    fcntl(sfd, F_SETLKW, &lock);
    close(sfd);

    
    wrb = write(cfd, "Student blocked successfully ^",30);
    if(wrb == -1){
        perror("Error while writing student blocking");
        return 0;            
    }
    rdb = read(cfd,rd,sizeof(rd));
    return 1; 
}

int activate_student(int cfd){
    ssize_t rdb, wrb;             
    int status,sfd;
    char rd[1024], wr[10240]; 
    char temp[1024];
    struct Student student;
    struct flock lock = {F_WRLCK, SEEK_SET, 0, sizeof(struct Student), getpid()};
    
    wrb = write(cfd,"Enter student id:",strlen("Enter student id:"));
    if(wrb==-1){
        perror("Error while writing");
        return 0;
    }

    rdb = read(cfd,rd,sizeof(rd));
    if(rdb==-1){
        perror("Error reading student id");
        return 0;
    }

    
    char *position = strstr(rd, "MT-");
    char *start = NULL;
    int studentID;

    if(position!=NULL) {
        start = position + strlen("MT-");
        studentID = atoi(start);
    }
    else{
        write(cfd,"Wrong student ID ^",18);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }

    sfd = open("student",O_RDONLY);
    int offset = lseek(sfd,(studentID-1)*sizeof(struct Student),SEEK_SET);
    if(offset == -1){
        perror("Error while seeking");
        return 0;
    }
    lock.l_type = F_RDLCK;
    lock.l_start = offset;
    status = fcntl(sfd, F_SETLKW, &lock);
    if(status == -1){
        perror("Error while obtaining read lock");
        return 0;
    }

    rdb = read(sfd, &student, sizeof(struct Student));
    if(rdb == -1){
        perror("Error while reading student record");
        return 0;
    }
    else if(rdb==0){
        write(cfd,"Wrong student ID ^",18);
        rdb = read(cfd,rd,sizeof(rd));
        return 0;
    }
    lock.l_type = F_UNLCK;
    status = fcntl(sfd, F_SETLK, &lock);   
    close(sfd);

    if(strcmp(student.access,"activated")==0){
       write(cfd,"Already active ^",17);
       rdb = read(cfd,rd,sizeof(rd));
       return 0;
    } 
    
    strcpy(student.access,"activated");
  
    
    sfd = open("student",O_WRONLY);
    if(sfd == -1){
        perror("Error while opening student file");
        return 0;
    }
    offset = lseek(sfd, (studentID-1) * sizeof(struct Student), SEEK_SET);
    if(offset == -1){
        perror("Error while seeking");
        return 0;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    status = fcntl(sfd, F_SETLKW, &lock);
    if(status == -1){
        perror("Error while obtaining write lock");
        return 0;
    }
    
    wrb = write(sfd, &student, sizeof(struct Student));
    if(wrb == -1){
        perror("Error while updating");
        return 0;            
    }

    lock.l_type = F_UNLCK;
    fcntl(sfd, F_SETLKW, &lock);
    close(sfd);

    wrb = write(cfd, "Student activated successfully ^",32);
    if(wrb == -1){
        perror("Error");
        return 0;            
    }
    rdb = read(cfd,rd,sizeof(rd));  
    return 1; 
}


int log_out(int cfd){
    ssize_t rdb, wrb;             
    char rd[1024], wr[1024]; 
    write(cfd,"You have been logged out successfully ^",sizeof("""You have been logged out successfully ^"));
    write(cfd,"^",1);
    close(cfd);
    return 0;

}



