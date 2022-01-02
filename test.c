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

//it doesn't work if the counter is declared in main
static volatile int counter;

static void signal_action(int signum){
    int error = 0;
    
    counter = 0;
    error = write(STDOUT_FILENO , "Got SIGUSR1. Reseting counter...\n" , 33);
    if(error == -1){
        perror("write");
        exit(-1);
    }
    if(error < 33){
        fprintf(stderr , "Error: write was not able to write the whole message.\n");
    }
}
    

int main(int argc, char *argv[]){
    
    int error;
    struct sigaction action = {{0}};
    pid_t pid;
    
    //setting the SIGUSR1 to IGNORE if signal comes before we have set everything up
    action.sa_handler = SIG_IGN;
    
    error = sigaction( SIGUSR1 , &action , NULL);
    if(error == -1){
        perror("Sigaction");
        exit(1);
    }
    
    //checking for correct arguments
    if(argc!=5){
        fprintf(stderr, "Wrong number of arguments!\nTry again...\n");
        return(-1);
    }
    
    if(strcmp(argv[1] , "-m") != 0 || strcmp(argv[3] , "-b") != 0){
        fprintf(stderr , "Wrong arguments.\nThe format is: ./test.c -m M -b B\n");
        return(-1);
    }
    
    //if the -b B=0 then we set SIGUSR1 to RESET COUNTER
    if(atoi(argv[4]) == 0){
        action.sa_handler = signal_action;
    
        error = sigaction( SIGUSR1 , &action , NULL);
        if(error == -1){
            perror("Sigaction");
            exit(1);
        }
    }
    
    pid = getpid();
    
    for(counter = 1 ; counter <= atoi(argv[2]) ; counter++){
        
        //if -b B=1 AND counter > N/2 we set SIGUSR1 to RESET COUNTER
        if(atoi(argv[4]) == 1 && counter == (atoi(argv[2]) / 2) ){
            action.sa_handler = signal_action;
    
            error = sigaction( SIGUSR1 , &action , NULL);
            if(error == -1){
                perror("Sigaction");
                exit(1);
            }
        }
        
        printf("[test.c]Pid: %ld\t(%d/%d)\n" , (long)pid , counter , atoi(argv[2]) );
        
        sleep(20);
    }
    
    return(0);
}
