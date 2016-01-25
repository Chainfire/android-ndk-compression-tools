/* compress.c --

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


/*************************************************************************
//
**************************************************************************/

int x_set_method(int m, int l)
{
    int r = -1;

#if defined(WITH_LZO)
    r = lzo_set_method(m,l);
    if (r >= 0)
        return r;
#endif
#if defined(WITH_NRV)
    r = nrv_set_method(m,l);
    if (r >= 0)
        return r;
#endif
#if defined(WITH_ZLIB)
    r = zlib_set_method(m,l);
    if (r >= 0)
        return r;
#endif

    UNUSED(r);
    return -1;
}


int x_get_method(header_t *h)
{
    int r = -1;

#if defined(WITH_LZO)
    r = lzo_get_method(h);
    if (r >= 0)
        return r;
#endif
#if defined(WITH_NRV)
    r = nrv_get_method(h);
    if (r >= 0)
        return r;
#endif
#if defined(WITH_ZLIB)
    r = zlib_get_method(h);
    if (r >= 0)
        return 0;
#endif

    UNUSED(r);
    return 14;
}


/*************************************************************************
// memory setup
**************************************************************************/

lzo_bool x_enter(const header_t *h)
{
    if (h == NULL)
    {
        lzo_bool ok = 1;
#if defined(WITH_LZO)
        ok &= lzo_enter(NULL);
#endif
#if defined(WITH_NRV)
        ok &= nrv_enter(NULL);
#endif
#if defined(WITH_ZLIB)
        ok &= zlib_enter(NULL);
#endif
        return ok;
    }

    switch (h->method)
    {
#if defined(WITH_LZO)
    case M_LZO1X_1:
    case M_LZO1X_1_15:
    case M_LZO1X_999:
        return lzo_enter(h);
#endif
#if defined(WITH_NRV)
    case M_NRV1A:
    case M_NRV1B:
    case M_NRV2A:
    case M_NRV2B:
        return nrv_enter(h);
#endif
#if defined(WITH_ZLIB)
    case M_ZLIB:
        return zlib_enter(h);
#endif
    }
    return 0;
}


void x_leave(const header_t *h)
{
    if (h == NULL)
    {
#if defined(WITH_ZLIB)
        zlib_leave(NULL);
#endif
#if defined(WITH_NRV)
        nrv_leave(NULL);
#endif
#if defined(WITH_LZO)
        lzo_leave(NULL);
#endif
        return;
    }

    switch (h->method)
    {
#if defined(WITH_LZO)
    case M_LZO1X_1:
    case M_LZO1X_1_15:
    case M_LZO1X_999:
        lzo_leave(h);
        break;
#endif
#if defined(WITH_NRV)
    case M_NRV1A:
    case M_NRV1B:
    case M_NRV2A:
    case M_NRV2B:
        nrv_leave(h);
        break;
#endif
#if defined(WITH_ZLIB)
    case M_ZLIB:
        zlib_leave(h);
        break;
#endif
    }
}


/*************************************************************************
// compress a file
**************************************************************************/

lzo_bool x_compress(file_t *fip, file_t *fop, header_t *h)
{
    lzo_bool ok = 0;

    init_compress_header(h,fip,fop);
    switch (h->method)
    {
#if defined(WITH_LZO)
    case M_LZO1X_1:
    case M_LZO1X_1_15:
    case M_LZO1X_999:
        lzo_init_compress_header(h);
        break;
#endif
#if defined(WITH_NRV)
    case M_NRV1A:
    case M_NRV1B:
    case M_NRV2A:
    case M_NRV2B:
        nrv_init_compress_header(h);
        break;
#endif
#if defined(WITH_ZLIB)
    case M_ZLIB:
        zlib_init_compress_header(h);
        break;
#endif
    default:
        fatal(fip,"Internal error");
        break;
    }

    if (!x_enter(h))
        e_memory();

    if (opt_verbose > 1)
    {
        if (opt_unlink)
            fprintf(con_term,"replacing %s with %s", fip->name, fop->name);
        else
            fprintf(con_term,"compressing %s into %s", fip->name, fop->name);
        fflush(con_term);
        set_err_nl(1);
    }

    write_header(fop,h);

    fip->bytes_processed = fop->bytes_processed = 0;

    switch (h->method)
    {
#if defined(WITH_LZO)
    case M_LZO1X_1:
    case M_LZO1X_1_15:
    case M_LZO1X_999:
        ok = lzo_compress(fip,fop,h);
        break;
#endif
#if defined(WITH_NRV)
    case M_NRV1A:
    case M_NRV1B:
    case M_NRV2A:
    case M_NRV2B:
        ok = nrv_compress(fip,fop,h);
        break;
#endif
#if defined(WITH_ZLIB)
    case M_ZLIB:
        ok = zlib_compress(fip,fop,h);
        break;
#endif
    default:
        fatal(fip,"Internal error");
        ok = 0;
        break;
    }

    if (opt_cmd == CMD_COMPRESS && opt_verbose > 1)
    {
        fprintf(con_term, ok ? "\n" : " FAILED\n");
        fflush(con_term);
    }
    set_err_nl(0);

    x_leave(h);
    return ok;
}


