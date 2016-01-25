/* add.c   not copyrighted (n) 1993 by Mark Adler */
/* version 1.1   11 Jun 1993 */

/* This filter reverses the effect of the sub filter.  It requires no
   arguments, since sub puts the information necessary for extraction
   in the stream.  See sub.c for what the filtering is and what it's
   good for. */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#define MAGIC1    'S' /* sub data */
#define MAGIC2    26  /* ^Z */
#define MAX_DIST  16384

char a[MAX_DIST];	/* last byte buffer for up to MAX_DIST differences */

int main()
{
  int n;		/* number of differences */
  int i;		/* difference counter */
  int c;		/* byte from input */

  /* check magic word */
  if (getchar() != MAGIC1 || getchar() != MAGIC2)
  {
    fputs("add: input stream not made by sub\n", stderr);
    exit(EXIT_FAILURE);
  }

  /* get number of differences from data */
  if ((n = getchar()) == EOF || (i = getchar()) == EOF) {
    fputs("add: unexpected end of file\n", stderr);
    exit(EXIT_FAILURE);
  }
  n += (i<<8);
  if (n <= 0 || n > MAX_DIST) {
    fprintf(stderr, "add: incorrect distance %d\n", n);
    exit(EXIT_FAILURE);
  }

  /* initialize last byte */
  i = n;
  do {
    a[--i] = 0;
  } while (i);

  /* read differenced data and restore original */
  while ((c = getchar()) != EOF)
  {
    c = (a[i++] += c) & 0xff;	/* restore data, save last byte */
    putchar(c);			/* write original */
    if (i == n)			/* cycle on n differences */
      i = 0;
  }
  exit(EXIT_SUCCESS);
  return 0;			/* avoid warning */
}
