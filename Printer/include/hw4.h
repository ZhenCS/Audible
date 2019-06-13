#ifndef VALIDARGS_H
#define VALIDARGS_H

#define MAX_CHARS 255 //for batch mode
#define MAX_TYPES 31
#define MAX_CONVERSIONS 63


typedef struct conversion {
    char *type;
    char *program;
    char *args;

} CONVERSION;

typedef struct conversionPath {
    CONVERSION *conversion;
    struct conversionPath *next;
} CONVERSIONPATH;


typedef struct type{
    char *type;

    CONVERSION *conversions[MAX_CONVERSIONS];
} TYPE;

typedef struct jobNode {
    JOB *job;
    struct jobNode *next;
} JOBNODE;

JOBNODE job_head;
TYPE *typeArray[MAX_TYPES];
PRINTER *printerArray[MAX_PRINTERS];

void batchMode(int argc, char **argv);
int getFlag(char * flag, int argc, char **argv);
void runCommand(char* input);
int print(char *file, char *printers);

//types.c
TYPE *newExtension(char *name);
TYPE *addType(TYPE *type);
TYPE *getType(char *type);

//conversions.c
void initConversions(CONVERSION **conversions); //for TYPES
CONVERSION *newConversion(char *type1, char *type2, char *program, char *args);
CONVERSION *addConversion(TYPE *type, CONVERSION *conversion);
CONVERSION *getConversion(char *type1, char *type2);

//printers.c
PRINTER *newPrinter(char *name, char *type);
PRINTER *addPrinter(PRINTER *printer);
PRINTER *getPrinter(char *name);
PRINTER_SET getPrinterSet(char *printers);

//jobs.c
JOBNODE *newJob(char *name, char *type, int pgid, PRINTER_SET set);
JOBNODE *newJobNode(JOB *job);
JOBNODE *getJobByID(int id);
JOBNODE *getJobByPID(int pid);
JOBNODE *addJob(JOB *job);
JOBNODE *removeJob(JOB *job);
void removeJobs();


//printers.c
PRINTER *getEligiblePrinter(JOB *job);

//conversions.c
CONVERSIONPATH *getConversionPath(char *fileType, char *printerType, int loops);
CONVERSIONPATH *newConversionPath(CONVERSION *conversion);

char **getPrinterArgs();
char **getConversionArgs(CONVERSION *conversion);
void freeConversion(char **argv);


//messages.c
void printerStatus();
void jobStatus();

void errorMessage(char *message);
void printerMessage(PRINTER *printer);
void jobMessage(JOB *job);

//printers.c
void disablePrinter(char *name);
void enablePrinter(char *name);
void busyPrinter(char *name);
void freePrinter(char *name);

//jobs.c
void pauseJob(JOB *job);
void resumeJob(JOB *job);
void completeJob(JOB *job);
void abortJob(JOB *job);

void runJobs();
int runJob(char *file, JOB *job, char *printername);
int runJobProcess(char *file, JOB *job, PRINTER *printername);


//helpers.c
void usage();
int arrlen(CONVERSION *array[], int max);
char *getFile(JOB *job);
#endif
