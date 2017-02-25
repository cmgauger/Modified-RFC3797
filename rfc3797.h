#ifndef _RFC3797_H
#define _RFC3797_H

#define SORTED	1
#define RANDOM	0

char	*setkey(int, char **);
int	 makeselection(int, int, int, int *);

#ifdef NPENTROPY
double	 NPentropy(int, int);
#endif

#ifdef VARARG
char	*vsetkey(int, ...);
#endif

#endif /* _RFC3797_H */
