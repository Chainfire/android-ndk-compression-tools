/* p_lzo.c -- LZO compression

   This file is part of the lzop file compressor.

   Copyright (C) 1996-2010 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   lzop and the LZO library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzop/
 */


#include "conf.h"


#if defined(WITH_LZO)


/*************************************************************************
//
**************************************************************************/

int lzo_set_method(int m, int level)
{
    int l = level & 0xff;

    if (m != 0 || l < 1 || l > 9)
        return -1;          /* not a LZO method */

#if defined(USE_LZO1X_1_15)
    if (l == 1)
        m = M_LZO1X_1_15;
    else
#endif
#if defined(USE_LZO1X_1)
    if (l <= 6)
    {
        m = M_LZO1X_1;
        l = 5;
    }
#endif
#if defined(USE_LZO1X_999)
    if (l >= 7 && l <= 9)
    {
        m = M_LZO1X_999;
    }
#endif

    if (m == 0)
        return 1;           /* error */

    opt_method = m;
    opt_level = l;
    return 0;
}


int lzo_get_method(header_t *h)
{
/* check method */
    if (h->method == M_LZO1X_1)
    {
        h->method_name = "LZO1X-1";
        if (h->level == 0)
            h->level = 3;
    }
    else if (h->method == M_LZO1X_1_15)
    {
        h->method_name = "LZO1X-1(15)";
        if (h->level == 0)
            h->level = 1;
    }
    else if (h->method == M_LZO1X_999)
    {
        static char s[11+1] = "LZO1X-999  ";
        s[9] = 0;
        if (h->level == 0)
            h->level = 9;
        else if (h->version >= 0x0950 && h->lib_version >= 0x1020)
        {
            s[9] = '/';
            s[10] = (char) (h->level + '0');
        }
        h->method_name = s;
    }
    else
        return -1;      /* not a LZO method */

/* check compression level */
    if (h->level < 1 || h->level > 9)
        return 15;

    return 0;
}


void lzo_init_compress_header(header_t *h)
{
    if (opt_checksum)
    {
        if (opt_crc32)
        {
            h->flags |= F_CRC32_D;
            if (opt_checksum >= 2)
                h->flags |= F_CRC32_C;
        }
        else
        {
            h->flags |= F_ADLER32_D;
            if (opt_checksum >= 2)
                h->flags |= F_ADLER32_C;
        }
    }
}


/*************************************************************************
// memory setup
**************************************************************************/

#if defined(ACC_OS_DOS16) && !defined(ACC_ARCH_I086PM)
#  define BLOCK_SIZE        (128*1024l)
#else
#  define BLOCK_SIZE        (256*1024l)
#endif
#define MAX_BLOCK_SIZE      (64*1024l*1024l)        /* DO NOT CHANGE */

#if defined(USE_LZO1X_999)
#  define WRK_LEN           LZO1X_999_MEM_COMPRESS
#elif defined(USE_LZO1X_1_15)
#  define WRK_LEN           LZO1X_1_15_MEM_COMPRESS
#elif defined(USE_LZO1X_1)
#  define WRK_LEN           LZO1X_1_MEM_COMPRESS
#else
#  error
#endif


#if 1
#  define ALIGN_SIZE    4096
#else
#  define ALIGN_SIZE    1
#endif

/* LZO may expand uncompressible data by a small amount */
#define MAX_COMPRESSED_SIZE(x)  ((x) + (x) / 16 + 64 + 3)


static mblock_t blocks[2];
static mblock_t wrkmem;


static void free_mem(void)
{
    mb_free(&wrkmem);
    mb_free(&blocks[1]);
    mb_free(&blocks[0]);
}


static lzo_bool alloc_mem(lzo_uint32 s1, lzo_uint32 s2, lzo_uint32 w)
{
    lzo_bool r = 1;

    r &= mb_alloc(&blocks[0], s1, ALIGN_SIZE);
    r &= mb_alloc(&blocks[1], s2, ALIGN_SIZE);
    r &= mb_alloc(&wrkmem, w,  ALIGN_SIZE);
    if (!r)
        free_mem();
    return r;
}


