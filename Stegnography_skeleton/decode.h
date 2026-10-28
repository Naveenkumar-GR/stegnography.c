
#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "common.h"

typedef struct DecodeInfo
{
    char *src_image_fname;
    FILE *src_image_fptr;

    char output_fname[256];
    FILE *output_fptr;

    char secret_extn[10];
    int secret_extn_size;

    unsigned int secret_file_size;
    int output_name_given;

} DecodeInfo;

/* Validate decoding arguments */
Status read_and_validate_decode_args(char *argv[],
                                     DecodeInfo *decInfo);

/* Open input image */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode one byte */
char decode_byte_from_lsb(char *buffer);

/* Decode a 4-byte integer */
unsigned int decode_size_from_lsb(char *buffer);

/* Decode and check magic string */
Status decode_magic_string(const char *magic_string,
                           DecodeInfo *decInfo);

/* Decode extension length */
Status decode_secret_file_extn_size(int *size,
                                    DecodeInfo *decInfo);

/* Decode extension characters */
Status decode_secret_file_extn(char *extension,
                               DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(unsigned int *size,
                               DecodeInfo *decInfo);

/* Decode secret file contents */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Perform decoding */
Status do_decoding(char *argv[], DecodeInfo *decInfo);

#endif