/*************************************************************************
// decompress a file
**************************************************************************/

lzo_bool x_decompress(file_t *fip, file_t *fop,
                      const header_t *h, lzo_bool skip)
{
    lzo_bool ok = 0;

    if (!x_enter(h))
        e_memory();

    if (skip)
    {
        assert(opt_cmd != CMD_TEST);
        assert(fop->fd < 0);
        if (opt_cmd == CMD_DECOMPRESS && opt_verbose > 0)
            fprintf(con_term,"skipping %s [%s]", fip->name, fop->name);
        fflush(con_term);
        set_err_nl(1);
    }
    else if (opt_cmd == CMD_DECOMPRESS && opt_verbose > 1)
    {
        if (opt_unlink)
            fprintf(con_term,"restoring %s into %s", fip->name, fop->name);
        else
            fprintf(con_term,"decompressing %s into %s", fip->name, fop->name);
        fflush(con_term);
        set_err_nl(1);
    }
    else if (opt_cmd == CMD_TEST && opt_verbose > 0)
    {
        /* note: gzip is quiet by default when testing, lzop is not */
        if (opt_verbose > 2)
            fprintf(con_term,"testing %s [%s]", fip->name, fop->name);
        else
            fprintf(con_term,"testing %s", fip->name);
        fflush(con_term);
        set_err_nl(1);
    }

    fip->bytes_processed = fop->bytes_processed = 0;

    switch (h->method)
    {
#if defined(WITH_LZO)
    case M_LZO1X_1:
    case M_LZO1X_1_15:
    case M_LZO1X_999:
        ok = lzo_decompress(fip,fop,h,skip);
        break;
#endif
#if defined(WITH_NRV)
    case M_NRV1A:
    case M_NRV1B:
    case M_NRV2A:
    case M_NRV2B:
        ok = nrv_decompress(fip,fop,h,skip);
        break;
#endif
#if defined(WITH_ZLIB)
    case M_ZLIB:
        ok = zlib_decompress(fip,fop,h,skip);
        break;
#endif
    default:
        fatal(fip,"Internal error");
        ok = 0;
        break;
    }

    if (skip && opt_cmd == CMD_DECOMPRESS && opt_verbose > 0)
    {
        fprintf(con_term, ok ? "\n" : " FAILED\n");
        fflush(con_term);
    }
    else if (opt_cmd == CMD_DECOMPRESS && opt_verbose > 1)
    {
        fprintf(con_term, ok ? "\n" : " FAILED\n");
        fflush(con_term);
    }
    else if (opt_cmd == CMD_TEST && opt_verbose > 0)
    {
        fprintf(con_term, ok ? " OK\n" : " FAILED\n");
        fflush(con_term);
    }
    set_err_nl(0);

    if (ok && opt_cmd == CMD_TEST)
        do_test(h,fop->bytes_processed,fip->bytes_processed);
    if (ok && opt_cmd == CMD_LIST)
        do_list(h,fop->bytes_processed,fip->bytes_processed);
    if (ok && opt_cmd == CMD_LS)
        do_ls(h,fop->bytes_processed,fip->bytes_processed);
    if (ok && opt_cmd == CMD_INFO)
        do_info(h,fop->bytes_processed,fip->bytes_processed);

    x_leave(h);
    return ok;
}


/*************************************************************************
//
**************************************************************************/

void x_filter(lzo_bytep p, lzo_uint l, const header_t *h)
{
    unsigned f = (unsigned) h->filter;
    lzo_bool c = (opt_cmd == CMD_COMPRESS);

    if (f == 0 || l <= 0)
        return;
    if (f == 1)
    {
        if (c) t_sub1(p,l); else t_add1(p,l);
    }
    else if (f <= 16)
    {
        if (c) t_sub(p,l,f); else t_add(p,l,f);
    }
    else
    {
        fatal(NULL,"Invalid filter");
    }
}



/*
vi:ts=4:et
*/