/*************************************************************************
// enter / leave
**************************************************************************/

/* maybe make this an option ? */
static const lzo_uint block_size = BLOCK_SIZE;

lzo_bool lzo_enter(const header_t *h)
{
    int r;
    lzo_uint32 wrk_len;

#if defined(WITH_THREADS)
    if (opt_num_threads > 1)
        return lzo_threaded_enter(h);
#endif

    if (h != NULL)
    {
        r = 1;
        if ((h->flags & F_ADLER32_C) && !(h->flags & F_ADLER32_D))
            { r = 0; assert(h->flags & F_ADLER32_D); }
        if ((h->flags & F_CRC32_C) && !(h->flags & F_CRC32_D))
            { r = 0; assert(h->flags & F_CRC32_D); }
        return r;
    }

#if 0
    fprintf(stderr,"%lu %lu %u\n", BLOCK_SIZE, MAX_BLOCK_SIZE, ALIGN_SIZE);
#endif
    assert(block_size <= BLOCK_SIZE);
    assert(block_size <= MAX_BLOCK_SIZE);
    assert(block_size >= 16*1024);

    if (opt_method == M_LZO1X_1)
        wrk_len = LZO1X_1_MEM_COMPRESS;
    else if (opt_method == M_LZO1X_1_15)
        wrk_len = LZO1X_1_15_MEM_COMPRESS;
    else if (opt_method == M_LZO1X_999)
        wrk_len = LZO1X_999_MEM_COMPRESS;
    else
        wrk_len = 0;
    assert(wrk_len <= WRK_LEN);

    if (opt_method == M_LZO1X_999)
    {
        if (opt_checksum < 1)
            opt_checksum = 1;           /* always compute a checksum */
        if (opt_cmd == CMD_COMPRESS)
            opt_optimize = 1;
    }

    switch (opt_cmd)
    {
    case CMD_COMPRESS:
        r = alloc_mem(block_size, MAX_COMPRESSED_SIZE(block_size), wrk_len);
        break;
    case CMD_DECOMPRESS:
    case CMD_TEST:
        r = alloc_mem(0, MAX_COMPRESSED_SIZE(block_size), 0);
        break;
    case CMD_LIST:
    case CMD_LS:
    case CMD_INFO:
        r = alloc_mem(0, block_size, 0);
        break;
    default:
        r = alloc_mem(0, 0, 0);
        break;
    }

    return r;
}


void lzo_leave(const header_t *h)
{
#if defined(WITH_THREADS)
    if (opt_num_threads > 1) {
        lzo_threaded_leave(h);
        return;
   }
#endif
    if (h == NULL)
        free_mem();
}


/*************************************************************************
// compress a file
**************************************************************************/

