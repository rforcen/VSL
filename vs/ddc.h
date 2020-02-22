/*============================================================================

	 ddc.h  -  Don Cross,  October 1992.

	 Generic ddclib stuff.

============================================================================*/

#ifndef DDC_DDC_H
#define DDC_DDC_H

// If you add something to DDCRET, please add the appropriate string
// to the function DDCRET_String() in the file 'source\ddcret.cpp'.

enum DDCRET
{
   DDC_SUCCESS,           // The operation succeded
   DDC_FAILURE,           // The operation failed for unspecified reasons
   DDC_OUT_OF_MEMORY,     // Operation failed due to running out of memory
   DDC_FILE_ERROR,        // Operation encountered file I/O error
   DDC_INVALID_CALL,      // Operation was called with invalid parameters
   DDC_USER_ABORT,        // Operation was aborted by the user
   DDC_INVALID_FILE       // File format does not match
};


const char *DDCRET_String ( DDCRET );   // See source\ddcret.cpp


#define  TRUE     1
#define  FALSE    0

typedef int dBOOLEAN;

typedef unsigned char BYTE;

typedef unsigned char        UINT8;
typedef signed   char        INT8;

typedef unsigned short   UINT16;
typedef signed   short   INT16;

typedef unsigned /* long */  int   UINT_32; // must be 4
typedef signed   /*long*/  int   INT_32;

// check sizes
typedef union {
    char   int8_t_incorrect[sizeof(  INT8) == 1 ? 1: -1];
    char  uint8_t_incorrect[sizeof( UINT8) == 1 ? 1: -1];
    char  int16_t_incorrect[sizeof( INT16) == 2 ? 1: -1];
    char uint16_t_incorrect[sizeof(UINT16) == 2 ? 1: -1];
    char  int32_t_incorrect[sizeof( INT_32) == 4 ? 1: -1];
    char uint32_t_incorrect[sizeof(UINT_32) == 4 ? 1: -1];
} _____test_size;


#endif /* __DDC_DDC_H */

/*--- end of file ddc.h ---*/
