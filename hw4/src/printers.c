#include <string.h>
#include <stdio.h>

#include "imprimer.h"
#include "hw4.h"

PRINTER *newPrinter(char *name, char *type){
  if(name == NULL || type == NULL)
    return NULL;

  if(getType(type) == NULL){
    errorMessage("File type is not defined.");
    return NULL;
  }

  static int index = 0;
  if(index >= MAX_PRINTERS){
    errorMessage("Max printers reached. Unable to add printer.");
    return NULL;
  }

  PRINTER *printer;
  if((printer = (PRINTER *)malloc(sizeof(PRINTER))) == NULL){
    errorMessage("Unable to start printer.");
    return NULL;
  }

  printer->id = index;
  printer->name = strdup(name);
  printer->type = strdup(type);
  printer->enabled = 0;
  printer->busy = 0;

  if(printer->name == NULL || printer->type == NULL){
    errorMessage("Unable to create printer due to memory.");
    return NULL;
  }

  index++;
  addPrinter(printer);
  return printer;
}

PRINTER *addPrinter(PRINTER *printer){
  if(printer == NULL)
    return NULL;

  printerArray[printer->id] = printer;
  return printer;
}

PRINTER *getPrinter(char *name){
  PRINTER *printer = NULL;
  for(int i = 0; i < MAX_PRINTERS && printerArray[i] != NULL; i++){
    printer = printerArray[i];
    if(strcmp(name, printer->name) == 0)
      return printer;
  }

  return NULL;
}

PRINTER_SET getPrinterSet(char *printers){
  PRINTER_SET printerSet = 0;

  if(printers == NULL)
    printerSet = ANY_PRINTER;
  else{
    char *printerName = strtok(printers, " ");
    while(printerName != NULL){
      PRINTER *printer = getPrinter(printerName);
      if(printer == NULL){
        char *error = "Printer  is not defined.";
        error = malloc(strlen(error) + strlen(printerName) + 1);
        strcpy(error, "Printer ");
        strcat(error, printerName);
        strcat(error, " is not defined.");
        errorMessage(error);
        return 0;
      }else{
        printerSet |= 1 << printer->id;
      }

      printerName = strtok(NULL, " ");
    }
  }

  return printerSet;
}

void disablePrinter(char *name){
  PRINTER *printer = getPrinter(name);
  if(printer == NULL)
    errorMessage("No printer with that name exists.");
  else if(printer->enabled == 0)
    errorMessage("Printer is already disabled.");
  else {
    printer->enabled = 0;
    printerMessage(printer);
  }
}

void enablePrinter(char *name){
  PRINTER *printer = getPrinter(name);
  if(printer == NULL)
    errorMessage("No printer with that name exists.");
  else if(printer->enabled > 0)
    errorMessage("Printer is already enabled.");
  else {
    printer->enabled = 1;
   printerMessage(printer);
  }
}

void busyPrinter(char *name){
  PRINTER *printer = getPrinter(name);
  if(printer == NULL)
    errorMessage("No printer with that name exists.");
  else {
    printer->busy = 1;
    printerMessage(printer);
  }
}

void freePrinter(char *name){
  PRINTER *printer = getPrinter(name);
  if(printer == NULL)
    errorMessage("No printer with that name exists.");
  else {
    printer->busy = 0;
    printerMessage(printer);
  }
}

PRINTER *getEligiblePrinter(JOB *job){
  TYPE *type = getType(job->file_type);

  if(type == NULL){
    errorMessage("File type of job is not supported.");
    return NULL;
  }

  PRINTER *printer = NULL;
  CONVERSIONPATH *path;
  for(int i = 0; i < MAX_PRINTERS && printerArray[i] != NULL; i++){// find a printer that does not need converting and is not busy
    printer = printerArray[i];
    if(printer->enabled && job->eligible_printers & (0x1 << printer->id)){
      if(printer->busy == 0 && strcmp(printer->type,type->type) == 0){
        break;
      }
    }

    printer = NULL;
  }

  if(printer == NULL){//if no printer is provided from the above for loop, this loop searches for a conversion path
    for(int i = 0; i < MAX_PRINTERS && printerArray[i] != NULL; i++){
      printer = printerArray[i];
      if(printer->enabled && job->eligible_printers & (0x1 << printer->id)){
        path = getConversionPath(type->type, printer->type, MAX_CONVERSIONS);
        if(path != NULL)
          break;
      }

      printer = NULL;
    }
  }

  if(printer == NULL){
    for(int i = 0; i < MAX_PRINTERS && printerArray[i] != NULL; i++){// find a printer that does not need converting
      printer = printerArray[i];
      if(printer->enabled && job->eligible_printers & (0x1 << printer->id)){
        if(strcmp(printer->type,type->type) == 0){
          break;
        }
      }

      printer = NULL;
    }
  }

  return printer;
}