lzo_bool lzo_compress(file_t *fip, file_t *fop, const header_t *h)
{
    int r = LZO_E_OK;
    lzo_bytep const b1 = blocks[0].mb_mem;
    lzo_bytep const b2 = blocks[1].mb_mem;
    lzo_uint32 src_len = 0;
    lzo_uint dst_len = 0;
    lzo_uint32 c_adler32 = ADLER32_INIT_VALUE, d_adler32 = ADLER32_INIT_VALUE;
    lzo_uint32 c_crc32 = CRC32_INIT_VALUE, d_crc32 = CRC32_INIT_VALUE;
    lzo_int l;
    lzo_bool ok = 1;

#if defined(WITH_THREADS)
    if (opt_num_threads > 1)
        return lzo_threaded_compress(fip, fop, h, skip);
#endif

    for (;;)
    {
        /* read a block */
        l = read_buf(fip, b1, block_size);
        src_len = (l > 0 ? l : 0);

        /* write uncompressed block size */
        write32(fop,src_len);

        /* exit if last block */
        if (src_len == 0)
            break;

        /* compute checksum of uncompressed block */
        if (h->flags & F_ADLER32_D)
            d_adler32 = lzo_adler32(ADLER32_INIT_VALUE,b1,src_len);
        if (h->flags & F_CRC32_D)
            d_crc32 = lzo_crc32(CRC32_INIT_VALUE,b1,src_len);

        x_filter(b1,src_len,h);

        /* compress */
        if (h->method == M_LZO1X_1)
            r = lzo1x_1_compress(b1, src_len, b2, &dst_len, wrkmem.mb_mem);
#if defined(USE_LZO1X_1_15)
        else if (h->method == M_LZO1X_1_15)
            r = lzo1x_1_15_compress(b1, src_len,
                                    b2, &dst_len, wrkmem.mb_mem);
#endif
#if defined(USE_LZO1X_999)
        else if (h->method == M_LZO1X_999)
            r = lzo1x_999_compress_level(b1, src_len,
                                         b2, &dst_len, wrkmem.mb_mem,
                                         NULL, 0, 0, h->level);
#endif
        else
            fatal(fip,"Internal error");

#if 0
        fprintf(stderr, "%ld %ld %ld\n", (long)src_len, (long)dst_len, (long)block2.size);
#endif
        assert(dst_len <= blocks[1].mb_size);
        if (r != LZO_E_OK)
            fatal(fip,"Internal error - compression failed");

        /* optimize */
        if (opt_optimize && dst_len < src_len)
        {
            lzo_uint new_len = src_len;
            r = lzo1x_optimize(b2, dst_len, b1, &new_len, NULL);
            if (r != LZO_E_OK || new_len != src_len)
                fatal(fip,"Internal error - optimization failed");
        }

        /* write compressed block size */
        if (dst_len < src_len)
            write32(fop,dst_len);
        else
            write32(fop,src_len);

        /* write checksum of uncompressed block */
        if (h->flags & F_ADLER32_D)
            write32(fop,d_adler32);
        if (h->flags & F_CRC32_D)
            write32(fop,d_crc32);

        /* write checksum of compressed block */
        if (dst_len < src_len && (h->flags & F_ADLER32_C))
        {
            c_adler32 = lzo_adler32(ADLER32_INIT_VALUE,b2,dst_len);
            write32(fop,c_adler32);
        }
        if (dst_len < src_len && (h->flags & F_CRC32_C))
        {
            c_crc32 = lzo_crc32(CRC32_INIT_VALUE,b2,dst_len);
            write32(fop,c_crc32);
        }

        /* write compressed block data */
        if (dst_len < src_len)
            write_buf(fop,b2,dst_len);
        else
            write_buf(fop,b1,src_len);
    }

    return ok;
}


/*************************************************************************
// decompress a file
**************************************************************************/

