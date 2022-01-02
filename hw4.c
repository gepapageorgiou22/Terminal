#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define MAX_PATH 4096 
#define MAX_NAME 512

//structs
typedef struct info{
    pid_t pid;
    char arguments[MAX_PATH - MAX_NAME];
    char name[MAX_NAME];
    struct info * next;
    volatile sig_atomic_t ended;
} info_T;

//globals
info_T * head = NULL;
pid_t running;
volatile sig_atomic_t markarisma = 0;
volatile sig_atomic_t mark = 0;


void del_node(pid_t delpid);


//signal handlers
static void sighandler(int sig){
    info_T * current;
    for(current = head->next ; current->next != head ; current = current->next){
        kill(current->pid , SIGUSR1);
    }
}

static void termhandler(int sig){
    info_T * current;
    for(current = head->next ; current->next != head ; current = current->next){
        kill(current->pid , SIGTERM);
        waitpid(current->pid , NULL , 0);
    }
    
}

static void childhandler(int sig){
    pid_t ret_pid;
    int wstatus;
    info_T *current;
    
    ret_pid = waitpid(-1 , &wstatus , 0);
    /*//if(WIFEXITED(wstatus) != 0){
        markarisma = ret_pid;
        mark = 1;
        write(STDOUT_FILENO , "Child terminated normally.\n" , 27);
    //}*/
    for(current=head->next; current!=head; current=current->next){
        if(current->pid==ret_pid){
            current->ended=1;
        }
    }
}

//functions
void init_list(){
    head = (info_T *)malloc(sizeof(info_T));
    if(head == NULL){
        perror("In init list malloc");
        exit (1);
    }
    
    head->next = head;
    head->pid = 0;
}

//pros8iki stoixeiwn enws neou programmatos
info_T * add_node(pid_t newpid , const char * argPtr , const char * progname){
    info_T *current;
    
    current=(info_T *)malloc(sizeof(info_T));
    if(current==NULL){
        fprintf(stderr , "Error in add_list malloc, line %d\n" , __LINE__);
        exit (-1);
    }
    
    current->next=head->next;
    head->next=current;
    
    strcpy(current->name,progname);
    current->pid=newpid;
    strcpy(current->arguments,argPtr);
    current->ended=0;
    
    return(current);
}

//euresi kapoiou programmatos
//find returns the pointer previous to the node we have searched for
info_T * find_node(pid_t findpid){
    info_T * current, *previous;
    printf("In find pid = %d\n", findpid);

    previous=head;

    for(current=head->next; current!=head; current=current->next){

        if(current->pid==findpid){
            return(previous);
        }
        previous=previous->next;
    }

   return(NULL);
   
}

//prints list
void print_list(){
    info_T *current;
    
    current=head->next;
    
    while(current != head){
        if(current->ended==1){
            current=current->next; 
            continue;
        }
        printf("pid: %d, name: (%s%s)" , current->pid , current->name , current->arguments);
        if(current->pid == running)
            printf("(R)");
        putchar('\n');

        current=current->next; 
    }
}

//diagrafi stoixeiwn enos programmatos
void del_node(pid_t delpid){
    info_T * current;
    
    current = find_node(delpid);
    if(current != NULL){
        current->next = current->next->next;
    }
}

//counts length of arguments
int arg_counter(char input[MAX_NAME]){
    int counter;
    char temp[MAX_NAME];
    char * i;
    
    counter = 0;
    while(strlen(input) != 0){
        i = strstr(input , " ");
        if(i != NULL){
            *i = '\0';
        }
        else{
            counter++;
            return(counter);
        }
               
        strcpy(temp , i+1);
        strcpy(input , temp);
            
        counter++;
    }
    return(counter);
}

