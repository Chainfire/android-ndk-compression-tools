/* unlzw.c -- decompress files in LZW format.
 * The code in this file is directly derived from the public domain 'compress'
 * written by Spencer Thomas, Joe Orost, James Woods, Jim McKie, Steve Davies,
 * Ken Turkowski, Dave Mack and Peter Jannesen.
 *
 * This is a temporary version which will be rewritten in some future version
 * to accommodate in-memory decompression.
 */

#include <config.h>
#include "tailor.h"

#include <unistd.h>
#include <fcntl.h>

#include "gzip.h"
#include "lzw.h"

typedef	unsigned char	char_type;
typedef          long   code_int;
typedef unsigned long 	count_int;
typedef unsigned short	count_short;
typedef unsigned long 	cmp_code_int;

#define MAXCODE(n)	(1L << (n))

#ifndef	BYTEORDER
#	define	BYTEORDER	0000
#endif

#ifndef	NOALLIGN
#	define	NOALLIGN	0
#endif


union	bytes {
    long  word;
    struct {
#if BYTEORDER == 4321
        char_type	b1;
        char_type	b2;
        char_type	b3;
        char_type	b4;
#else
#if BYTEORDER == 1234
        char_type	b4;
        char_type	b3;
        char_type	b2;
        char_type	b1;
#else
#	undef	BYTEORDER
        int  dummy;
#endif
#endif
    } bytes;
};

#if BYTEORDER == 4321 && NOALLIGN == 1
#  define input(b,o,c,n,m){ \
     (c) = (*(long *)(&(b)[(o)>>3])>>((o)&0x7))&(m); \
     (o) += (n); \
   }
#else
#  define input(b,o,c,n,m){ \
     char_type *p = &(b)[(o)>>3]; \
     (c) = ((((long)(p[0]))|((long)(p[1])<<8)| \
     ((long)(p[2])<<16))>>((o)&0x7))&(m); \
     (o) += (n); \
   }
#endif

#ifndef MAXSEG_64K
   /* DECLARE(ush, tab_prefix, (1<<BITS)); -- prefix code */
#  define tab_prefixof(i) tab_prefix[i]
#  define clear_tab_prefixof()	memzero(tab_prefix, 256);
#else
   /* DECLARE(ush, tab_prefix0, (1<<(BITS-1)); -- prefix for even codes */
   /* DECLARE(ush, tab_prefix1, (1<<(BITS-1)); -- prefix for odd  codes */
   ush *tab_prefix[2];
#  define tab_prefixof(i) tab_prefix[(i)&1][(i)>>1]
#  define clear_tab_prefixof()	\
      memzero(tab_prefix0, 128), \
      memzero(tab_prefix1, 128);
#endif
#define de_stack        ((char_type *)(&d_buf[DIST_BUFSIZE-1]))
#define tab_suffixof(i) tab_suffix[i]

int block_mode = BLOCK_MODE; /* block compress mode -C compatible with 2.0 */

/* ============================================================================
 * Decompress in to out.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.
 * IN assertions: the buffer inbuf contains already the beginning of
 *   the compressed data, from offsets iptr to insize-1 included.
 *   The magic header has already been checked and skipped.
 *   bytes_in and bytes_out have been initialized.
 */
