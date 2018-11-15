#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "imprimer.h"
#include "hw4.h"

JOBNODE *newJob(char *name, char *type, int pgid, PRINTER_SET set){
  if(name == NULL || type == NULL || set == 0 || pgid == 0)
    return NULL;

  static int index = 0;

  JOB *job;
  if((job = (JOB *)malloc(sizeof(JOB))) == NULL){
    errorMessage("Unable to create job.");
    return NULL;
  }

  job->jobid = index;
  job->status = QUEUED;
  job->pgid = pgid;
  job->file_name = strdup(name);
  job->file_type = strdup(type);
  job->eligible_printers = set;
  job->chosen_printer = NULL;
  gettimeofday(&job->creation_time, NULL);
  job->change_time = job->creation_time;

  if(job->file_name == NULL || job->file_type == NULL){
    errorMessage("Unable to create job due to memory.");
    return NULL;
  }

  index++;
  return addJob(job);


}

JOBNODE *getJobByPID(int pid){
  JOBNODE *current = job_head.next;
  while(current != NULL){
    if(current->job->pgid == pid)
      return current;

    current = current->next;
  }
  return NULL;
}

JOBNODE *getJobByID(int id){
  JOBNODE *current = job_head.next;
  while(current != NULL){
    if(current->job->jobid == id)
      return current;

    current = current->next;
  }
  return NULL;
}

JOBNODE *addJob(JOB *job){
  JOBNODE *current = &job_head;
  while(current->next != NULL){
    current = current->next;
  }

  current->next = newJobNode(job);
  return current->next;
}

JOBNODE *removeJob(JOB *job){
  JOBNODE *current = &job_head;
  while(current->next != NULL){
    if(current->next->job->jobid == job->jobid){
      JOBNODE *deleted = current->next;
      current->next = current->next->next;
      return deleted;
    }
    current = current->next;
  }

  return NULL;
}

void removeJobs(){
  JOBNODE *current = job_head.next;
  while(current != NULL){
    if(current->job->status == COMPLETED || current->job->status == ABORTED){
      struct timeval currentTime;
      gettimeofday(&currentTime, NULL);
      if(currentTime.tv_sec - current->job->change_time.tv_sec >= 60)
        removeJob(current->job);
    }
    current = current->next;
  }
}

void runJobs(){
  JOBNODE *current = job_head.next;
  while(current != NULL){
    if(current->job->status == QUEUED){
        PRINTER *chosenPrinter = getEligiblePrinter(current->job);
        if(chosenPrinter != NULL)
          runJob(getFile(current->job), current->job, chosenPrinter->name);
    }
    current = current->next;
  }
}

int runJob(char *file, JOB *job, char *printername){
  PRINTER *printer = getPrinter(printername);
  if(job == NULL || printer == NULL || printer->busy)
    return 0;

  job->status = RUNNING;
  job->chosen_printer = printer;
  gettimeofday(&job->change_time, NULL);
  busyPrinter(printername);
  jobMessage(job);
  return runJobProcess(file, job, printer);
}



