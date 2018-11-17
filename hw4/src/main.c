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
    int pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
    if(WIFEXITED(status)){
      int exitStatus = WEXITSTATUS(status);
      if(exitStatus == EXIT_SUCCESS){
        JOB* job = getJobByPID(pid)->job;
        completeJob(job);
        freePrinter(job->chosen_printer->name);
        runJobs();
      }else if(exitStatus == EXIT_FAILURE){
        JOB* job = getJobByPID(pid)->job;
        abortJob(job);
        freePrinter(job->chosen_printer->name);
        runJobs();
      }
    }else if(WIFSTOPPED(status)){
      JOB* job = getJobByPID(pid)->job;
      pauseJob(job);
    }else if(WIFCONTINUED(status)){
      JOB* job = getJobByPID(pid)->job;
      resumeJob(job);
    }else if(WIFSIGNALED(status)){
      JOB* job = getJobByPID(pid)->job;
      abortJob(job);
      freePrinter(job->chosen_printer->name);
      runJobs();
    }
  }
}

int main(int argc, char *argv[])
{
  if(signal(SIGCHLD, printHandler) == SIG_ERR){
    errorMessage("Unable to create signal handler.");
    exit(EXIT_FAILURE);
  }

  batchMode(argc, argv);

  char *input;
  while(1){
    if((input = readline("imp> ")) != NULL){
        runCommand(input);

        free(input);
        runJobs();
        removeJobs();
    }
  }
  exit(EXIT_SUCCESS);
}

void batchMode(int argc, char **argv){
  if(argc > 1){
    FILE *outFile;
    int oFlag = getFlag("-o", argc, argv);
    if(oFlag >= argc - 1){
      errorMessage("No output file specified");
      exit(EXIT_FAILURE);
    }else if(oFlag > 0){
      char *ofile = argv[oFlag + 1];
      if((outFile = freopen(ofile, "w", stdout)) == NULL){
        errorMessage("Unable to open input file.");
        exit(EXIT_FAILURE);
      }
    }

    int iFlag = getFlag("-i", argc, argv);

    if(iFlag >= argc - 1){
      errorMessage("No input file specified");
      exit(EXIT_FAILURE);
    }else if(iFlag > 0){
      char *file = argv[iFlag + 1];

      FILE *script;
      if((script = fopen(file, "r")) == NULL){
        errorMessage("Unable to open input file.");
        exit(EXIT_FAILURE);
      }


      char line[MAX_CHARS];
      while(fgets(line, MAX_CHARS, script) != NULL){
        char *input = strtok(line, "\n");

        if(input != NULL){
          printf("BATCH: imp> %s\n", line);
          runCommand(line);
        }
      }

      if(fclose(script) == EOF){
        errorMessage("Unable to close input file.");
        exit(EXIT_FAILURE);
      }
    }
  }
}

int getFlag(char * flag, int argc, char **argv){

  for(int i = 0; i < argc; i++)
    if(!strcmp(flag, argv[i]))
      return i;

  return -1;
}

void runCommand(char* input){
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

      PRINTER *printer = newPrinter(name, type);
      if(printer != NULL)
        printerMessage(printer);

    }
    if(!strcmp(flag1, "printers")){
        printerStatus();
    }
    if(!strcmp(flag1, "jobs")){
        jobStatus();
    }
    if(!strcmp(flag1, "pause")){
      char *jobID = strtok(NULL, " ");
      if(jobID != NULL){
        int id = atoi(jobID);
        if(id == 0 && strcmp(jobID, "0") != 0){
          errorMessage("Correct Use: pause jobID");
        }else{
          JOBNODE *jobNode = getJobByID(id);
          if(jobNode != NULL){
            if(jobNode->job->status == RUNNING)
              killpg(jobNode->job->pgid, SIGTSTP);
            else
              errorMessage("Job is not running.");
          }
          else
            errorMessage("No job with that ID.");
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
          JOBNODE *jobNode = getJobByID(id);
          if(jobNode != NULL){
            if(jobNode->job->status == PAUSED)
              killpg(jobNode->job->pgid, SIGCONT);
            else
              errorMessage("Job is not paused.");
          }
          else
            errorMessage("No job with that ID.");
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
          JOBNODE *jobNode = getJobByID(id);
          if(jobNode != NULL){
            if(jobNode->job->status == RUNNING)
              killpg(jobNode->job->pgid, SIGTERM);
            else
              errorMessage("Job is not running.");
          }
          else
            errorMessage("No job with that ID.");
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
          fclose(stdout);

        exit(EXIT_SUCCESS);
    }
  }
}



int print(char *file, char *printers){
  char *name = strtok(strdup(file), ".");
  char *type = strtok(NULL, "\n");

  if(type == NULL || getType(type) == NULL){
    errorMessage("File type is not defined.");
    return 0;
  }

  PRINTER_SET printerSet = getPrinterSet(printers);
  if(printerSet == 0)
    return 0;

  JOB *job = newJob(name, type, 0, printerSet)->job;
  if(job == NULL)
    return 0;

  jobMessage(job);

  PRINTER *chosenPrinter = getEligiblePrinter(job);
  if(chosenPrinter != NULL)
    return runJob(file, job, chosenPrinter->name);

  return 0;
}
