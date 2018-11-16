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
  if(name == NULL || type == NULL || set == 0)
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
  sigset_t mask_child, prev_all;
  sigemptyset(&mask_child);
  sigemptyset(&prev_all);
  sigaddset(&mask_child, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask_child, &prev_all);

  pid_t pid;
  if((pid = fork()) == 0){
    setpgid(getpid(), getpid());

    int fd;
    if((fd = open(file, O_RDONLY)) < 0){
      errorMessage("Unable to open file.");
      exit(EXIT_FAILURE);
    }

    int parent2child[2];
    int child2parent[2];
    if(pipe(parent2child) != 0 || pipe(child2parent) != 0){
      errorMessage("Unable to create pipes.");
      exit(EXIT_FAILURE);
    }


    pid_t pid2;
    char **argv;
    CONVERSIONPATH *path = getConversionPath(job->file_type, chosenPrinter->type, MAX_CONVERSIONS);
    CONVERSIONPATH *head = path;
    close(parent2child[1]);

    if(path == NULL)
      dup2(fd, 0);

    while(path != NULL){
      if((pid2 = fork()) == 0){
        sleep(5);
        if(path == head){
          dup2(fd, parent2child[0]);
          close(fd);
        }

        dup2(parent2child[0], 0); //standard input
        dup2(child2parent[0], parent2child[0]);
        dup2(child2parent[1], 1); //standard output

        //printf("FILE HAS BEEN CONVERTED TO %s\n", path->conversion->type);
        close(parent2child[0]);
        close(child2parent[0]);
        close(child2parent[1]);
        argv = getConversionArgs(path->conversion);
        if(execv(argv[0], argv) < 0){
          errorMessage("Unable to run conversion program.");
          exit(EXIT_FAILURE);
        }

        exit(EXIT_FAILURE);
      }else if(pid2 < 0){
        errorMessage("Unable to fork master process.");
        exit(EXIT_FAILURE);
      }
      else{
        int status = -1;
        waitpid(pid2, &status, 0);
        if(WIFEXITED(status)){
        int exitStatus = WEXITSTATUS(status);
          if(exitStatus == EXIT_FAILURE){
            errorMessage("A conversion process have terminated abnormally.");
            exit(EXIT_FAILURE);
          }
        }

        path = path->next;
        if(path == NULL) dup2(child2parent[0], 0);
      }//parent
    }//while
    close(parent2child[0]);
    close(child2parent[0]);
    close(child2parent[1]);

    pid_t pid3;
    if((pid3 = fork()) == 0){
      int printerfd = imp_connect_to_printer(chosenPrinter, PRINTER_NORMAL);
      dup2(printerfd, 1);
      argv = getPrinterArgs();
      if(execv(argv[0], argv) < 0){
        errorMessage("Unable to connect to printer.");
        exit(EXIT_FAILURE);
      }
    }else if(pid3 < 0){
      errorMessage("Unable to fork master process.");
      exit(EXIT_FAILURE);
    }else{
      int status = -1;
      waitpid(pid3, &status, 0);
      if(WIFEXITED(status)){
        int exitStatus = WEXITSTATUS(status);
        if(exitStatus == EXIT_FAILURE){
          errorMessage("Printer has terminated abnormally.");
          exit(EXIT_FAILURE);
        }
      }
    }

    exit(EXIT_SUCCESS);
  }else if(pid < 0){
    errorMessage("Unable to fork master process.");
    exit(EXIT_FAILURE);
  }else{
    job->pgid = pid;
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
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