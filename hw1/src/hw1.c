#include <stdlib.h>

#include "debug.h"
#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the content of three frames of audio data and
 * two annotation fields have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 1 if validation succeeds and 0 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variables "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    global_options = 0x0;
    if(argc <= 1)
        return 0;

    char *helpFlagPointer = "-h";
    char *speedUpPointer = "-u";
    char *slowDownPointer = "-d";
    char *cryptPointer = "-c";

    char *flag1 = *(argv + 1);
    int valid;

    if(compareFlags(flag1, helpFlagPointer)){
        global_options = 0x1UL << 63;
        printf("%lx\n", global_options);
        return 1;
    }
    else if(compareFlags(flag1, speedUpPointer)){
        valid = validSpeedUsage(argc, argv); //index of factor
        if(valid){
            global_options = 0x1UL << 62;
            if(containsPreserve(argc, argv))
                preserveRecord();
            if(valid > 1){
                factorRecord(*(argv + valid));
            }
            return 1;
        }else return 0;

    }
    else if(compareFlags(flag1, slowDownPointer)){
        valid = validSpeedUsage(argc, argv); //index of factor
        if(valid){
            global_options = 0x1UL << 61;
            if(containsPreserve(argc, argv))
                preserveRecord();
            if(valid > 1){
                factorRecord(*(argv + valid));
            }
            return 1;
        }else return 0;
    }
    else if(compareFlags(flag1, cryptPointer)){
        valid = validCryptUsage(argc, argv); //index of factor
        if(valid){
            global_options = 0x1UL << 60;
            if(containsPreserve(argc, argv))
                preserveRecord();
            if(valid > 1){
                keyRecord(*(argv + valid));
            }
            return 1;
        }else return 0;
    }
    else
        return 0;

    return 0;
}

int compareFlags(char *flag1, char *flag2){

    if(*flag1 == *flag2)
        if(*(flag1 + 1) == *(flag2 + 1))
            if(*(flag1 + 2) == *(flag2 + 2))
                return 1;

    return 0;
}

int sizeOfString(char *string){
    int size = 0;
    while(*string != '\0'){
        size++;
        string++;
    }

    return size;
}

/*
 * @brief Validates if the preserve flag is present.
 * @details This function will validate if the arguments contain a preserve flag.
 * To be used after the speed up flag, slow down flag OR crypt flag is present.
 * Upon successful return, global variable "global_options" will be modified,
 * where it will be accessible elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 */
int containsPreserve(int argc, char **argv){
    char *perservePointer = "-p";

    for (int i = argc - 1; i > 0; i--)
    {
        char *flag2 = *(argv + i);
        if(compareFlags(flag2, perservePointer))
            return 1;
    }

    return 0;
}

/*
 * @brief Validates if the key flag is present and a valid key is present.
 * @details This function will validate if the arguments contain a key
 * flag and valid key (at most 8 digits of alphanumericals) as the next arguement.
 * Upon successful return, global variable "global_options" will be modified,
 * where it will be accessible elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return the index of the key if validation succeeds and 0 if validation fails.
 */
int containsValidKey(int argc, char **argv){
    char *keyPointer = "-k";

    for (int i = argc - 1; i > 0; i--)
    {
        char *flag2 = *(argv + i);
        if(compareFlags(flag2, keyPointer)){
            if(i < argc - 1){
                if(validKeyHelper(*(argv + i + 1)))
                    return i + 1;
                else return 0;

            }
            else return 0;
        }
    }

    return 0;
}

int validKeyHelper(char *key){

    int digits = sizeOfString(key);
    if(digits > 8)
        return 0;

    for (int i = 0; i < digits; i++)
    {
        char *digit = (key + i);
        //printf("%c\n", *digit);

        if(!((*digit >= 'a' && *digit <= 'f') ||
             (*digit >= 'A' && *digit <= 'F') ||
             (*digit >= '0' && *digit <= '9')))
                return 0;
    }

    return 1;
}

