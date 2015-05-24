//Tony Martinez amshell.c
//CS 4560 Lab 1

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define BUFFER_SIZE 50
#define HISTSIZE 10

//global history array and command count
static char histArr[HISTSIZE][(MAX_LINE)/2 + 1];
static int commandCount = 0;

void printHist();
void addHist(char *args[]);
void setup(char inputBuffer[], char *args[],int *background);
void handle_SIGQUIT();

int main(void)
{
    pid_t pid;
    char inputBuffer[MAX_LINE];      /* buffer to hold the command entered */
    int background;              /* equals 1 if a command is followed by '&' */
    char *args[(MAX_LINE/2)+1];  /* command line (of 80) has max of 40 arguments*/

    //setting up signal handler
    struct sigaction handler;
    handler.sa_handler = handle_SIGQUIT;
    handler.sa_flags = SA_RESTART;
    sigaction(SIGQUIT, &handler, NULL);

    printf("Welcome to amshell. My pid is %d\n", getpid());
    while (1){            /* Program terminates normally inside setup */
       background = 0;
       char *foo;
       //print prompt and run setup function
       printf("amshell[%d]:\n",commandCount + 1);
       setup(inputBuffer,args,&background);       /* get next command */
       
       //add command to history array and increment command count
       addHist(args);
       commandCount++;

       //case when user wants to use yell function
       //goes through user entered string and capitalizes each char
       if(strcmp(args[0],"yell") == 0){
	 int i = 1;
	 while(args[i] != NULL){
	   char *a = args[i];
	   int j = 0;
	   while(a[j] != '\0'){
	     printf("%c",toupper(a[j]));
	     j++;
	   }
	   printf(" ");
	   i++;
	   if(args[i] == NULL) printf("\n");
	 }

       }
       //case where user wishes to exit, prints statistics
       else if(strcmp(args[0],"exit") == 0){
	 char command[50] = "ps -p ";
	 int a = getpid();
	 char b[15];
	 sprintf(b,"%d",a);
	 strcat(command,b);
	 strcat(command," -o pid,ppid,pcpu,pmem,etime,user,command");
	 system(command);

	 exit(0);
       }
       //if args[0] is "r", we know they wish to repeat
       else if(strcmp(args[0], "r") == 0){
	 if(args[1] == NULL){//tells us r is only argument, repeat most recent
	   printf("%s\n",histArr[1]);
	   char temporary[10];
	   char temp2[10];
	   strncpy(temporary,histArr[1],4);
	   temporary[strlen(temporary)] = 0;//take substring of history array
	   if(strcmp(temporary, "yell") == 0){
	     //if it's yell, copy what user wishes to yell and
             //yell same way we did previously
	     strncpy(temp2,histArr[1] + 5, strlen(histArr[1]));
	     int j = 0;
	     while(temp2[j] != '\0'){
	       printf("%c",toupper(temp2[j]));
	       j++;
	     }
	     printf("\n");
             //overrite history array with command we repeated
	     strcpy(histArr[0],histArr[1]);
	   }
	   else{//otherwise simply run command
	   system(histArr[1]);
	   strcpy(histArr[0],histArr[1]);//overrite history array with most recent
	   }
	 }
	 else {//case where user wishes to repeat certain command
	   int i = atoi(args[1]);
	   char temporary[10];
	   char temp2[10];
	   int length = commandCount - i;
	   printf("%s\n",histArr[length]);
	   strncpy(temporary,histArr[length],4);
	   temporary[strlen(temporary)] = 0;//take substring of history array
	   //to see if it's "yell"
	   
	   if(strcmp(temporary, "yell") == 0){
	     //if it's yell, copy what user wishes to yell and
             //yell same way we did previously
	     strncpy(temp2,histArr[length] + 5, strlen(histArr[length]));
	     int j = 0;
	     while(temp2[j] != '\0'){
	       printf("%c",toupper(temp2[j]));
	       j++;
	    }
	     printf("\n");
             //overrite history array with command we repeated
	     strcpy(histArr[0],histArr[length]);
	   }
	   else{
             //if we aren't repeating yell, simply run command again
	     system(histArr[commandCount - i]);
	     strcpy(histArr[0],histArr[length]);//overrite history array
	   }
	 }
       }
       else{//otherwise run command using fork method
	 pid = fork();
	 if (pid < 0){//fork failure
	   fprintf(stderr, "Fork Failed");
	   return 1;
	 }
	 else if (pid == 0){//child process
	   execvp(args[0],args);//runs command
	 }
	 else{//parent
	   int status;
	   //check if we are running in background
	   if (background == 1) foo = "TRUE"; else foo = "FALSE";
	   printf("[Child pid = %d, background = %s]\n",pid,foo);
	   if(background == 0){
	     waitpid(pid, &status, 0);//function to wait for child
	     printf("Child process complete\n");
	   }
	 }
       }
    }
}






void addHist(char *args[]){
       char temp[10] = "";
       //move everything in history array up one
       int i;
       for(i = 9; i > 0; i--){
	 strcpy(histArr[i],histArr[i-1]);
       }
       //copy new command into temp array
       int j;
       for(j = 0; j < 20; j++){
	 if(args[j] != NULL){
	   strcat(temp,args[j]);
	   strcat(temp," ");
	 }
	 else break;
       }
       //copy temp array into most recent, histArr[0]
       strcpy(histArr[0],temp);
       strcpy(temp,"");
}

/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 */

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
   
    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE); 

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */
    if (length < 0){
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    /* examine every character in the inputBuffer */
    for (i=0;i<length;i++) {
        switch (inputBuffer[i]){
          case ' ':
          case '\t' :               /* argument separators */
            if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;
          case '\n':                 /* should be the final char examined */
            if (start != -1){
                    args[ct] = &inputBuffer[start];    
                ct++;
            }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
            break;
          default :             /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&'){
                *background  = 1;
                start = -1;
                inputBuffer[i] = '\0';
            }
          }
     }   
     args[ct] = NULL; /* just in case the input line was > 80 */
}

//function that goes through and prints historry array
//as well as the associated command count
void printHist(){
       int k;
       for(k = 0; k < 10; k++){
	 if((commandCount - k) >= 1)
	 printf("%i %s\n",commandCount - k,histArr[k]);
       }
}

//signal handler simply runs printHist()
void handle_SIGQUIT() {
  printHist();
}
