#include <stdlib.h>

#include "debug.h"
#include "hw1.h"

/*
 * @brief Sets the new data size of a header.
 * @details This function will set the data_size
 * of a header based on if the slow down or speed
 * up flag is present.
 *
 * @param *header The pointer to the header.
 * @param frameSize The size of a frame.
 * @param frames The number of frames.
 * @return 1 when data size has changed.
 */


int setNewDataSize(AUDIO_HEADER *header, int frameSize, int frames){
    if(isSlowDown())
        header -> data_size = (frames + (getFactor() - 1) * (frames - 1) )* frameSize;
    else if(isSpeedUp())
        header -> data_size = (frames / getFactor() + 1) * frameSize;

    return 1;
}

/*
 * @brief Gets the number of arguments in argv.
 * @details This function will look at global_options
 * to determine the number of arguments in argv.
 *
 * @return the number of arguments in argv.
 */

int getArgC(){
    int i = 1;
    if(isSpeedUp() || isSlowDown() || isCrypt())
        i++;
    if(isPreserve())
        i++;
    if(getKey() || getFactor())
        i += 2;

    return i;
}

/*
 * @brief Creates a new annotation string for an AUDIO_HEADER.
 * @details This function will write a new annotation by
 * prepending the arguements in argv onto the existing
 * annotation and padding the new annotation so that it
 * is divisible by 8, conforming to the Sun audio format.
 *
 * @param ap The pointer of the array which will contain the
 * new annotation.
 * @param argv The argument strings passed to the program from the CLI.
 * @return the new size of the annotation.
 */

int getNewAnnotation(char *ap, char **argv){
    int k = 0;
    int argc = getArgC();
    for (int i = 0; i < argc; i++)
    {
        k += copyString((ap + k), *(argv + i));
        if(i + 1 < argc){
            *(ap + k) = ' ';
            k++;
        }
    }

    *(ap + k) = '\n';
    k++;

    int size = sizeOfString(input_annotation);

    for (int i = 0; i < size; i++)
        *(ap + k + i) = *(input_annotation + i);

    k+= size;

    for (int i = k%8; i < 8; i++)
    {
        *(ap + k + i) = 0x0;
        k++;
    }

    return k;
}

/*
 * @brief Copies the second string into the first.
 * @details This function will copy the contents of the second
 * string into the first string until a null character is found
 * in the second string.
 *
 * @param p1 pointer to the destination of the copied string.
 * @param p2 pointer to the string that will be copied.
 * @return the size of the copied string without the null char.
 */

int copyString(char *p1, char *p2){
    int k = 0;
    for (int i = 0; *(p2 + i) != '\0' ; i++)
    {
        *(p1 + i) = *(p2 + i);
        k++;
    }
    return k;
}

/*
 * @brief Encrpyts the frames of a Sun audio file.
 * @details This function will read the next frame from stdin
 * and encrypt the sample with myrand32() by appling the XOR
 * opperator to the sample with the psedorandom encrpytion
 * key. It will then write the encrypted frame to stdout.
 *
 * @param header pointer to the header.
 * @param fp pointer to the destination of the frame from stdin.
 * @return 1 if the encrpyted frame is successfully written,
 * 0 otherwise.
 */

int crypt(AUDIO_HEADER *header, int *fp){
    int output_frame2[CHANNELS_MAX * sizeof(int)];

    read_frame(fp, header -> channels, (header -> encoding - 1));
    for (int c = 0; c < header -> channels; c++)
        *(output_frame2 + c) = *(fp + c) ^ myrand32();


    return write_frame(output_frame2, header -> channels, (header -> encoding - 1));
}


/*
 * @brief Speeds up a Sun audio file.
 * @details This function will read the next frame from stdin
 * and only write that frame to stdout if it is a multiple of
 * a specified factor.
 *
 * @param header pointer to the header.
 * @param fp pointer to the destination of the frame from stdin.
 * @param i the current frame.
 * @return 1 if the encrpyted frame is successfully written,
 * 0 otherwise.
 */

int speedUp(AUDIO_HEADER *header, int *fp, int i){
    read_frame(fp, header -> channels, (header -> encoding - 1));

    if(i%getFactor() == 0)
        return write_frame(fp, header -> channels, (header -> encoding - 1));

    return 1;
}


/*
 * @brief Slows down a Sun audio file.
 * @details This function will only read the next frame from stdin
 * if the current frame is 0 and write that frame to stdout.
 * Then it will write N-1 frames inbetween the current and next frame.
 * The values of the inserted samples are obtained by the formula:
 * S+(T-S)*k/N. Where S is the first frame, T is the second frame,
 * k is the current inserted frame, and N is the factor. The values
 * of T are then copied to S.
 *
 *
 * @param header pointer to the header.
 * @param S The pointer for the destination of the first frame.
 * @param T the pointer for the destination of the next frame.
 * @param i the current frame.
 * @param frames the total amount of frames.
 * @return 1 if the inserted frames are successfully written.
 */

int slowDown(AUDIO_HEADER *header, int *S, int *T, int i, int frames){
    int factor = getFactor();
    int output_frame2[CHANNELS_MAX * sizeof(int)];

    if(i == 0)
        read_frame(S, header -> channels, (header -> encoding - 1));

    write_frame(S, header -> channels, (header -> encoding - 1));

    if(i + 1 < frames){
        read_frame(T, header -> channels, (header -> encoding - 1));

        for (int k = 1; k < factor; k++)
        {
            for (int c = 0; c < header -> channels; c++)
                *(output_frame2 + c) = getInterpolation(*(S + c), *(T + c), k, factor);

            write_frame(output_frame2, header -> channels, (header -> encoding - 1));
        }

        copyFrame(S, T, header -> channels);
    }
    return 1;
}

/*
 * @brief copies the values of a frame to another.
 * @details This function will copy the integer values from
 * the second array into the first array.
 *
 * @param S The pointer of the destination of the copied values.
 * @param T The pointer to the values to be copied.
 * @param channels the number of channels.
 * @return 1 if the frames are successfully copied.
 */

int copyFrame(int *S, int *T, int channels){
    for (int c = 0; c < channels; c++)
        *(S + c) = *(T + c);

    return 1;
}

int getInterpolation(signed int S, signed int T, signed int k, signed int N){
    return S + (T - S) * k/N;
}

int isSpeedUp(){
    return global_options >> 62 & 0x1;
}

int isSlowDown(){
    return global_options >> 61 & 0x1;
}

int isCrypt(){
    return global_options >> 60 & 0x1;
}

int isPreserve(){
    return global_options >> 59 & 0x1;
}

int getFactor(){
    return (global_options >> 48 & 0x7ff) + 1;
}

int getKey(){
    return global_options & 0xffffffff;
}