//MAIN
int main(int argc, char *argv[]){
    pid_t pid , termpid , sigpid ;
    char string[MAX_PATH] = {'\0'} , name[MAX_NAME] = {'\0'} , input[5] = {'\0'} , * *argvs = NULL , * i;
    char temp[MAX_PATH] = {'\0'} , args[MAX_NAME] = {'\0'} , arguments[MAX_PATH];
    int error = 0, a , counter , times = 0 , kill_return = 0;
    struct sigaction handler1 = {{0}};
    struct sigaction handler2 = {{0}};
    struct sigaction handler3 = {{0}};
    info_T * current , * tofree;
    
    init_list();
    
    //set the signals
    handler1.sa_handler = SIG_IGN;
    handler1.sa_flags = SA_RESTART;
    handler2.sa_handler = SIG_IGN;
    handler2.sa_flags = SA_RESTART;
    handler3.sa_handler = childhandler;
    handler3.sa_flags = SA_RESTART;
    
    error = sigaction(SIGUSR1 , &handler1 , NULL);
    if(error == -1){
        perror("In main sigaction 1");
    }
    error = sigaction(SIGTERM , &handler2 , NULL);
    if(error == -1){
        perror("In main sigaction 2");
    }
    error = sigaction(SIGCHLD , &handler3 , NULL);
    if(error == -1){
        perror("In main sigaction 3");
    }
    
    //creating the menu
    while(1){
        times=0;
        mark = 1;
        markarisma = 0;
        for(a=0 ; a<5 ; a++){
            input[a]='\0';
        }
        scanf(" %4s" , input);
        
        //here is exec
        if(strcmp(input , "exec") == 0){
                
            scanf("%s" , name);
            
            fgets(string , MAX_PATH - MAX_NAME , stdin);
            string[strlen(string) - 1] = '\0';
            
            strcpy(temp , string);
            counter = arg_counter(temp);
            
            for(a=0 ; a < strlen(temp) ; a++){
                temp[a]='\0';
            }
            
            argvs = (char* *)malloc( (counter+1) * sizeof(char*));
            if(argvs == NULL){
                perror("In seperator malloc1");
                exit(-2);
            }
    
            while(strlen(string) != 0){
                i = strstr(string , " ");
                if(i != NULL){
                    *i = '\0';
                }
                else{
                    //for the last case the str contains only the last argument so we do:
                    if(strlen(string) != 0){
                        argvs[times] = (char*)malloc(strlen(args) * sizeof(char));
                        if(argvs[times] == NULL){
                            perror("In main malloc3");
                            exit(-2);
                        }
                        
                        strcat(arguments , string);
                        strcpy(argvs[times] , string);
                        break;
                    }
                }
        
                strcpy(args , string);
                strcpy(temp , i+1);
                
                for(a=0 ; a < strlen(string) ; a++){
                    string[a]='\0';
                }
                strcpy(string , temp);
        
                argvs[times] = (char*)malloc(strlen(args) * sizeof(char));
                if(argvs[times] == NULL){
                    perror("In main malloc4");
                    exit(-2);
                }
                
                strcat(arguments , args);
                strcat(arguments , " , ");
                strcpy(argvs[times] , args);
        
                times++;
            }
            
            argvs[times+1] = NULL;
            
            //set signalactions
            handler2.sa_handler = SIG_DFL;
            sigaction(SIGUSR1 , &handler2 , NULL);
            
            pid = fork();
            if(pid == 0){
                for(counter=0; counter<5; counter++){
                    fprintf(stderr, "%s\n", argvs[counter]);
                }
                error = execvp(name , argvs);
                if(error == -1){
                    printf("Enter a good propgramm.\n");
                    return(-2);
                }
        
                
            }
            else{
                add_node(pid , arguments , name);
                
                handler1.sa_handler = termhandler;
                handler2.sa_handler = sighandler;
                sigaction(SIGTERM , &handler1 , NULL);
                sigaction(SIGUSR1 , &handler2 , NULL);
                
                for(a=0 ; a<counter ; a++){
                    free(argvs[a]);
                }
                free(*argvs);
                *argvs = NULL;
            }
        }
        //here is list
        if(strcmp(input , "list") == 0){
            print_list();
        }
        //here is term
        if(strcmp(input , "term") == 0){
            scanf(" %d" , &termpid);
            
            kill(termpid , SIGTERM);
        }
        //here is sigusr1
        if(strcmp(input , "sig") == 0){
            scanf(" %d", &sigpid);
            
            kill(sigpid , SIGUSR1);
        }
        if(strcmp(input , "quit") == 0){
            
            for(current=head->next; current!=head; current=current->next){
                kill_return=kill( current->pid, SIGKILL);
                if(kill_return==-1){
                  //  fprintf(stderr, "Error occured trying to kill all procceses!\nI will try again\n%s\n", strerror(errno));
                }
                fprintf(stderr, "Just killed the proccess with pid: %d\n", (int)current->pid);
            }
            
            return(0);
        }
        
        //trying to Find the node
        if(markarisma != 0 && mark == 1){
            current = find_node((pid_t)markarisma);
            printf("head = %p , curr = %p , pid = %d , mark = %d\n", head , current , current->pid , markarisma);
            if(current != NULL){
                tofree = current->next;
                current->next = current->next->next;
                free(tofree);
            }
        }
    }
        
    return(0);
}