/*
 * @brief Validates if the factor flag is present and a valid factor is present.
 * @details This function will validate if the arguments contain a factor
 * flag and valid factor (decimal integer from [1, 1024]) as the next arguement.
 * Upon successful return, global variable "global_options" will be modified,
 * where it will be accessible elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return the index of the factor if validation succeeds and 0 if validation fails.
 */

int containsValidFactor(int argc, char **argv){
    char *factorPointer = "-f";

    for (int i = argc - 1; i > 0; i--)
    {
        char *flag2 = *(argv + i);
        if(compareFlags(flag2, factorPointer)){
            if(i < argc - 1){
                if(validFactorHelper(*(argv + i + 1)))
                    return i + 1;
                else return 0;

            }
            else return 0;
        }
    }

    return 0;
}

int validFactorHelper(char *factor){
    int f = atoi(factor);
    if(f > 0 && f <= 1024)
        return 1;
    return 0;
}

/*
 * @brief Validates argument combinations for the speed up and slow down flags.
 * @details This function will validate if the arguments is valid for the
 * speed up and slow down flags.
 * To be used after the speed up flag OR slow down flag is present.
 * Upon successful return, global variable "global_options" will be modified,
 * where it will be accessible elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds without a factor, index of factor if factor is present and 0 if validation fails.
 */

int validSpeedUsage(int argc, char **argv){
    //valid # of arguments (bin/audible -u|-d): 2, 3(p), 4(f factor), 5(f factor p)
    if(argc > 5 || argc < 2)
        return 0;

    int valid = 1;
    if(argc == 3 || argc == 5)
        valid = containsPreserve(argc, argv);
    if(argc == 4 || argc == 5)
        if(valid)
            valid = containsValidFactor(argc, argv);

    return valid;
}

/*
 * @brief Validates argument combinations for the crypt flag.
 * @details This function will validate if the arguments is valid for the crypt flag.
 * To be used after the crypt flag is present.
 * Upon successful return, global variable "global_options" will be modified,
 * where it will be accessible elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds without a key, index of key if key is present and 0 if validation fails.
 */

int validCryptUsage(int argc, char **argv){
    //valid # of arguments (bin/audible -c): 2, 3(p), 4(k key), 5(k key p)
    if(argc > 5 || argc < 2)
        return 0;

    int valid = 1;
    if(argc == 3 || argc == 5)
        valid = containsPreserve(argc, argv);
    if(argc == 4 || argc == 5)
        if(valid)
            valid = containsValidKey(argc, argv);

    return valid;
}

int preserveRecord(){
    global_options |= 0x1UL << 59;
    return 0;
}

int factorRecord(char *start){
    unsigned long factor = (atoi(start) - 1);
    global_options |= factor << 48;
    return 0;
}

int keyRecord(char *start){
    unsigned long key = hexStringToInt(start);
    global_options |= key;
    return 0;
}

int hexToInt(char hex){
    if(hex >= '0' && hex <= '9')
        return hex - '0';
    if(hex >= 'a' && hex <= 'f')
        return hex - 'a' + 10;
    if(hex >= 'A' && hex <= 'F')
        return hex - 'A' + 10;

    return -1;
}

unsigned long hexStringToInt(char *string){
    unsigned long hex = 0x0;

    while(*string != '\0'){
        hex |= hexToInt(*string);

        if(*(string + 1) != '\0')
            hex = hex << 4;

        string++;
    }

    return hex;
}
/**
 * @brief  Recodes a Sun audio (.au) format audio stream, reading the stream
 * from standard input and writing the recoded stream to standard output.
 * @details  This function reads a sequence of bytes from the standard
 * input and interprets it as digital audio according to the Sun audio
 * (.au) format.  A selected transformation (determined by the global variable
 * "global_options") is applied to the audio stream and the transformed stream
 * is written to the standard output, again according to Sun audio format.
 *
 * @param  argv  Command-line arguments, for constructing modified annotation.
 * @return 1 if the recoding completed successfully, 0 otherwise.
 */
int recode(char **argv) {
    return 0;
}
