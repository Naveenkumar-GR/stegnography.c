#include <stdio.h>
#include <string.h>
#include "encode.h"

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

unsigned int get_image_size_for_bmp(FILE *fptr_image)
{
    unsigned int width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: success or failure, on file errors
 */

/* Get secret file size */
unsigned int get_file_size(FILE *fptr)
{
    unsigned int size;

    fseek(fptr, 0, SEEK_END);
    size = (unsigned int)ftell(fptr);
    rewind(fptr);

    return size;
}

/* Check operation type */
Opr_type check_operation(char *option)
{
    if (strcmp(option, "-e") == 0)
        return encode;
    else if (strcmp(option, "-d") == 0)
        return decode;
    else
        return unsupported;
}

/* Validate encoding arguments */
Status validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *dot;
    int length;

    if (argv[2] == NULL || argv[3] == NULL)
        return failure;

    length = strlen(argv[2]);

    if (length < 4 ||
        strcmp(argv[2] + length - 4, ".bmp") != 0)
        return failure;

    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];

    dot = strrchr(argv[3], '.');

    if (dot == NULL)
        return failure;

    length = strlen(dot);

    if (length > 6)
        return failure;

    strcpy(encInfo->secret_extn, dot);
    encInfo->secret_extn_size = length;

    if (argv[4] != NULL)
        encInfo->output_image_fname = argv[4];
    else
        encInfo->output_image_fname = "stego.bmp";

    return success;
}

/* Open files - using your original function */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->src_image_fptr =
        fopen(encInfo->src_image_fname, "rb");

    if (encInfo->src_image_fptr == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n",
                encInfo->src_image_fname);
        return failure;
    }

    encInfo->secret_fptr =
        fopen(encInfo->secret_fname, "rb");

    if (encInfo->secret_fptr == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n",
                encInfo->secret_fname);
        fclose(encInfo->src_image_fptr);
        return failure;
    }

    encInfo->output_image_fptr =
        fopen(encInfo->output_image_fname, "wb");

    if (encInfo->output_image_fptr == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n",
                encInfo->output_image_fname);
        fclose(encInfo->src_image_fptr);
        fclose(encInfo->secret_fptr);
        return failure;
    }

    return success;
}

/* Check image capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    unsigned int image_size;
    unsigned int secret_size;
    unsigned int required_size;

    image_size =
        get_image_size_for_bmp(encInfo->src_image_fptr);

    secret_size =
        get_file_size(encInfo->secret_fptr);

    encInfo->secret_file_size = secret_size;

    required_size =
        (strlen(MAGIC_STRING)
        + 4
        + encInfo->secret_extn_size
        + 4
        + secret_size) * 8;

    if (image_size >= required_size)
        return success;

    return failure;
}

/* Copy BMP header */
Status copy_bmp_header(EncodeInfo *encInfo)
{
    char header[54];

    rewind(encInfo->src_image_fptr);

    if (fread(header, 1, 54,
              encInfo->src_image_fptr) != 54)
        return failure;

    if (fwrite(header, 1, 54,
               encInfo->output_image_fptr) != 54)
        return failure;

    return success;
}

/* Encode one byte using 8 image bytes */
Status encode_1byte_to_lsb(char data, char *buffer_8)
{
    unsigned char value;
    int i;

    value = (unsigned char)data;

    for (i = 0; i < 8; i++)
    {
        buffer_8[i] =
            (buffer_8[i] & 254) |
            ((value >> (7 - i)) & 1);
    }

    return success;
}

/* Encode a 4-byte integer using 32 image bytes */
Status encode_4byte_to_lsb(int data, char *buffer_32)
{
    unsigned int value;
    int i;

    value = (unsigned int)data;

    for (i = 0; i < 32; i++)
    {
        buffer_32[i] =
            (buffer_32[i] & 254) |
            ((value >> (31 - i)) & 1);
    }

    return success;
}

/* Encode magic string */
Status encode_magic_string(const char *magic_string,
                           EncodeInfo *encInfo)
{
    char buffer[8];
    int i;

    for (i = 0; magic_string[i] != '\0'; i++)
    {
        if (fread(buffer, 1, 8,
                  encInfo->src_image_fptr) != 8)
            return failure;

        encode_1byte_to_lsb(magic_string[i], buffer);

        if (fwrite(buffer, 1, 8,
                   encInfo->output_image_fptr) != 8)
            return failure;
    }

    return success;
}

/* Encode secret file extension */
Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    char buffer32[32];
    char buffer8[8];
    int i;

    /* Encode extension length */
    if (fread(buffer32, 1, 32,
              encInfo->src_image_fptr) != 32)
        return failure;

    encode_4byte_to_lsb(encInfo->secret_extn_size,
                        buffer32);

    if (fwrite(buffer32, 1, 32,
               encInfo->output_image_fptr) != 32)
        return failure;

    /* Encode extension characters */
    for (i = 0; i < encInfo->secret_extn_size; i++)
    {
        if (fread(buffer8, 1, 8,
                  encInfo->src_image_fptr) != 8)
            return failure;

        encode_1byte_to_lsb(encInfo->secret_extn[i],
                            buffer8);

        if (fwrite(buffer8, 1, 8,
                   encInfo->output_image_fptr) != 8)
            return failure;
    }

    return success;
}

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char data;
    char buffer32[32];
    char buffer8[8];
    unsigned int size, i;

    size = get_file_size(encInfo->secret_fptr);
    encInfo->secret_file_size = size;

    /* Encode secret file size */
    if (fread(buffer32, 1, 32,
              encInfo->src_image_fptr) != 32)
        return failure;

    encode_4byte_to_lsb((int)size, buffer32);

    if (fwrite(buffer32, 1, 32,
               encInfo->output_image_fptr) != 32)
        return failure;

    rewind(encInfo->secret_fptr);

    /* Encode secret file contents */
    for (i = 0; i < size; i++)
    {
        if (fread(&data, 1, 1,
                  encInfo->secret_fptr) != 1)
            return failure;

        if (fread(buffer8, 1, 8,
                  encInfo->src_image_fptr) != 8)
            return failure;

        encode_1byte_to_lsb(data, buffer8);

        if (fwrite(buffer8, 1, 8,
                   encInfo->output_image_fptr) != 8)
            return failure;
    }

    return success;
}

/* Copy remaining image bytes */
Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    char data;

    while (fread(&data, 1, 1,
                 encInfo->src_image_fptr) == 1)
    {
        if (fwrite(&data, 1, 1,
                   encInfo->output_image_fptr) != 1)
            return failure;
    }

    return success;
}

/* Perform encoding */
Status do_encoding(char *argv[], EncodeInfo *encInfo)
{
    Status result = failure;

    if (validate_encode_args(argv, encInfo) == success)
    {
        if (open_files(encInfo) == success)
        {
            if (check_capacity(encInfo) == success)
            {
                if (copy_bmp_header(encInfo) == success)
                {
                    if (encode_magic_string(MAGIC_STRING,
                                            encInfo) == success)
                    {
                        if (encode_secret_file_extn(encInfo)
                            == success)
                        {
                            if (encode_secret_file_data(encInfo)
                                == success)
                            {
                                if (copy_remaining_img_data(encInfo)
                                    == success)
                                    result = success;
                            }
                        }
                    }
                }
            }

            fclose(encInfo->src_image_fptr);
            fclose(encInfo->secret_fptr);
            fclose(encInfo->output_image_fptr);
        }
    }

    return result;
}