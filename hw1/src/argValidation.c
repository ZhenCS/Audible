#include <stdlib.h>

#include "debug.h"
#include "hw1.h"

/*
 * @brief Compares the values of two flags.
 * @details This function will take the pointer of two char flags and compare them
 * with each other to see if they are equal. Flags with different lengths will
 * be determined as not equal.
 *
 * @param flag1 First flag to be compared.
 * @param argv Second flag to be compared with the first.
 * @return 1 if both flags are equal and 0 if they are not equal.
 */

int compareFlags(char *flag1, char *flag2){

    if(*flag1 == *flag2)
        if(*(flag1 + 1) == *(flag2 + 1))
            if(*(flag1 + 2) == *(flag2 + 2))
                return 1;

    return 0;
}

/*
 * @brief Gets the length of a string.
 * @details This function will count the number of chars from the provided
 * starting point and end when it reaches a null terminator.
 *
 * @param string The pointer to the start of the string.
 * @return Length of the string
 */

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

int charToInt(char *c){
    int value = 0;
    while(*c != '\0'){
        value += *c - '0';

        if(*(c+1) != '\0')
            value *= 10;

        c++;
    }
    return value;
}

int numbersOnly(char *start){

    for (int i = 0; *(start + i) != '\0'; ++i)
    {
        if(*(start + i) > '9' || *(start + 1) < 0)
            return 0;
    }

    return 1;
}


int validFactorHelper(char *factor){

    if(!numbersOnly(factor))
        return 0;

    int f = charToInt(factor);
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
    unsigned long factor = (charToInt(start) - 1);
    global_options |= factor << 48;
    return 0;
}

int keyRecord(char *start){
    unsigned long key = hexStringToInt(start);
    global_options |= key;
    return 0;
}

/*
 * @brief Converts a hex char into its numerical value.
 * @details This function will convert a hex char into [0-15]
 *
 * @param hex The char to be converted into int.
 * @return The decimal value of the provided hex char, -1 If
 * the provided char is not in the range of hex.
 */

int hexToInt(char hex){
    if(hex >= '0' && hex <= '9')
        return hex - '0';
    if(hex >= 'a' && hex <= 'f')
        return hex - 'a' + 10;
    if(hex >= 'A' && hex <= 'F')
        return hex - 'A' + 10;

    return -1;
}

/*
 * @brief Converts a hex string into its numerical value.
 * @details This function will convert a hex string into
 * a unisgned long that represents the string.
 *
 * @param string The pointer to the string to be converted into unisned long.
 * @return The numerical value of the provided hex string.
 */

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