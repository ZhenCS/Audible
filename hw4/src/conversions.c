#include <string.h>

#include "imprimer.h"
#include "hw4.h"

CONVERSIONPATH *getConversionPath(char *fileType, char *printerType){
  TYPE *type = getType(fileType);
  CONVERSION *conversion = type->conversions[0];
  //printf("fileType: %s printerType: %s\n", fileType, printerType);
  if(conversion == NULL || !strcmp(fileType, printerType)){
    //printf("fileType %s has 0 conversions \n", fileType);
    return NULL;
  }

  if((conversion = getConversion(fileType, printerType)) != NULL){
    //printf("conversion %s found.\n", conversion->program);
    return newConversionPath(conversion);
  }

  //printf("There is no conversion from %s to %s\n", fileType, printerType);
  for(int i = 0; i < MAX_CONVERSIONS && type->conversions[i] != NULL; i++){
    conversion = type->conversions[i];
    CONVERSIONPATH *path = getConversionPath(conversion->type, printerType);

    if(path != NULL){
      //printf("%s\n", conversion->program);
      return (newConversionPath(conversion)->next = path);
    }
  }

  return NULL;
}

CONVERSIONPATH *newConversionPath(CONVERSION *conversion){
  CONVERSIONPATH *conversionPath;
  if((conversionPath = (CONVERSIONPATH *)malloc(sizeof(CONVERSIONPATH))) == NULL){
    errorMessage("Unable to create conversion path.");
    return NULL;
  }

  conversionPath->conversion = conversion;
  conversionPath->next = NULL;

  return conversionPath;
}


CONVERSION *newConversion(char *type1, char *type2, char *program, char *args){
  if(type1 == NULL || type2 == NULL || program == NULL)
    return NULL;

  if(getType(type1) == NULL || getType(type2) == NULL){
    errorMessage("file types are not defined.");
    return NULL;
  }

  CONVERSION *conversion;
  if((conversion = (CONVERSION *)malloc(sizeof(CONVERSION))) == NULL){
    errorMessage("Unable to create conversion.");
    return NULL;
  }

  if(getConversion(type1, type2)){
    errorMessage("conversion for these files is already defined.");
    return NULL;
  }

  conversion->type = strdup(type2);
  conversion->program = strdup(program);
  if(args != NULL)
    conversion->args = strdup(args);
  else
    conversion->args = NULL;

  if(conversion->type == NULL || conversion->program == NULL){
    errorMessage("Unable to create conversion due to memory.");
    return NULL;
  }

  addConversion(getType(type1), conversion);
  return conversion;
}

CONVERSION *addConversion(TYPE *type, CONVERSION *conversion){
  if(type == NULL || conversion == NULL)
    return NULL;

  int size = arrlen(type->conversions, MAX_CONVERSIONS);

  if(size >= MAX_CONVERSIONS){
    errorMessage("Max conversions reached. Unable to add conversion.");
    return NULL;
  }

  type->conversions[size] = conversion;

  return conversion;
}

CONVERSION *getConversion(char *type1, char *type2){

  TYPE *type = getType(type1);
  if(type1 == NULL || type2 == NULL || type == NULL)
    return NULL;

  CONVERSION *conversion = type->conversions[0];
  for(int i = 0; i < MAX_CONVERSIONS && type->conversions[i] != NULL; i++){
    conversion = type->conversions[i];
    if(!strcmp(type2, conversion->type))
      return conversion;
  }

  return NULL;
}

char **getConversionArgs(CONVERSION *conversion){
  char *args = strdup(conversion->args);
  char **argv = malloc(sizeof(char *));
  argv[0] = strdup(conversion->program);

  if(args == NULL)
    return NULL;

  int length = 1;
  char *token = strtok("args", " ");
  while(token != NULL){
    length++;
    argv = realloc(argv, sizeof(char *) * length);
    argv[length - 1] = token;

    token = strtok(NULL, " ");
  }

  argv = realloc(argv, sizeof(char *) * length + 1);
  argv[length] = NULL;

  return argv;
}

char **getPrinterArgs(){
  char **argv = malloc(sizeof(char *) * 2);
  char *program = strdup("/bin/cat");

  argv[0] = program;
  argv[1] = NULL;
  return argv;
}

void freeConversion(char **argv){
  char *string = argv[0];
  for (int i = 0; argv[i] != NULL; i++){
    string = argv[i];
    free(string);
  }

  free(argv);
}