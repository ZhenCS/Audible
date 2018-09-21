#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "hw1.h"

/*
 * @brief Checks if all the elements of a header is valid.
 * @details This function will check if the header contains:
 * the correct magic number, a valid dataoffset divisible by 8,
 * a valid encoding [2,5], and a valid channel [1,2]
 *
 * @param *hp the pointer to the header to validate.
 * @return 0 if any one of the elements are not valid,
 * 1 if all elements are valid.
 */
int validHeader(AUDIO_HEADER *hp){
    return validMagicNumber(hp) && validDataOffset(hp) && validEncoding(hp) && validChannels(hp);

}

/*
 * @brief Validates the magic number of a header.
 * @details This function will compare the magic number
 * in the header with AUDIO_MAGIC to see if they are equal.
 *
 * @param *hp the pointer to the header to validate.
 * @return 1 if the header magic number is equal to AUDIO_MAGIC,
 * 0 otherwise.
 */

int validMagicNumber(AUDIO_HEADER *hp){
    if(hp->magic_number == AUDIO_MAGIC)
        return 1;

    return 0;
}

/*
 * @brief Validates the data offset of a header.
 * @details This function will see if the data offset is
 * divisible by 8 and has a minimum value of the size of
 * an AUDIO_HEADER (24).
 *
 * @param *hp the pointer to the header to validate.
 * @return 1 if the header data offset is divisible by 8 and
 * greater than the size of AUDIO_HEADER, 0 otherwise.
 */

int validDataOffset(AUDIO_HEADER *hp){
    if(hp->data_offset % 8 == 0 && hp->data_offset >= sizeof(hp))
        return 1;

    return 0;
}

/*
 * @brief Validates the encoding of a header.
 * @details This function will checl if the encoding has
 * a value less than or equal to PCM32_ENCODING and
 * greater than or equal to PCM8_ENCODING
 *
 * @param *hp the pointer to the header to validate.
 * @return 1 if the header encoding is in the range of
 * PCM8_ENCODING and PCM32_ENCODING, 0 otherwise.
 */

int validEncoding(AUDIO_HEADER *hp){
    if(hp->encoding <= PCM32_ENCODING && hp->encoding >= PCM8_ENCODING)
        return 1;

    return 0;
}

/*
 * @brief Validates the channels of a header.
 * @details This function will check if the channels
 * is either 1 or 2.
 *
 * @param *hp the pointer to the header to validate.
 * @return 1 if the header channels is 1 or 2,
 * 0 otherwise.
 */

int validChannels(AUDIO_HEADER *hp){
    if(hp->channels == 1 || hp->channels == 2)
        return 1;

    return 0;
}

/*
 * @brief Sign extends a 32 bit integer.
 * @details This function will will check for a leading 1
 * in the byte number specified and if a leading 1 is
 * present, sign extend it to the 32nd bit.
 *
 * @param i The int to be sign extended.
 * @param bytes The leading byte of the integer.
 * @return i sign extended.
 */

int signExtend(int i, int bytes){

    char bit =  (i >> 8 * (bytes - 1)) >> 7 && 0x1;
    if(bit){
        for (int k = 4; k > bytes; k--)
        {
            int extend = 0xff << (k - 1) * 8;
            i |= extend;
        }
    }

    return i;
}

/*
 * @brief Concatenates bytes from stdin into an integer.
 * @details This function will get the specified number
 * of bytes, assumed in big-endian order, and concatenate
 * them into an integer value.
 *
 * @param i The number of bytes to get from stdin.
 * @return an integer value of the bytes from stdin,
 * 0 if stdin reaches EOF.
 */

int getStdinInt(int i){
    int stdin = 0x0;
    while(i > 0){
        if((stdin |= getchar()) == EOF)
            return 0;

        if(i > 1)
            stdin = stdin << 8;
        i--;
    }
    return stdin;
}

/*
 * @brief Writes an integer into stdout.
 * @details This function will write a 32 bit integer
 * into stdout.
 *
 * @param i The integer to write to stdout.
 * @return 1 if the integer is successfully written,
 * 0 if stdout reaches EOF.
 */

int putCharInt(int i){

    for (int k = 1; k <= 4; k++)
    {
        char bit =  i >> (32 - (8 * k)) & 0xff;

        if(putchar(bit) == EOF)
            return 0;
    }

    return 1;
}

/*
 * @brief Writes an integer of n bytes into stdout.
 * @details This function will write n bytes of an
 * integer starting from the least significant byte
 * to the most significant byte into stdout.
 *
 * @param i The integer to write to stdout.
 * @param bytes The number of bytes of i to write to stdout
 * @return 1 if the integer bytes is successfully written,
 * 0 if stdout reaches EOF.
 */

int putByteInt(int i, int bytes){

    for (int k = bytes; k >= 1; k--)
    {
        char bit =  (i >> 8 * (k - 1)) & 0xff;

        if(putchar(bit) == EOF)
            return 0;
    }

    return 1;
}