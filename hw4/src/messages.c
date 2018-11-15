#include <string.h>
#include <sys/time.h>
#include <stdio.h>

#include "imprimer.h"
#include "hw4.h"

#define no_argument 0
#define required_argument 1

static struct option_info {
        char *name;
        int has_arg;
        char *argname;
        char *descr;
} option_table[] = {
 {"quit",                no_argument,        NULL,
  "Cause execution to terminate."},
 {"type",                required_argument,  "type",
  "Declare type to be a file type to be supported by the program."},
 {"printer",             required_argument,  "printer_name type",
  "Declare the existence of a printer which is capable of printing files of type type."},
 {"conversion",          required_argument,  "type1 type2 con_prog",
  "Declare that files of type type1 can be converted into type2 by running con_prog."},
 {"printers",            no_argument,        NULL,
  "Print current status of the declared printers."},
 {"jobs",                no_argument,        NULL,
  "Print current status of the print jobs."},
 {"print",               required_argument, "name [printer1 ...]",
  "Print a job for printing file_name."},
 {"cancel",              required_argument,  "job_number",
  "Cancel an existing job."},
 {"pause",               required_argument,  "job_number",
  "Pause a job that is currently being processed."},
 {"resume",              required_argument,  "job_number",
  "Resume a job that was previously paused."},
 {"disable",             required_argument,  "printer_name",
  "Set the state of a specified printer to 'disabled'."},
 {"enable",              required_argument,  "printer_name",
  "Set the state of a specified printer to 'enabled'."}
};

#define NUM_OPTIONS (sizeof(option_table)/sizeof(option_table[0]))

void usage(){
  struct option_info *opt;

  fprintf(stderr, "Valid options are:\n");
  for(unsigned int i = 0; i < NUM_OPTIONS; i++) {
    opt = &option_table[i];

    char arg[32];
    if(opt->has_arg)
        sprintf(arg, " <%.30s>", opt->argname);
    else
        sprintf(arg, "%.13s", "");
    fprintf(stderr, "\t%-10s%-13s\t\t%s\n",
                 opt->name, arg, opt->descr);
    opt++;
  }
}

void printerStatus(){
  PRINTER *printer = printerArray[0];
  for(int i = 0; i < MAX_PRINTERS && printerArray[i] != NULL; i++){
    printer = printerArray[i];
    printerMessage(printer);
  }
}

void jobStatus(){
  JOBNODE *jobNode = job_head.next;
  while(jobNode != NULL){

    if(jobNode != NULL)
      jobMessage(jobNode->job);
    jobNode = jobNode->next;
  }
}




void errorMessage(char *message){
  int length = strlen(message) + 10;
  char *buffer = malloc(length);
  printf("%s\n", imp_format_error_message(message, buffer, length));
  free(buffer);
}

void printerMessage(PRINTER *printer){
  int length = strlen(printer->name) + strlen(printer->type) + 32;
  char *buffer = malloc(length);
  printf("%s\n", imp_format_printer_status(printer, buffer, length));
  free(buffer);
}

void jobMessage(JOB *job){
  int length = strlen(job->file_name) + strlen(job->file_type) + 128;
  char *buffer = malloc(length);
  printf("%s\n", imp_format_job_status(job, buffer, length));
  free(buffer);
}