int runJobProcess(char *file, JOB *job, PRINTER *chosenPrinter){
  pid_t pid;
  if((pid = fork()) == 0){
    setpgid(getpid(), getpid());
    /*int i = 0;
    while(1){
      printf("i = %i\n", i);
      sleep(2);
      i++;
    }*/
    /*int fd;
    if((fd = open("src/text.txt", O_RDONLY)) < 0){
      errorMessage("Unable to open file.");
          exit(EXIT_FAILURE);
    }

    int printerfd = imp_connect_to_printer(chosenPrinter, PRINTER_NORMAL);
    dup2(fd, 0);
    dup2(printerfd, 1);
    char **argv = getPrinterArgs();
    printf("%s\n", "EXECUTING");
    if(execv(argv[0], argv) < 0){
      errorMessage("Unable to run conversion program.");
      exit(EXIT_FAILURE);
    }
    printf("%s\n", "DONE EXECUTING");*/



    exit(EXIT_SUCCESS);
    /*CONVERSIONPATH *path = getConversionPath(job->file_type, chosenPrinter->type);
    //open file
    int fd;
    if((fd = open(file, O_RDONLY)) < 0){
      errorMessage("Unable to open file.");
          exit(EXIT_FAILURE);
    }

    dup2(fd, 0);
    char **argv;
    if(path != NULL){
      int parent2child[2];
      int child2parent[2];
      if(pipe(parent2child) != 0 || pipe(child2parent) != 0){
        errorMessage("Unable to create pipe.");
        exit(EXIT_FAILURE);
      }

      while(path != NULL){
        argv = getConversionArgs(path->conversion);

        pid_t pid2;
        if((pid2 = fork()) == 0){
          dup2(parent2child[0], 0);

          if(path->next == NULL){
            int printerfd = imp_connect_to_printer(chosenPrinter, PRINTER_NORMAL);
            dup2(printerfd, 1);
          }else dup2(child2parent[1], 1);


          if(execv(argv[0], argv) < 0){
            errorMessage("Unable to run conversion program.");
            exit(EXIT_FAILURE);
          }

          if(argv != NULL)
            freeConversion(argv);

          exit(1);
        }else if(pid2 < 0){
          errorMessage("Unable to fork.");
          exit(EXIT_FAILURE);
        }else{
          dup2(fd, parent2child[1]);

          int status;
          waitpid(pid2, &status, 0);
          if(!WIFEXITED(status)){
            errorMessage("conversion program did not terminate normally.");
            exit(EXIT_FAILURE);
          }

          if(path->next != NULL)
            dup2(child2parent[0], fd);
          else
            dup2(child2parent[0], 0);
        }

        path = path->next;
      }


      if((pid = fork()) == 0){
        argv = getPrinterArgs();
        if(execv(argv[0], argv) < 0){
          errorMessage("Unable to connect to printer.");
          exit(EXIT_FAILURE);
        }
      }
      close(parent2child[0]);
      close(parent2child[1]);

      close(child2parent[0]);
      close(child2parent[1]);
    }else{
      if((pid = fork()) == 0){
        argv = getPrinterArgs();
        if(execv(argv[0], argv) < 0){
          errorMessage("Unable to connect to printer.");
          exit(EXIT_FAILURE);
        }
      }
    }

    close(fd);
    exit(EXIT_SUCCESS);*/
  }else if(pid < 0){
    errorMessage("Unable to fork master process.");
    exit(EXIT_FAILURE);
  }else{
    job->pgid = pid;
  }

  return pid;
}

JOBNODE *newJobNode(JOB *job){
  JOBNODE *jobNode;
  if((jobNode = (JOBNODE *)malloc(sizeof(JOBNODE))) == NULL){
    char *error = "Unable to create job.";
    errorMessage(error);
  }
  jobNode->job = job;
  jobNode->next = NULL;

  return jobNode;
}

void pauseJob(JOB *job){
  if(job->status == RUNNING){
    job->status = PAUSED;
    gettimeofday(&job->change_time, NULL);
    jobMessage(job);
  }else{
    errorMessage("This job is not running.");
  }
}

void resumeJob(JOB *job){
  if(job->status == PAUSED){
    job->status = RUNNING;
    gettimeofday(&job->change_time, NULL);
    jobMessage(job);
  }else{
    errorMessage("This job is not paused.");
  }
}

void completeJob(JOB *job){
  if(job->status == RUNNING){
    job->status = COMPLETED;
    gettimeofday(&job->change_time, NULL);
    jobMessage(job);
  }else{
    errorMessage("This job is not running.");
  }
}

void abortJob(JOB *job){
  if(job->status == RUNNING || job->status == PAUSED){
    job->status = ABORTED;
    gettimeofday(&job->change_time, NULL);
    jobMessage(job);
  }else{
    errorMessage("This job is not running.");
  }
}