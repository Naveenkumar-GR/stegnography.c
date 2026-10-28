#ifndef COMMON_H
#define COMMON_H

/* Magic string to identify whether stegged or not */
#define MAGIC_STRING "#*"

/* Status will be used in fn. return type */
typedef enum
{
    failure,
    success
} Status;

typedef enum
{
    unsupported,
    encode,
    decode
} Opr_type;

#endif
