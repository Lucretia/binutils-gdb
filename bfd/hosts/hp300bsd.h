#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <sys/file.h>

void *malloc();
void free();

#ifndef O_ACCMODE
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)
#endif

#define SEEK_SET 0
#define SEEK_CUR 1

#include <machine/machparam.h>

#define	HOST_PAGE_SIZE		NBPG
#define	HOST_SEGMENT_SIZE	NBPG	/* Data seg start addr rounds to NBPG */
#define	HOST_MACHINE_ARCH	bfd_arch_m68k
/* #define	HOST_MACHINE_MACHINE	 */

#define	HOST_TEXT_START_ADDR		0
#define	HOST_STACK_END_ADDR		0xfff00000

/* EXACT TYPES */
typedef char int8e_type;
typedef unsigned char uint8e_type;
typedef short int16e_type;
typedef unsigned short uint16e_type;
typedef int int32e_type;
typedef unsigned int uint32e_type;

/* CORRECT SIZE OR GREATER */
typedef char int8_type;
typedef unsigned char uint8_type;
typedef short int16_type;
typedef unsigned short uint16_type;
typedef int int32_type;
typedef unsigned int uint32_type;
/* Macros for the 'type' part of an fopen, freopen or fdopen. 
	<Read|Write>[Update]<Binary file><text file>
 */
#define FOPEN_RB	"r"
#define FOPEN_WB 	"w"
#define FOPEN_AB 	"a"
#define FOPEN_RUB 	"r+"
#define FOPEN_WUB 	"w+"
#define FOPEN_AUB 	"a+"
#define FOPEN_RT	"r"
#define FOPEN_WT 	"w"
#define FOPEN_AT 	"a"
#define FOPEN_RUT 	"r+"
#define FOPEN_WUT 	"w+"
#define FOPEN_AUT 	"a+"
