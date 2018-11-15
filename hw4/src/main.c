#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <readline/readline.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#include "imprimer.h"
#include "hw4.h"

/*
 * "Imprimer" printer spooler.
 */

sigset_t mask_all, prev_all;
void printHandler(int sig){
  if(sig == SIGCHLD){
    int status = -1;
    int pid = waitpid(-1, &status, WNOHANG);
    if(WIFEXITED(status)){
      //printf("child terminated normally %d\n", status);
      JOB* job = getJobByPID(pid)->job;
      completeJob(job);
      freePrinter(job->chosen_printer->name);
    }else if(status == -1){

    }
    else{
      //printf("child terminated abnormally %d\n", status);
      JOB* job = getJobByPID(pid)->job;
      abortJob(job);
      freePrinter(job->chosen_printer->name);
    }
  }
}

/*void forkit(){
  pid_t a;
  a = fork();
  if(a == 0){
    printf("child pid is %i\n", getpid());
    exit(EXIT_SUCCESS);
  }
}*/

int main(int argc, char *argv[])
{
  if(signal(SIGCHLD, printHandler) == SIG_ERR){
    errorMessage("Unable to create signal handler.");
    exit(EXIT_FAILURE);
  }

  char *input;
  while(1){
    if((input = readline("imp> ")) != NULL){

      char *flag1 = strtok(input, " ");
      if(flag1 != NULL){
        if(!strcmp(flag1, "help")){
            usage();
        }
        if(!strcmp(flag1, "type")){
          newExtension(strtok(NULL, " "));
        }
        if(!strcmp(flag1, "printer")){

          char *name = strtok(NULL, " ");
          char *type = strtok(NULL, " ");

          newPrinter(name, type);
        }
        if(!strcmp(flag1, "printers")){
            printerStatus();
        }
        if(!strcmp(flag1, "jobs")){
            removeJobs();
            jobStatus();
        }
        if(!strcmp(flag1, "pause")){
          char *jobID = strtok(NULL, " ");

          if(jobID != NULL){
            int id = atoi(jobID);
            if(id == 0 && strcmp(jobID, "0") != 0){
              errorMessage("Correct Use: pause jobID");
            }else{

              JOB *job = getJobByID(id)->job;
              killpg(job->pgid, SIGTSTP);
              pauseJob(job);
            }
          }
        }
        if(!strcmp(flag1, "resume")){
          char *jobID = strtok(NULL, " ");

          if(jobID != NULL){
            int id = atoi(jobID);
            if(id == 0 && strcmp(jobID, "0") != 0){
              errorMessage("Correct Use: resume jobID");
            }else{
              JOB *job = getJobByID(id)->job;
              killpg(job->pgid, SIGCONT);
              resumeJob(job);
            }
          }
        }
        if(!strcmp(flag1, "cancel")){
          char *jobID = strtok(NULL, " ");

          if(jobID != NULL){
            int id = atoi(jobID);
            if(id == 0 && strcmp(jobID, "0") != 0){
              errorMessage("Correct Use: cancel jobID");
            }else{
              JOB *job = getJobByID(id)->job;
              killpg(job->pgid, SIGTERM);
              //abortJob(job);
            }
          }
        }
        if(!strcmp(flag1, "conversion")){
          char *type1 = strtok(NULL, " ");
          char *type2 = strtok(NULL, " ");
          char *program = strtok(NULL, " ");
          char *args = strtok(NULL, "\n");

          newConversion(type1, type2, program, args);
        }
        if(!strcmp(flag1, "print")){
          char *file = strtok(NULL, " ");
          char *printers = strtok(NULL, "\n");

          print(file, printers);
        }
        if(!strcmp(flag1, "disable")){
            disablePrinter(strtok(NULL, " "));
        }
        if(!strcmp(flag1, "enable")){
            enablePrinter(strtok(NULL, " "));
        }
        if(!strcmp(flag1, "quit")){
            exit(EXIT_SUCCESS);
        }
        if(!strcmp(flag1, "test")){
          char *file = strtok(NULL, " ");
          int pid = print(file, NULL);
          printf("pid recieved from print %i\n", pid);

          sleep(1);
          killpg(pid, SIGSTOP);

        }
        free(input);

        /*sigset_t mask_all, prev_all;
        sigfillset(&mask_all);
        sigemptyset(&prev_all);

        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        printf("FORKING WHICH SOULD NOT WORK %s\n", "!");*/
        runJobs();
        removeJobs();

        //sigprocmask(SIG_UNBLOCK, &mask_all, NULL);
      }
    }
  }
  exit(EXIT_SUCCESS);
}




int print(char *file, char *printers){
  char *name = strtok(strdup(file), ".");
  char *type = strtok(NULL, "\n");

  if(type == NULL || getType(type) == NULL){
    errorMessage("File type is not defined.");
    return 0;
  }

  PRINTER_SET printerSet = getPrinterSet(printers);
  if(printerSet < 0)
    return 0;

  //sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
  JOB *job = newJob(name, type, getpid(), printerSet)->job;
  //sigprocmask(SIG_BLOCK, &prev_all, NULL);

  PRINTER *chosenPrinter = getEligiblePrinter(job);
  if(chosenPrinter != NULL)
    return runJob(file, job, chosenPrinter->name);

  return 0;
}
