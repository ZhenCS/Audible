#include <string.h>

#include "imprimer.h"
#include "hw4.h"


TYPE *newExtension(char *name){
  if(name == NULL)
    return NULL;

  TYPE *type;
  if((type = (TYPE *)malloc(sizeof(TYPE))) == NULL){
    errorMessage("Unable to add type.");
    return NULL;
  }
  type->type = strdup(name);
  addType(type);
  return type;
}

TYPE *addType(TYPE *type){
  if(type == NULL)
    return NULL;

  static int index = 0;

  if(index >= MAX_TYPES){
    errorMessage("Max types reached. Unable to add type.");
    return NULL;
  }

  typeArray[index] = type;
  index++;
  return type;
}

TYPE *getType(char *type){
  TYPE *type2 = typeArray[0];
  for(int i = 0; i < MAX_TYPES && typeArray[i] != NULL; i++){
    type2 = typeArray[i];
    if(strcmp(type, type2->type) == 0)
      return type2;
  }

  return NULL;
}