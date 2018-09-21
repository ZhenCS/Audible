#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "hw1.h"

int validHeader(AUDIO_HEADER *hp){
    return validMagicNumber(hp) && validDataOffset(hp) && validEncoding(hp) && validChannels(hp);

}

int validMagicNumber(AUDIO_HEADER *hp){
    if(hp->magic_number == AUDIO_MAGIC)
        return 1;

    return 0;
}

int validDataOffset(AUDIO_HEADER *hp){
    if(hp->data_offset % 8 == 0)
        return 1;

    return 0;
}


int validEncoding(AUDIO_HEADER *hp){
    if(hp->encoding <= PCM32_ENCODING && hp->encoding >= PCM8_ENCODING)
        return 1;

    return 0;
}

int validChannels(AUDIO_HEADER *hp){
    if(hp->channels == 1 || hp->channels == 2)
        return 1;

    return 0;
}

int signExtend(int i, int bytes){

    char bit =  (i >> 8 * (bytes - 1)) >> 7 && 0x1;
    if(bit){
        for (int k = 4; k > bytes; k--)
        {
            int extend = 0xff << (k - 1) * 8;

            i |= extend;
            //printf("%s\n", "");
        }
    }

    return i;
}

int getStdinInt(int i){
    //2e73 getchar = e, e in decimal = 15
    //00000010 00001111
    //0x02 0x0f
    int stdin = 0x0;
    while(i > 0){
        if((stdin |= getchar()) == EOF)
            return 0;

        if(i > 1)
            stdin = stdin << 8;

        i--;
    }
    //printf("%i\n", stdin);
    return stdin;
}

int putCharInt(int i){

    for (int k = 1; k <= 4; k++)
    {
        char bit =  i >> (32 - (8 * k)) & 0xff;

        if(putchar(bit) == EOF)
            return 0;
        //printf("%s\n", "");
    }

    return 1;
}

int putByteInt(int i, int bytes){

    for (int k = bytes; k >= 1; k--)
    {
        char bit =  (i >> 8 * (k - 1)) & 0xff;

        if(putchar(bit) == EOF)
            return 0;
        //printf("%s\n", "");
    }

    return 1;
}

char bitsToHex(int bit){//4 bits
    //0010 1110 0111 0011 |||| 0110 1110 0110 0100
    //2e73 6e64
    //2e
    bit = bit & 0xf;

    if(bit >= 0 && bit <= 9)
        return bit + '0';
    if(bit >= 10 && bit <= 15)
        return bit - 10 + 'A';

    return 0;
}