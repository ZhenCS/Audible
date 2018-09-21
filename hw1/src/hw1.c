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
    AUDIO_HEADER header;
    if(!read_header(&header) || !read_annotation(input_annotation, (&header) -> data_offset - 24))
        return 0;

    int frameSize = ((&header) -> channels * ((&header) -> encoding - 1));
    int frames = (&header) -> data_size/frameSize;

    if(isPreserve()){

        setNewDataSize(&header, frameSize, frames);

        write_header(&header);
        write_annotation(input_annotation, (&header) -> data_offset - 24);
    }
    else {
        int newSize = getNewAnnotation(output_annotation, argv);

        if(newSize > ANNOTATION_MAX || newSize < 0)
            return 0;

        setNewDataSize(&header, frameSize, frames);

        (&header) -> data_offset = 24 + newSize;
        write_header(&header);
        write_annotation(output_annotation, newSize);
    }


    int input_frame2[CHANNELS_MAX * sizeof(int)];
    int next_frame2[CHANNELS_MAX * sizeof(int)];

    mysrand(getKey());

    for (int i = 0; i < frames; i++)
    {

        if(isSpeedUp()){
            speedUp(&header, input_frame2, i);
        }
        else if(isSlowDown()){
            slowDown(&header, input_frame2, next_frame2, i, frames);
        }
        else if(isCrypt()){
            crypt(&header, input_frame2, i);
        }
    }
    return 1;
}

