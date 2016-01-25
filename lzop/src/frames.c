/* frames.c --

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

#if defined(USE_FRAMES)

#include "screen.h"


/*************************************************************************
//
**************************************************************************/

static void recolor_line(screen_t *screen, int frame, int line,
                         lzo_bytep f, int len, int cy, int col)
{
    unsigned char colmap[4];
    int k;

    UNUSED(screen);
    UNUSED(frame);
    UNUSED(line);
    UNUSED(f);
    UNUSED(len);
    UNUSED(cy);

    colmap[0] = 0;          /* black background */
    if (col <= 0x333)
    {
        /* palette */
        static const unsigned char pal[4] = { 0x2, 0x3, 0x5, 0x5 };
        colmap[1] = pal[ (col >> 8) & 3 ];
        colmap[2] = pal[ (col >> 4) & 3 ];
        colmap[3] = pal[ (col >> 0) & 3 ];
    }
    else
    {
        /* direct colors */
        colmap[1] = (col >> 16) & 0x07;
        colmap[2] = (col >>  8) & 0x07;
        colmap[3] = (col >>  0) & 0x07;
    }

#if 0
    for (k = 0; k < len; k += 2)
        if (f[k+1] != 0)
        {
            if (f[k] != ' ')
                f[k] = '.';
            f[k+1] = 0x10;
        }
        else
            assert(f[k] == ' ');
#endif

#if 1
    for (k = 0; k < len; k += 2)
    {
        lzo_bytep p = &f[k+1];
        if (*p != 0)
            *p = (colmap[ (*p >> 4) & 3 ] << 4) | colmap[ *p & 3 ];
    }
#endif

#if 0
    f[0*2+0] = f[77*2+0] = line / 10 + '0';
    f[0*2+1] = f[77*2+1] = 0x07;
    f[1*2+0] = f[78*2+0] = line % 10 + '0';
    f[1*2+1] = f[78*2+1] = 0x07;
#if 1
    if (line == 18 - 1)
    {
        char s[2*80+1];
        int y = cy - 4;

        sprintf(s,"  %2d: %2d %2d  ", frame, 0, 0);
        screen->putStringAttr(screen,s,0x70,33,y+1);
        for (k = 0; k < 79; k++)
        {
            screen->putCharAttr(screen,k%10+'0',k%10==0?0x06:0x07,k,y+3);
            screen->putCharAttr(screen,k%10+'0',k%10==0?0x06:0x07,k,y+3+18+1);
        }
#if 1
        usleep(200*1000);
#else
        while (!screen->kbhit(screen))
            ;
        while (screen->kbhit(screen))
            if (getch() == 27)
                exit(0);
#endif
    }
#endif
#endif
}


static int random_color(void)
{
    static const short c[6] = { 0x012, 0x021, 0x102, 0x120, 0x201, 0x210 };
    int t;
    lzo_uint32 seed;

    seed = time(NULL);
#if 0
    seed = seed * 69069L + 5;
    t = (int)(seed >> 16) % 50;
#else
    seed = seed * 0x015a4e35L + 1;
    t = (int)(seed >> 16) % 50;
#endif
    return (t < 6 ? c[t] : 0x333);
}


/*************************************************************************
//
**************************************************************************/

#include "frames.h"

void screen_show_frames(screen_t *screen)
{
    int i, j, k;
    lzo_uint out_len = UNCOMPRESSED_SIZE;
    lzo_bytep frames = NULL;
    lzo_bytep frames_mem = NULL;
    int c_cx, c_cy;
    const int len = 79 * 2;         /* size of one line */
    const int nframes = 19;         /* number of frames */
    const int lines = 18;           /* intro */
    const int total_lines = 24;     /* head + help */
    lzo_bool hit = 0;
    int col;
    lzo_uint32 c;

    assert(acc_isatty(STDIN_FILENO));
    assert(out_len == (lzo_uint) (lines * len * nframes));

#if 0
    c = lzo_adler32(ADLER32_INIT_VALUE,compressed_frames,COMPRESSED_SIZE);
    if (c != COMPRESSED_ADLER32)
        exit(EXIT_ERROR);
#endif

    frames = frames_mem = (lzo_bytep) malloc(UNCOMPRESSED_SIZE + 3);
    if (frames == NULL)
        return;

    if ((lzo1x_decompress_safe(compressed_frames,COMPRESSED_SIZE,
                               frames,&out_len,NULL) != LZO_E_OK)
       || (out_len != UNCOMPRESSED_SIZE))
        exit(EXIT_ERROR);
#if 0
    c = lzo_adler32(ADLER32_INIT_VALUE,frames,UNCOMPRESSED_SIZE);
    if (c != UNCOMPRESSED_ADLER32)
        exit(EXIT_ERROR);
#endif

    screen->getCursor(screen,&c_cx,&c_cy);
    c_cy += total_lines;
    if (c_cy >= screen->getRows(screen))
        c_cy -= screen->scrollUp(screen,c_cy-screen->getRows(screen)+1);
    c_cy -= total_lines;
    if (c_cy < 0)
        c_cy = 0;
    c_cx = 0;
    screen->setCursor(screen,c_cx,c_cy);
#if 1
    for (k = 0; k < 4; k++)
        screen->clearLine(screen,c_cy+k);
#endif
    head();
    screen->getCursor(screen,&c_cx,&c_cy);
#if 1
    for (k = 0; k < lines; k++)
        screen->clearLine(screen,c_cy+k);
#endif
    screen->refresh(screen);

    col = random_color();
    for (i = 0; i < nframes && !hit; i++)
    {
        for (j = 0; j < lines; j++)
        {
            recolor_line(screen,i,j,frames,len,c_cy,col);
            screen->updateLineN(screen,frames,c_cy+j,len);
            frames += len;
        }
        screen->refresh(screen);

        if (i == 0)
        {
            for (j = 0; !(hit = screen->kbhit(screen)) && j < 10; j++)
                usleep(120*1000);
        }
        else
        {
            if (!(hit = screen->kbhit(screen)))
                usleep(60*1000);
        }
    }

    free(frames_mem);

    for (k = 0; k < lines; k++)
        screen->clearLine(screen,c_cy+k);
    screen->refresh(screen);
    help();

    UNUSED(c);
}


#endif /* USE_FRAMES */


/*
vi:ts=4:et
*/

