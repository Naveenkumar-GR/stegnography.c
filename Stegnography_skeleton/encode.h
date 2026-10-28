#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include "common.h"

typedef struct EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;  // => Store the Src_Image_fname
    FILE *src_image_fptr;   // => File pointer for src_image

    /* Secret File Info */
    char *secret_fname;    // => Store the Secret_fname
    FILE *secret_fptr;     // => File pointer for secret_file
    char secret_extn[7];   // => Store the secret file extn
    int secret_extn_size;  // => Store the secret file extn size
    unsigned long int secret_file_size;  // Store the file size

    /* Stego Image Info */
    char *output_image_fname;   // => Store the Output_img_fname
    FILE *output_image_fptr;    // => File pointer for output_Img

} EncodeInfo;


/* Check operation type */
Opr_type check_operation(char *option);

/* Read and validate Encode args from argv */
Status validate_encode_args(char *argv[], EncodeInfo *encInfo);

/* Perform the encoding */
Status do_encoding(char *argv[], EncodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
Status open_files(EncodeInfo *encInfo);

/* check capacity */
Status check_capacity(EncodeInfo *encInfo);

/* Get image size */
unsigned int get_image_size_for_bmp(FILE *fptr_image);

/* Get secret file size */
unsigned int get_file_size(FILE *fptr);

/* Copy bmp image header */
Status copy_bmp_header(EncodeInfo *encInfo);

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Encode secret file extenstion */
Status encode_secret_file_extn(EncodeInfo *encInfo);

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo);

/* Encode 1 byte into LSB of image data array */
Status encode_1byte_to_lsb(char data, char *buffer_8);

/* Encode 4byte(int) into LSB of image data array */
Status encode_4byte_to_lsb(int data, char *buffer_32);

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(EncodeInfo * encInfo);

#endif
