/* 
 * Copyright (c) 2017, Christian Gauger-Cosgrove
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Christian Gauger-Cosgrove.
 * 4. Neither the name of Christian Gauger-Cosgrove nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY CHRISTIAN GAUGER-COSGROVE ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CHRISTIAN GAUGER-COSGROVE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifdef NPENTROPY
#	include <math.h>
#endif

#ifdef VARARG
#	include <stdarg.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rfc3797.h"
#include "whirlpool.h"

typedef union MEMBER {
	uint16_t u16;
	uint8_t u8[2];
} MEMBER;

static char	K[BUFSIZ];

static uint16_t	longremainder(uint16_t, uint8_t [DIGESTBYTES]);
static int	cmp(const void *, const void *);

char *
setkey(int sources, char **szSource) {
	unsigned int numbers[16];
	int **values;
	int *counts;
	register int i, j;
	char szKey[BUFSIZ], szPart[BUFSIZ], szNumber[BUFSIZ];
	
	if (sources < 1 || sources > 16) {
		return NULL;
	}

	values = (int **) malloc(sources * sizeof(int *));
	counts = (int *) malloc(sources * sizeof(int));
	if (values == NULL || counts == NULL) {
		if (values) free(values);
		if (counts) free(counts);
		memset(K, 0, (size_t) BUFSIZ * sizeof(uint8_t));
		return NULL;
	}
	
	for (i = 0; i < sources; ++i) {
		counts[i] = sscanf(szSource[i],
		    "%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u",
		    &numbers[ 0], &numbers[ 1], &numbers[ 2], &numbers[ 3],
		    &numbers[ 4], &numbers[ 5], &numbers[ 6], &numbers[ 7],
		    &numbers[ 8], &numbers[ 9], &numbers[10], &numbers[11],
		    &numbers[12], &numbers[13], &numbers[14], &numbers[15]);
		values[i] = (int *) malloc(counts[i] * sizeof(int));
		memcpy(values[i], numbers, counts[i] * sizeof(int));
		qsort(values[i], counts[i], sizeof(int), cmp);
	}

	strcpy(szKey, "");
	for (i = 0; i < sources; ++i) {
		strcpy(szPart, "");
		for (j = 0; j < counts[i]; ++j) {
			sprintf(szNumber, "%d.", values[i][j]);
			strncat(szPart, szNumber, strlen(szNumber));
		}
		strcat(szPart, "/");

		strncat(szKey, szPart, strlen(szPart));
	}
	strncpy(K, szKey, strlen(szKey));

	for (i = 0; i < sources; ++i) {
		free(values[i]);
	}
	free(values);
	free(counts);
	return K;
}


int
makeselection(int pool, int number, int sort, int *select) {
	uint16_t *selected;
	uint8_t *value, hash[DIGESTBYTES];
	size_t klen;
	int *output;
	register int i, j, k, remaining;
	MEMBER usel;
	
	/* Double-check inputs */
	if ((pool > (UINT16_MAX + 1)) || (number > UINT16_MAX) ||
	    (number < 1) || ((sort != SORTED) && (sort != RANDOM))) {
		return 1;
	}

	klen = strlen(K);

	selected = (uint16_t *) malloc(pool * sizeof(uint16_t));
	output = (int *) malloc(number * sizeof(int));
	value = (uint8_t *) malloc(2 * sizeof(uint16_t) + klen * sizeof(char));
	if (selected == NULL || output == NULL || value == NULL) {
		if (selected) free(selected);
		if (output) free(output);
		if (value) free(value);
		return 1;
	}

	for (i = 0; i < pool; ++i) {
		selected[i] = i + 1;
	}

	memset(value, 0, 2 * sizeof(uint16_t) + klen * sizeof(char));
	memcpy(value + 2, K, klen * sizeof(uint8_t));
	for (usel.u16 = 0, remaining = pool; usel.u16 < number;
	    ++usel.u16, --remaining) {
		for (j = 0; j < 2; ++j) {
			value[j] = usel.u8[1 - j];
			value[j + klen + 2] = usel.u8[1 - j];
		}
		whirlpool(value, klen + 4, hash);
		k = longremainder(remaining, hash);

		for (j = 0; j < pool; ++j) {
			if (selected[j]) {
				if (--k < 0) {
					output[usel.u16] = selected[j];
					selected[j] = 0;
					break;
				}
			}
		}
	}
	
	if (sort) {
		qsort(output, number, sizeof(int), cmp);
	}
	memcpy(select, output, number * sizeof(int));

	if (selected) free(selected);
	if (output) free(output);
	if (value) free(value);
	return 0;
}

uint16_t
longremainder(uint16_t divisor, uint8_t dividend[DIGESTBYTES]) {
	register int i;
	register uint64_t kruft;

	if (!divisor) {
		return -1;
	} 

	kruft = 0;
	for (i = 0; i < DIGESTBYTES; ++i) {
		kruft = (kruft << 8) + dividend[i];
		kruft %= divisor;
	}

	return (uint16_t) kruft;
}

int
cmp(const void *a, const void *b) {
	return ( *(int*)a - *(int*)b );
}

#ifdef NPENTROPY
double
NPentropy(int N, int P) {
	register int i;
	double result;

	if ((N < 1) || (N == P)) {
		return 0.0;
	} else if ((P > (UINT16_MAX + 1)) || (N > P)) {
		return -1.0;
	} else {
		result = 0.0;
		for (i = P; i > (P - N); --i) {
 			result += log(i);
		}
		for (i = N; i > 1; --i) {
			result -= log(i);
		}
		result /= log(2);

		return result;
	}
}
#endif

#ifdef VARARG
char *
vsetkey(int sources, ...) {
	va_list argPool;
	unsigned int numbers[16];
	int **values;
	int *counts;
	char *szLine;
	register int i, j;
	char szKey[BUFSIZ], szPart[BUFSIZ], szNumber[BUFSIZ];

	if (sources < 1 || sources > 16) {
		return NULL;
	}

	values = (int **) malloc(sources * sizeof(int *));
	counts = (int *) malloc(sources * sizeof(int));
	if (values == NULL || counts == NULL) {
		if (values) free(values);
		if (counts) free(counts);
		memset(K, 0, (size_t) BUFSIZ * sizeof(uint8_t));
		return NULL;
	}

	va_start(argPool, sources);
	for (i = 0; i < sources; ++i) {
		szLine = va_arg(argPool, char *);
		counts[i] = sscanf(szLine, "%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u",
		    &numbers[ 0], &numbers[ 1], &numbers[ 2], &numbers[ 3],
		    &numbers[ 4], &numbers[ 5], &numbers[ 6], &numbers[ 7],
		    &numbers[ 8], &numbers[ 9], &numbers[10], &numbers[11],
		    &numbers[12], &numbers[13], &numbers[14], &numbers[15]);
		values[i] = (int *) malloc(counts[i] * sizeof(int));
		memcpy(values[i], numbers, counts[i] * sizeof(int));
		qsort(values[i], counts[i], sizeof(int), cmp);
	}
	va_end(argPool);

	strcpy(szKey, "");
	for (i = 0; i < sources; ++i) {
		strcpy(szPart, "");
		for (j = 0; j < counts[i]; ++j) {
			sprintf(szNumber, "%d.", values[i][j]);
			strncat(szPart, szNumber, strlen(szNumber));
		}
		strcat(szPart, "/");

		strncat(szKey, szPart, strlen(szPart));
	}
	strncpy(K, szKey, strlen(szKey));

	for (i = 0; i < sources; ++i) {
		free(values[i]);
	}
	free(values);
	free(counts);
	return K;
}
#endif
