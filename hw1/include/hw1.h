#ifndef HW_H
#define HW_H

#include "audio.h"
#include "const.h"
#include "myrand.h"

int containsPreserve(int argc, char **argv);
int containsValidKey(int argc, char **argv);
int containsValidFactor(int argc, char **argv);

int compareFlags(char *flag1, char *flag2);
int sizeOfString(char *string);

int validKeyHelper(char *key);
int validFactorHelper(char *factor);

int validSpeedUsage(int argc, char **argv);
int validCryptUsage(int argc, char **argv);

int preserveRecord();
int factorRecord(char *start);
int keyRecord(char *start);

int hexToInt(char hex);
unsigned long hexStringToInt(char *string);

#endif