int unlzw(in, out)
    int in, out;    /* input and output file descriptors */
{
    char_type  *stackp;
    code_int   code;
    int        finchar;
    code_int   oldcode;
    code_int   incode;
    long       inbits;
    long       posbits;
    int        outpos;
/*  int        insize; (global) */
    unsigned   bitmask;
    code_int   free_ent;
    code_int   maxcode;
    code_int   maxmaxcode;
    int        n_bits;
    int        rsize;

#ifdef MAXSEG_64K
    tab_prefix[0] = tab_prefix0;
    tab_prefix[1] = tab_prefix1;
#endif
    maxbits = get_byte();
    block_mode = maxbits & BLOCK_MODE;
    if ((maxbits & LZW_RESERVED) != 0) {
        WARN((stderr, "\n%s: %s: warning, unknown flags 0x%x\n",
              program_name, ifname, maxbits & LZW_RESERVED));
    }
    maxbits &= BIT_MASK;
    maxmaxcode = MAXCODE(maxbits);

    if (maxbits > BITS) {
        fprintf(stderr,
                "\n%s: %s: compressed with %d bits, can only handle %d bits\n",
                program_name, ifname, maxbits, BITS);
        exit_code = ERROR;
        return ERROR;
    }
    rsize = insize;
    maxcode = MAXCODE(n_bits = INIT_BITS)-1;
    bitmask = (1<<n_bits)-1;
    oldcode = -1;
    finchar = 0;
    outpos = 0;
    posbits = inptr<<3;

    free_ent = ((block_mode) ? FIRST : 256);

    clear_tab_prefixof(); /* Initialize the first 256 entries in the table. */

    for (code = 255 ; code >= 0 ; --code) {
        tab_suffixof(code) = (char_type)code;
    }
    do {
        int i;
        int  e;
        int  o;

    resetbuf:
        o = posbits >> 3;
        e = o <= insize ? insize - o : 0;

        for (i = 0 ; i < e ; ++i) {
            inbuf[i] = inbuf[i+o];
        }
        insize = e;
        posbits = 0;

        if (insize < INBUF_EXTRA) {
            rsize = read_buffer (in, (char *) inbuf + insize, INBUFSIZ);
            if (rsize == -1) {
                read_error();
            }
            insize += rsize;
            bytes_in += (off_t)rsize;
        }
        inbits = ((rsize != 0) ? ((long)insize - insize%n_bits)<<3 :
                  ((long)insize<<3)-(n_bits-1));

        while (inbits > posbits) {
            if (free_ent > maxcode) {
                posbits = ((posbits-1) +
                           ((n_bits<<3)-(posbits-1+(n_bits<<3))%(n_bits<<3)));
                ++n_bits;
                if (n_bits == maxbits) {
                    maxcode = maxmaxcode;
                } else {
                    maxcode = MAXCODE(n_bits)-1;
                }
                bitmask = (1<<n_bits)-1;
                goto resetbuf;
            }
            input(inbuf,posbits,code,n_bits,bitmask);
            Tracev((stderr, "%ld ", code));

            if (oldcode == -1) {
                if (256 <= code)
                  gzip_error ("corrupt input.");
                outbuf[outpos++] = (char_type)(finchar = (int)(oldcode=code));
                continue;
            }
            if (code == CLEAR && block_mode) {
                clear_tab_prefixof();
                free_ent = FIRST - 1;
                posbits = ((posbits-1) +
                           ((n_bits<<3)-(posbits-1+(n_bits<<3))%(n_bits<<3)));
                maxcode = MAXCODE(n_bits = INIT_BITS)-1;
                bitmask = (1<<n_bits)-1;
                goto resetbuf;
            }
            incode = code;
            stackp = de_stack;

            if (code >= free_ent) { /* Special case for KwKwK string. */
                if (code > free_ent) {
#ifdef DEBUG
                    char_type *p;

                    posbits -= n_bits;
                    p = &inbuf[posbits>>3];
                    fprintf(stderr,
                            "code:%ld free_ent:%ld n_bits:%d insize:%u\n",
                            code, free_ent, n_bits, insize);
                    fprintf(stderr,
                            "posbits:%ld inbuf:%02X %02X %02X %02X %02X\n",
                            posbits, p[-1],p[0],p[1],p[2],p[3]);
#endif
                    if (!test && outpos > 0) {
                        write_buf(out, (char*)outbuf, outpos);
                        bytes_out += (off_t)outpos;
                    }
                    gzip_error (to_stdout
                                ? "corrupt input."
                                : "corrupt input. Use zcat to recover some data.");
                }
                *--stackp = (char_type)finchar;
                code = oldcode;
            }

            while ((cmp_code_int)code >= (cmp_code_int)256) {
                /* Generate output characters in reverse order */
                *--stackp = tab_suffixof(code);
                code = tab_prefixof(code);
            }
            *--stackp =	(char_type)(finchar = tab_suffixof(code));

            /* And put them out in forward order */
            {
                int i;

                if (outpos+(i = (de_stack-stackp)) >= OUTBUFSIZ) {
                    do {
                        if (i > OUTBUFSIZ-outpos) i = OUTBUFSIZ-outpos;

                        if (i > 0) {
                            memcpy(outbuf+outpos, stackp, i);
                            outpos += i;
                        }
                        if (outpos >= OUTBUFSIZ) {
                            if (!test) {
                                write_buf(out, (char*)outbuf, outpos);
                                bytes_out += (off_t)outpos;
                            }
                            outpos = 0;
                        }
                        stackp+= i;
                    } while ((i = (de_stack-stackp)) > 0);
                } else {
                    memcpy(outbuf+outpos, stackp, i);
                    outpos += i;
                }
            }

            if ((code = free_ent) < maxmaxcode) { /* Generate the new entry. */

                tab_prefixof(code) = (unsigned short)oldcode;
                tab_suffixof(code) = (char_type)finchar;
                free_ent = code+1;
            }
            oldcode = incode;	/* Remember previous code.	*/
        }
    } while (rsize != 0);

    if (!test && outpos > 0) {
        write_buf(out, (char*)outbuf, outpos);
        bytes_out += (off_t)outpos;
    }
    return OK;
}
