#ifndef __MINIZ_H__
#define __MINIZ_H__

#include "doomtype.h"

#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_STDIO
#define MINIZ_HEADER_FILE_ONLY
#define MINIZ_NO_TIME
//#define MINIZ_NO_MALLOC

#if defined(__REMOOD_LITTLE_ENDIAN)
	#define MINIZ_LITTLE_ENDIAN
#endif

#include "z_miniz.c"

#endif /* __MINIZ_H__ */

