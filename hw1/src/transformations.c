#include <stdlib.h>

#include "debug.h"
#include "hw1.h"

int setNewDataSize(AUDIO_HEADER *header, int frameSize, int frames){
    if(isSlowDown())
        header -> data_size = (frames + (getFactor() - 1) * (frames - 1) )* frameSize;
    else if(isSpeedUp())
        header -> data_size = (frames / getFactor() + 1) * frameSize;

    return 1;
}

int getArgC(){
    int i =1;
    if(isSpeedUp() || isSlowDown() || isCrypt())
        i++;
    if(isPreserve())
        i++;
    if(getKey() || getFactor())
        i += 2;

    return i;
}

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
    {
        *(ap + k + i) = *(input_annotation + i);

        if(i + 1 >= size && *(input_annotation + i) != '\0')
            return 0;

    }

    k+= size;

    for (int i = k%8; i < 8; i++)
    {
        *(ap + k + i) = 0x0;
        k++;
    }
    return k;
}

int copyString(char *p1, char *p2){
    int k = 0;
    for (int i = 0; *(p2 + i) != '\0' ; i++)
    {

        *(p1 + i) = *(p2 + i);
        k++;
    }
    //printf("%i\n", k);
    return k;
}

int crypt(AUDIO_HEADER *header, int *fp, int i){
    int output_frame2[CHANNELS_MAX * sizeof(int)];

    read_frame(fp, header -> channels, (header -> encoding - 1));
    for (int c = 0; c < header -> channels; c++)
    {
        *(output_frame2 + c) = *(fp + c) ^ myrand32();
    }

    return write_frame(output_frame2, header -> channels, (header -> encoding - 1));
}

int speedUp(AUDIO_HEADER *header, int *fp, int i){
    read_frame(fp, header -> channels, (header -> encoding - 1));

    if(i%getFactor() == 0){
        return write_frame(fp, header -> channels, (header -> encoding - 1));
    }

    return 1;
}

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
            {
                *(output_frame2 + c) = getInterpolation(*(S + c), *(T + c), k, factor);
            }
            write_frame(output_frame2, header -> channels, (header -> encoding - 1));
        }

        swapFrame(S, T, header -> channels);
    }
    return 1;
}

int swapFrame(int *S, int *T, int channels){
    for (int c = 0; c < channels; c++)
    {
        *(S + c) = *(T + c);
    }

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