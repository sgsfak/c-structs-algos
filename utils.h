#pragma once

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define memzero(p,n) memset(p,0,n)

/* 
 * reallocarray, copied from OpenBSD:
 * http://cvsweb.openbsd.org/cgi-bin/cvsweb/src/lib/libc/stdlib/reallocarray.c
 * See also:
 * http://lteo.net/blog/2014/10/28/reallocarray-in-openbsd-integer-overflow-detection-for-free/
 */

/*
 * This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
 * if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
 */
#define MUL_NO_OVERFLOW	((size_t)1 << (sizeof(size_t) * 4))

void *
reallocarray(void *optr, size_t nmemb, size_t size)
{
	if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
	    nmemb > 0 && SIZE_MAX / nmemb < size) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(optr, size * nmemb);
}

#undef MUL_NO_OVERFLOW
