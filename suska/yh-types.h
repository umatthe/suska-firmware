// Diese Definitionen stammen vom Elm-FAT-Filesytem (und werden dort auch benutzt)
// für andere Module könnte speziell die BOOL Definition interessant sein.

/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _YH_TYPES
#include <stdint.h>

/* These types must be 16-bit, 32-bit or larger integer */
typedef int16_t		INT;
typedef uint16_t	UINT;

/* These types must be 8-bit integer */
typedef int8_t		CHAR;
typedef uint8_t 	UCHAR;
typedef uint8_t 	BYTE;

/* These types must be 16-bit integer */
typedef int16_t		SHORT;
typedef uint16_t	USHORT;
typedef uint16_t	WORD;
typedef uint16_t	WCHAR;

/* These types must be 32-bit integer */
typedef int32_t		LONG;
typedef uint32_t	ULONG;
typedef uint32_t	DWORD;

/* Boolean type */
typedef enum { FALSE = 0, TRUE } BOOL;

#define _YH_TYPES
#endif
