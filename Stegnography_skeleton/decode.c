#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"

/* Validate decoding arguments */
Status read_and_validate_decode_args(char *argv[],
                                     DecodeInfo *decInfo)
{
    int length;

    if (argv[2] == NULL)
        return failure;

    length = strlen(argv[2]);

    if (length < 4 ||
        strcmp(argv[2] + length - 4, ".bmp") != 0)
        return failure;

    decInfo->src_image_fname = argv[2];
    decInfo->src_image_fptr = NULL;
    decInfo->output_fptr = NULL;

    if (argv[3] != NULL)
    {
        snprintf(decInfo->output_fname,
                 sizeof(decInfo->output_fname),
                 "%s", argv[3]);

        decInfo->output_name_given = 1;
    }
    else
    {
        decInfo->output_name_given = 0;
        decInfo->output_fname[0] = '\0';
    }

    return success;
}

/* Open input image */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->src_image_fptr =
        fopen(decInfo->src_image_fname, "rb");

    if (decInfo->src_image_fptr == NULL)
    {
        perror("fopen");
        return failure;
    }

    return success;
}

/* Decode one byte from 8 image bytes */
char decode_byte_from_lsb(char *buffer)
{
    unsigned char value = 0;
    int i;

    for (i = 0; i < 8; i++)
    {
        value = (value << 1) |
                ((unsigned char)buffer[i] & 1U);
    }

    return (char)value;
}

/* Decode a 4-byte integer from 32 image bytes */
unsigned int decode_size_from_lsb(char *buffer)
{
    unsigned int value = 0;
    int i;

    for (i = 0; i < 32; i++)
    {
        value = (value << 1) |
                ((unsigned char)buffer[i] & 1U);
    }

    return value;
}

/* Decode and verify magic string */
Status decode_magic_string(const char *magic_string,
                           DecodeInfo *decInfo)
{
    char buffer[8];
    char decoded;
    int i;

    for (i = 0; magic_string[i] != '\0'; i++)
    {
        if (fread(buffer, 1, 8,
                  decInfo->src_image_fptr) != 8)
            return failure;

        decoded = decode_byte_from_lsb(buffer);

        if (decoded != magic_string[i])
            return failure;
    }

    return success;
}

/* Decode extension length */
Status decode_secret_file_extn_size(int *size,
                                    DecodeInfo *decInfo)
{
    char buffer[32];
    unsigned int value;

    if (fread(buffer, 1, 32,
              decInfo->src_image_fptr) != 32)
        return failure;

    value = decode_size_from_lsb(buffer);

    if (value == 0 || value >= sizeof(decInfo->secret_extn))
        return failure;

    *size = (int)value;
    decInfo->secret_extn_size = *size;

    return success;
}

/* Decode secret file extension */
Status decode_secret_file_extn(char *extension,
                               DecodeInfo *decInfo)
{
    char buffer[8];
    int i;

    for (i = 0; i < decInfo->secret_extn_size; i++)
    {
        if (fread(buffer, 1, 8,
                  decInfo->src_image_fptr) != 8)
            return failure;

        extension[i] = decode_byte_from_lsb(buffer);
    }

    extension[decInfo->secret_extn_size] = '\0';

    return success;
}

/* Decode secret file size */
Status decode_secret_file_size(unsigned int *size,
                               DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32,
              decInfo->src_image_fptr) != 32)
        return failure;

    *size = decode_size_from_lsb(buffer);
    decInfo->secret_file_size = *size;

    return success;
}

/* Decode secret file contents */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char data;
    unsigned int i;

    for (i = 0; i < decInfo->secret_file_size; i++)
    {
        if (fread(buffer, 1, 8,
                  decInfo->src_image_fptr) != 8)
            return failure;

        data = decode_byte_from_lsb(buffer);

        if (fwrite(&data, 1, 1,
                   decInfo->output_fptr) != 1)
            return failure;
    }

    return success;
}

/* Perform decoding */
Status do_decoding(char *argv[], DecodeInfo *decInfo)
{
    Status result = failure;
    int extn_size;

    if (read_and_validate_decode_args(argv, decInfo) != success)
        return failure;

    if (open_decode_files(decInfo) != success)
        return failure;

    /* Skip the 54-byte BMP header */
    if (fseek(decInfo->src_image_fptr, 54, SEEK_SET) == 0)
    {
        if (decode_magic_string(MAGIC_STRING, decInfo) == success)
        {
            if (decode_secret_file_extn_size(&extn_size,
                                             decInfo) == success)
            {
                if (decode_secret_file_extn(decInfo->secret_extn,
                                            decInfo) == success)
                {
                    if (decode_secret_file_size(
                            &decInfo->secret_file_size,
                            decInfo) == success)
                    {
                        /* Choose output filename */
                        if (decInfo->output_name_given == 0)
                        {
                            snprintf(decInfo->output_fname,
                                     sizeof(decInfo->output_fname),
                                     "decoded%s",
                                     decInfo->secret_extn);
                        }

                        decInfo->output_fptr =
                            fopen(decInfo->output_fname, "wb");

                        if (decInfo->output_fptr != NULL)
                        {
                            result =
                                decode_secret_file_data(decInfo);

                            fclose(decInfo->output_fptr);
                            decInfo->output_fptr = NULL;
                        }
                        else
                        {
                            perror("fopen");
                        }
                    }
                }
            }
        }
    }

    fclose(decInfo->src_image_fptr);
    decInfo->src_image_fptr = NULL;

    return result;
}