lzo_bool lzo_decompress(file_t *fip, file_t *fop,
                        const header_t *h, lzo_bool skip)
{
    int r;
    lzo_uint32 src_len, dst_len;
    lzo_uint32 c_adler32 = ADLER32_INIT_VALUE, d_adler32 = ADLER32_INIT_VALUE;
    lzo_uint32 c_crc32 = CRC32_INIT_VALUE, d_crc32 = CRC32_INIT_VALUE;
    lzo_bool ok = 1;
    lzo_bool use_seek;
    mblock_t * const block = &blocks[1];
    lzo_bytep b1;
    lzo_bytep const b2 = block->mb_mem;

#if defined(WITH_THREADS)
    if (opt_num_threads > 1)
        return lzo_threaded_decompress(fip, fop, h, skip);
#endif

    use_seek = skip || opt_cmd == CMD_LIST || opt_cmd == CMD_LS ||
                       opt_cmd == CMD_INFO;

    for (;;)
    {
        lzo_bytep dst;

        /* read uncompressed block size */
        read32(fip,&dst_len);

        /* exit if last block */
        if (dst_len == 0)
            break;

        /* error if split file */
        if (dst_len == 0xffffffffUL)
        {
            /* should not happen - not yet implemented */
            error(fip,"this file is a split " PACKAGE " file");
            ok = 0; break;
        }

        if (dst_len > MAX_BLOCK_SIZE)
        {
            error(fip, PACKAGE " file corrupted");
            ok = 0; break;
        }

        /* read compressed block size */
        read32(fip,&src_len);
        if (src_len <= 0 || src_len > dst_len)
        {
            error(fip, PACKAGE " file corrupted");
            ok = 0; break;
        }

        if (dst_len > BLOCK_SIZE)
        {
            fatal(fip,"block size too small -- recompile " PACKAGE);
            ok = 0; break;
        }
        if (dst_len > block_size)
        {
            /* should not happen - not yet implemented */
            fatal(fip,"block size too small -- use option '--blocksize'");
            ok = 0; break;
        }
        assert(block->mb_size >= src_len);

        /* read checksum of uncompressed block */
        if (h->flags & F_ADLER32_D)
            read32(fip,&d_adler32);
        if (h->flags & F_CRC32_D)
            read32(fip,&d_crc32);

        /* read checksum of compressed block */
        if (h->flags & F_ADLER32_C)
        {
            if (src_len < dst_len)
                read32(fip,&c_adler32);
            else
            {
                assert(h->flags & F_ADLER32_D);
                c_adler32 = d_adler32;
            }
        }
        if (h->flags & F_CRC32_C)
        {
            if (src_len < dst_len)
                read32(fip,&c_crc32);
            else
            {
                assert(h->flags & F_CRC32_D);
                c_crc32 = d_crc32;
            }
        }

        /* read the block */
        b1 = block->mb_mem + block->mb_size - src_len;
        if (use_seek && fip->fd != STDIN_FILENO)
        {
            if (lseek(fip->fd, src_len, SEEK_CUR) == -1)
                read_error(fip);
        }
        else
        {
            if (read_buf(fip, b1, src_len) != (lzo_int) src_len)
                read_error(fip);
        }

        fip->bytes_processed += src_len;
        if (use_seek)
        {
            fop->bytes_processed += dst_len;
            continue;
        }
        assert(block->mb_size >= MAX_COMPRESSED_SIZE(dst_len));

        /* verify checksum of compressed block */
        if (opt_checksum && (h->flags & F_ADLER32_C))
        {
            lzo_uint32 c;
            c = lzo_adler32(ADLER32_INIT_VALUE,b1,src_len);
            if (c != c_adler32)
            {
                error(fip,"Checksum error (" PACKAGE " file corrupted)");
                ok = 0; break;
            }
        }
        if (opt_checksum && (h->flags & F_CRC32_C))
        {
            lzo_uint32 c;
            c = lzo_crc32(CRC32_INIT_VALUE,b1,src_len);
            if (c != c_crc32)
            {
                error(fip,"Checksum error (" PACKAGE " file corrupted)");
                ok = 0; break;
            }
        }

        if (src_len < dst_len)
        {
            lzo_uint d = dst_len;

            /* decompress */
            if (opt_decompress_safe)
                r = lzo1x_decompress_safe(b1,src_len,b2,&d,NULL);
            else
                r = lzo1x_decompress(b1,src_len,b2,&d,NULL);

            if (r != LZO_E_OK || dst_len != d)
            {
                error(fip,"Compressed data violation");
#if 0
                fprintf(stderr,"%d %ld %ld\n", r, (long)dst_len, (long)d);
#endif
                ok = 0; break;
            }
            dst = b2;
        }
        else
        {
            assert(dst_len == src_len);
            dst = b1;
        }

        x_filter(dst,dst_len,h);

        /* verify checksum of uncompressed block */
        if (opt_checksum && (h->flags & F_ADLER32_D))
        {
            lzo_uint32 c;
            c = lzo_adler32(ADLER32_INIT_VALUE,dst,dst_len);
            if (c != d_adler32)
            {
                error(fip,"Checksum error");
                ok = 0; break;
            }
        }
        if (opt_checksum && (h->flags & F_CRC32_D))
        {
            lzo_uint32 c;
            c = lzo_crc32(CRC32_INIT_VALUE,dst,dst_len);
            if (c != d_crc32)
            {
                error(fip,"Checksum error");
                ok = 0; break;
            }
        }

        /* write uncompressed block data */
        write_buf(fop,dst,dst_len);
        fop->bytes_processed += dst_len;
    }

    return ok;
}


#endif /* WITH_LZO */


/*
vi:ts=4:et
*/

