#include <string.h>

#include "imprimer.h"
#include "hw4.h"

char *getFile(JOB *job){
  char *file = strdup(job->file_name);
  int length = strlen(file) + strlen(job->file_type) + 2;
  file = realloc(file, sizeof(char *) * length);
  strcat(file, ".");
  strcat(file, job->file_type);

  return file;
}

int arrlen(CONVERSION *array[], int max){
  int i = 0;
  for(; i < max; i++){
    if(array[i] == NULL)
      return i;
  }

  return i;
}