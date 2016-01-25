/* djgpp2.c --

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

#if defined(__DJGPP__)

#include <dpmi.h>
#include <go32.h>


/*************************************************************************
// init
**************************************************************************/

/* Minimum stack */
unsigned _stklen = 65536;


/* This is called before 'main'. */
static void __attribute__((__constructor__))
djgpp_startup (void)
{
    const char *s;
    static char lfn_y[] = "LFN=y";
    static char lfn_n[] = "LFN=n";

    s = getenv("LZOP_LFN");
    if (s == NULL || (s[0] != 'n' && s[0] != 'N'))
        putenv(lfn_y);
    else
    {
        putenv(lfn_n);
        _crt0_startup_flags |= _CRT0_FLAG_NO_LFN;
    }

    _djstat_flags |= _STAT_INODE;
    _djstat_flags |= _STAT_EXEC_EXT;
    _djstat_flags |= _STAT_EXEC_MAGIC;
    _djstat_flags |= _STAT_DIRSIZE;
    _djstat_flags |= _STAT_ROOT_TIME;
}


/* No need for loading the environment. */
void __crt0_load_environment_file(char *app_name) { UNUSED(app_name); }


/* No need for this function */
int _is_executable(const char *filename, int fhandle, const char *extension)
{
    UNUSED(filename);
    UNUSED(fhandle);
    UNUSED(extension);
    return 0;
}


/*************************************************************************
// info
**************************************************************************/

#if 0

typedef struct {
  unsigned long largest_available_free_block_in_bytes;
  unsigned long maximum_unlocked_page_allocation_in_pages;
  unsigned long maximum_locked_page_allocation_in_pages;
  unsigned long linear_address_space_size_in_pages;
  unsigned long total_number_of_unlocked_pages;
  unsigned long total_number_of_free_pages;
  unsigned long total_number_of_physical_pages;
  unsigned long free_linear_address_space_in_pages;
  unsigned long size_of_paging_file_partition_in_pages;
  unsigned long reserved[3];
} __dpmi_free_mem_info;

typedef struct {
  unsigned long available_memory;
  unsigned long available_pages;
  unsigned long available_lockable_pages;
  unsigned long linear_space;
  unsigned long unlocked_pages;
  unsigned long available_physical_pages;
  unsigned long total_physical_pages;
  unsigned long free_linear_space;
  unsigned long max_pages_in_paging_file;
  unsigned long reserved[3];
} _go32_dpmi_meminfo;

#endif

static void meminfo(void)
{
    __dpmi_free_mem_info info;
    const unsigned long err = (unsigned long) -1;
    unsigned long ps = 0;
    unsigned long k;

    if (__dpmi_get_page_size(&ps) != 0 || ps <= 0)
        return;
    if (__dpmi_get_free_memory_information(&info) != 0)
        return;

#if 0
    k = info.largest_available_free_block_in_bytes;
    if (k == err)
    {
        k = info.maximum_unlocked_page_allocation_in_pages;
        if (k != err)
            k *= ps;
        else
            k = info.largest_available_free_block_in_bytes;
    }
#endif

#if 0
    k = info.total_number_of_physical_pages;
    k = info.total_number_of_unlocked_pages;
#endif
    k = info.total_number_of_free_pages;
    if (k != err)
    {
        k = (k * ps) / 1024;
        con_fprintf(con_term,"DPMI physical memory available: %6ld kB\n",k);
    }

    k = info.maximum_unlocked_page_allocation_in_pages;
    if (k != err)
    {
        k = (k * ps) / 1024;
        con_fprintf(con_term,"DPMI virtual memory available:  %6ld kB\n",k);
    }
    con_fprintf(con_term,"\n");
}


void sysinfo_djgpp(void)
{
    meminfo();

    if (_USE_LFN)
        con_fprintf(con_term,"Long filenames are supported.\n");
    else
        con_fprintf(con_term,"Long filenames are not supported.\n");
    con_fprintf(con_term,"\n");
}


#endif /* __DJGPP__ */


/*
vi:ts=4:et
*/

