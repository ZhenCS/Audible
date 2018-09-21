#ifndef HW_H
#define HW_H

#include "audio.h"
#include "const.h"
#include "myrand.h"

// argValidation.c
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
int numbersOnly(char *string);

//audioHelper.c
int putCharInt(int i);
int putByteInt(int i, int bytes);

int signExtend(int i, int bytes);
int getStdinInt(int i);

int validMagicNumber(AUDIO_HEADER *hp);
int validDataOffset(AUDIO_HEADER *hp);
int validEncoding(AUDIO_HEADER *hp);
int validChannels(AUDIO_HEADER *hp);
int validHeader(AUDIO_HEADER *hp);

//transformations.c
int isSpeedUp();
int isSlowDown();
int isCrypt();
int isPreserve();
int getFactor();
int getKey();

int getArgC();

int getNewAnnotation(char *ap, char **argv);
int copyString(char *p1, char *p2);

int speedUp(AUDIO_HEADER *header, int *fp, int i);
int slowDown(AUDIO_HEADER *header, int *S, int *T, int i, int frames);
int crypt(AUDIO_HEADER *header, int *fp);

int setNewDataSize(AUDIO_HEADER *header, int framesize, int frames);
int copyFrame(int *S, int *T, int channels);
int getInterpolation(signed int S, signed int T, signed int k, signed int N);

#endif
