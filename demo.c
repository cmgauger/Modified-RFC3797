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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rfc3797.h"

void	usage(char *);

int main(int argc, char *argv[]) {
	int *resultList;
	char **szSources;
	char *pEnd;
	long n, p, sourceCount;
	register int i, j;
	
	if (argc != 4) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	
	for (i = 1; i < 4; ++i) {
		switch (i) {
			case 1:
				p = strtol(argv[1], &pEnd, 10);
				break;
			case 2:
				n = strtol(argv[2], &pEnd, 10);
				break;
			case 3:
				sourceCount = strtol(argv[3], &pEnd, 10);
				break;
			default:
				fputs("%RFC3797-F-EWTF, something has gone "
				    "terribly, terrible wrong\n", stderr);
				exit(EXIT_FAILURE);
		}
		if ((size_t) (pEnd - argv[i]) != (size_t) strlen(argv[i])) {
			fprintf(stderr, "%%RFC3797-F-EINVAL, Invalid input for "
			    "argument %d; input must be a valid integer\n", i);
			exit(EXIT_FAILURE);
		}
	}
	
	if ((p > (UINT16_MAX + 1)) || (p < 1) ||
	    (n > p) || (n < 1) ||
	    (sourceCount > 16) || (sourceCount < 1)) {
		fputs("%RFC3797-F-ERANGE, Input value out of range\n", stderr);
		exit(EXIT_FAILURE);
	}
	
	resultList = (int *) malloc((size_t) n * sizeof(int));
	szSources = (char **) malloc((size_t) n * sizeof(char *));
	if ((resultList == NULL) || (szSources == NULL)) {
		if (resultList == NULL) {
			fputs("%RFC3797-F-ENOMEM, Cannot allocate memory for "
			    "selection list\n", stderr);
		} else {
			free(resultList);
		}
		
		if (szSources == NULL) {
			fputs("%RFC3797-F-ENOMEM, Cannot allocate memory for "
			    "list of randomness sources\n", stderr);
		} else {
			free(szSources);
		}
		
		exit(EXIT_FAILURE);
	}
	
	for (i = 0; i < sourceCount; ++i) {
		szSources[i] = (char *) malloc((size_t) BUFSIZ * sizeof(char));
		if (szSources[i] == NULL) {
			fprintf(stderr, "%%RFC3797-F-ENOMEM, Cannot allocate "
			    "memory for random source string #%d\n", i + 1);
			
			free(resultList);
			for (j = i - 1; j >= 0; --j) {
				free(szSources[i]);
			}
			free(szSources);
			
			exit(EXIT_FAILURE);
		}
	}
	
	for (i = 0; i < sourceCount; ++i) {
		fprintf(stdout, "%%RFC3797-I-RANDOM, Enter randomness source "
		    "#%d; input must be a SPACE SEPARATED list of no more than "
		    "sixteen values\n", i + 1);
		fflush(stdout);
		fgets(szSources[i], BUFSIZ, stdin);
	}
	
	pEnd = setkey(sourceCount, szSources);
	if (pEnd == NULL) {
		fputs("%RFC3797-E-ENOKEY, No key set\n", stderr);
	} else {
		fprintf(stdout, "%%RFC3797-S-KEYSET, Key set for selection "
		    "algoritm\n-RFC3797-I-KEYVAL, Key value: [%s]\n", pEnd);
	}
	
#ifdef SORTLIST
	if (makeselection(p, n, SORTED, resultList)) {
#else
	if (makeselection(p, n, RANDOM, resultList)) {
#endif
		fputs("%RFC3797-E-NOLIST, Selection failure\n", stderr);
	} else {
		fputs("%RFC3797-S-SELECT, Selection succeeded\n"
		    "-RFC3797-I-LSTVAL, Selected values list:\n", stdout);
		
		for (i = 0; i < n; ++i) {
			fprintf(stdout, "%5d: %5d\n", i + 1, resultList[i]);
		}
	}
	
	free(resultList);
	for (i = 0; i < sourceCount; ++i) {
		free(szSources[i]);
	}
	free(szSources);
	
	exit(EXIT_SUCCESS);
}

void usage(char *szName) {
	fprintf(stderr, "%%RFC3797-F-EUSAGE, Program usage incorrect\n\n"
	    "%s POOL_SIZE SELECTION_SIZE RANDOMNESS_SOURCES\n"
	    "\tPOOL_SIZE: 1 .. %d\n"
	    "\tSELECTION_SIZE: 1 .. POOL_SIZE\n"
	    "\tRANDOMNESS_SOURCES: 1 .. 16\n",
	    szName, UINT16_MAX + 1);
	exit(EXIT_FAILURE);
}
