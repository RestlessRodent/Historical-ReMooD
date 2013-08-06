#ifndef __MINIZ_H__
#define __MINIZ_H__

//#include "doomtype.h"

#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#if defined(__REMOOD_NOMINIZINCLUDE)
	#define MINIZ_HEADER_FILE_ONLY
#endif

#if defined(__REMOOD_LITTLE_ENDIAN)
	#define MINIZ_LITTLE_ENDIAN 1
#endif

#if !defined(__REMOOD_NOMINIZINCLUDE)
	//#include "z_miniz.c"
#endif

#endif /* __MINIZ_H__ */

