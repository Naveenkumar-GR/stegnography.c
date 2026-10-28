#include <stdio.h>
#include "encode.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    EncodeInfo encodeInfo;
    DecodeInfo decodeInfo;

    if (argc == 1)
    {
        printf("Error: Please provide arguments\n");
        return 1;
    }

    Opr_type opr = check_operation(argv[1]);

    if (opr == encode)
    {
        if (argc < 4 || argc > 5)
        {
            printf("Usage: ./a.out -e source.bmp secret.txt [output.bmp]\n");
            return 1;
        }

        if (do_encoding(argv, &encodeInfo) == success)
            printf("Encoding successful\n");
        else
            printf("Encoding failed\n");
    }
    else if (opr == decode)
    {
        if (argc < 3 || argc > 4)
        {
            printf("Usage: ./a.out -d stego.bmp [output.txt]\n");
            return 1;
        }

        if (do_decoding(argv, &decodeInfo) == success)
            printf("Decoding successful\n");
        else
            printf("Decoding failed\n");
    }
    else
    {
        printf("Invalid operation. Use -e or -d\n");
        return 1;
    }

    return 0;
}