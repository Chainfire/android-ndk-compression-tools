/* c_screen.c --

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

#if defined(USE_SCREEN)

#include "screen.h"

#define mask_fg 0x0f
#define mask_bg 0xf0


/*************************************************************************
//
**************************************************************************/

static screen_t *do_construct(screen_t *s, int fd)
{
    if (!s)
        return NULL;
    if (s->init(s,fd) != 0)
    {
        s->destroy(s);
        return NULL;
    }
    return s;
}


static screen_t *screen = NULL;

static void do_destroy(void)
{
    if (screen)
    {
        screen->destroy(screen);
        screen = NULL;
    }
}


static int mode = -1;
static int init_fg = -1;
static int init_bg = -1;
static int cur_fg = -1;
static int cur_bg = -1;


static int init(FILE *f, int o, int now)
{
    int fd = fileno(f);

    UNUSED(now);
    assert(screen == NULL);
    atexit(do_destroy);
#if defined(__DJGPP__)
    if (!screen)
        screen = do_construct(screen_djgpp2_construct(),fd);
#endif
#if defined(USE_SCREEN_VCSA)
    if (!screen)
        screen = do_construct(screen_vcsa_construct(),fd);
#endif
#if defined(USE_SCREEN_CURSES)
    if (!screen && o == CON_SCREEN)
        screen = do_construct(screen_curses_construct(),fd);
#endif
    if (!screen)
        return CON_INIT;
    mode = screen->getMode(screen);
    init_fg = cur_fg = screen->getFg(screen);
    init_bg = cur_bg = screen->getBg(screen);
    if (mode == 7)
        cur_fg = -1;
    if (screen->getCols(screen) < 80 || screen->getRows(screen) < 24)
        return CON_INIT;
    if (cur_fg == (cur_bg >> 4))
        return CON_INIT;
    if (cur_bg != BG_BLACK)
        if (mode != 7)
        {
            /* return CON_ANSI_MONO; */     /* we could emulate ANSI mono */
            return CON_INIT;
        }

    if (o == CON_SCREEN)
        return CON_SCREEN;
    if (o == CON_INIT)                      /* use by default */
        return CON_SCREEN;
    if (o == CON_ANSI_COLOR)                /* can emulate ANSI color */
        return CON_ANSI_COLOR;
    if (o == CON_ANSI_MONO)                 /* can emulate ANSI mono */
        return CON_ANSI_MONO;

    return CON_INIT;
}


static int set_fg(FILE *f, int fg)
{
    const int last_fg = cur_fg;
    int f1 = fg & mask_fg;
    int f2 = init_fg & mask_fg;

    UNUSED(f);
    cur_fg = fg;
    if (mode == 7)
    {
        const int b = (init_bg & mask_bg) >> 4;
        if (fg == -1)           /* restore startup fg */
            f1 = f2;
        else if (b == 0)
            f1 = (f2 <= 8) ? 15 : 8;
        else if (b <= 8)
            f1 = (f2 == 0) ? 15 : 0;
        else
            f1 = (f2 == 0) ? 8 : 0;
    }
    else if (con_mode == CON_ANSI_MONO && f1 != f2)
    {
        f1 = f2 ^ 0x08;
    }

    screen->setFg(screen,f1 & mask_fg);
    return last_fg;
}


static void print0(FILE *f, const char *s)
{
    int c_cx, c_cy;

    UNUSED(f);
    screen->getCursor(screen,&c_cx,&c_cy);
    while (*s)
    {
        while (*s == '\n')
        {
            s++;
            c_cy++;
            c_cx = 0;
        }
        if (c_cy >= screen->getRows(screen))
        {
            c_cy -= screen->scrollUp(screen,c_cy-screen->getRows(screen)+1);
            if (c_cy < 0)
                c_cy = 0;
            c_cx = 0;
        }
        if (*s)
        {
            screen->putChar(screen,*s,c_cx,c_cy);
            s++;
            c_cx++;
        }
    }
    screen->setCursor(screen,c_cx,c_cy);
    screen->refresh(screen);
}


static lzo_bool intro(FILE *f)
{
    UNUSED(f);
#if defined(USE_FRAMES)
    if (screen->intro)
        return screen->intro(screen,screen_show_frames);
#endif
    return 0;
}


console_t console_screen =
{
    init,
    set_fg,
    print0,
    intro
};

#endif /* USE_SCREEN */


/*
vi:ts=4:et
*/

