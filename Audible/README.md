# AUDIBLE

This program takes in an .au audio file and can speed up or slow down the audio

#How to Use
bin.audible [-h] -u|-d|-c [-f FACTOR] [-k KEY] [-p]
    -h      Help: displays this help menu.
    -d      Slow down: decrease playback speed by inserting interpolated samples

            Optional additional parameters with -u or -d:
                -f      FACTOR is an integer factor for speed up or slow down
                        It must be a decimal number in the range [1, 1024
                -p      Preserve input annotation without modification
    -c      Crypt: encrypt or decrypt (requires -k)

            Required additional parameter with -c:
            -k          KEY is a secret key for encryption or decryption
                        It must be a hexadecimal number with at most 8 digits
            Optional additional parameter with -c:
            -p          Preserve input annotation without modification

[< AUDIO]   AUDIO:  The path for the input audio
[> OUT]     OUT:    The path to the output audio
                    If file does not exist, it will create it. If the file does exist, it will rewrite it.

Example: bin/audible -u -f 2 < rsrc/sample.au > speedup.au

Projects programmed in C for CSE 320 with Professor Gene Stark

