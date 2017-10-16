#include "Auxiliar.h"
#include "Assets.h"
#include "List.h"

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#else
#include <ctime>
#include <sys/time.h>
#endif

#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>

SDL_Surface* loadImage(const char* path)
{
    return IMG_Load(assets::makePath(path));
}

Mix_Music* loadMusic(const char* path)
{
    return Mix_LoadMUS(assets::makePath(path));
}

Mix_Chunk* loadSound(const char* path)
{
    return Mix_LoadWAV(assets::makePath(path));
}

TTF_Font* loadFont(const char* path, int size)
{
    return TTF_OpenFont(assets::makePath(path), size);
}

#ifndef _WIN32

struct timeval init_tick_count_value;

void setupTickCount()
{
    gettimeofday(&init_tick_count_value, NULL);
}

long GetTickCount()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval diff;
    diff.tv_sec = now.tv_sec - init_tick_count_value.tv_sec;
    diff.tv_usec = now.tv_usec - init_tick_count_value.tv_usec;
    if (diff.tv_usec < 0)
    {
        diff.tv_sec--;
        diff.tv_usec += 1000000;
    }
    return diff.tv_sec * 1000 + diff.tv_usec / 1000;
}

#endif

void pause(unsigned int time)
{
    unsigned int initt = GetTickCount();

    while ((GetTickCount() - initt) < time)
        ;
}

SDL_Surface* load_maskedimage(char* imagefile, char* maskfile, char* path)
{
    char name[256];

    sprintf(name, "%s%s", path, imagefile);
    SDL_Surface* tmp = loadImage(name);
    sprintf(name, "%s%s", path, imagefile);
    SDL_Surface* mask = loadImage(name);

    if (tmp == nullptr || mask == nullptr)
        return nullptr;

    SDL_Surface* res = SDL_DisplayFormatAlpha(tmp);

    /* Aplicar la m�scara: */
    {
        SDL_LockSurface(res);
        SDL_LockSurface(mask);

        for (int y = 0; y < mask->h; y++)
        {
            for (int x = 0; x < mask->w; x++)
            {
                Uint32 v = getpixel(res, x, y);

                Uint8 r, g, b, a;
                SDL_GetRGBA(v, res->format, &r, &g, &b, &a);

                v = getpixel(mask, x, y);

                if (v != 0)
                    a = 255;
                else
                    a = 0;

                v = SDL_MapRGBA(res->format, r, g, b, a);

                putpixel(res, x, y, v);
            }
        }

        SDL_UnlockSurface(mask);
        SDL_UnlockSurface(res);
    }

    SDL_FreeSurface(tmp);
    SDL_FreeSurface(mask);

    return res;
}

void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
    SDL_Rect clip;
    int bpp = surface->format->BytesPerPixel;

    SDL_GetClipRect(surface, &clip);

    if (x < clip.x || x >= clip.x + clip.w || y < clip.y || y >= clip.y + clip.h)
        return;

    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return;

    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16*)p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = pixel;
        break;
    }
}

void maximumpixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
    SDL_Rect clip;
    int bpp = surface->format->BytesPerPixel;
    Uint32 r, g, b, r2, g2, b2;
    Uint8* p;

    SDL_GetClipRect(surface, &clip);

    if (x < clip.x || x >= clip.x + clip.w || y < clip.y || y >= clip.y + clip.h)
        return;

    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h || bpp != 4)
        return;

    p = (Uint8*)&pixel;

    r2 = p[ROFFSET];
    g2 = p[GOFFSET];
    b2 = p[BOFFSET];

    p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    r = p[ROFFSET];
    g = p[GOFFSET];
    b = p[BOFFSET];

    *(Uint32*)p = SDL_MapRGB(surface->format, std::max(r, r2), std::max(g, g2), std::max(b, b2));
}

Uint32 getpixel(SDL_Surface* surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;

    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return 0;

    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
    case 1:
        return *p;

    case 2:
        return *(Uint16*)p;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32*)p;

    default:
        return 0;
    }
}

void surface_fader(SDL_Surface* surface, float r_factor, float g_factor, float b_factor, SDL_Rect* r)
{
    SDL_Rect r2;
    int i, x, y, offs;
    Uint8 rtable[256], gtable[256], btable[256];
    Uint8* pixels;
    SDL_Surface* tmp;

    if (r == 0)
    {
        r2.x = 0;
        r2.y = 0;
        r2.w = surface->w;
        r2.h = surface->h;
        r = &r2;
    }

    if (surface->format->BytesPerPixel != 4 || (r_factor == 1.0 && g_factor == 1.0 && b_factor == 1.0))
        return;

    for (i = 0; i < 256; i++)
    {
        rtable[i] = (Uint8)(i * r_factor);
        gtable[i] = (Uint8)(i * g_factor);
        btable[i] = (Uint8)(i * b_factor);
    }

    if ((surface->flags & SDL_HWSURFACE) != 0)
    {
        /* HARDWARE SURFACE!!!: */
        tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h, 32, 0, 0, 0, 0);
        SDL_BlitSurface(surface, 0, tmp, 0);
        SDL_LockSurface(tmp);
        pixels = (Uint8*)(tmp->pixels);
    }
    else
    {
        SDL_LockSurface(surface);
        pixels = (Uint8*)(surface->pixels);
    }

    for (y = r->y; y < r->y + r->h && y < surface->h; y++)
    {
        for (x = r->x, offs = y * surface->pitch + r->x * 4; x < r->x + r->w && x < surface->w; x++, offs += 4)
        {
            pixels[offs + ROFFSET] = rtable[pixels[offs + ROFFSET]];
            pixels[offs + GOFFSET] = gtable[pixels[offs + GOFFSET]];
            pixels[offs + BOFFSET] = btable[pixels[offs + BOFFSET]];
        }
    }

    if ((surface->flags & SDL_HWSURFACE) != 0)
    {
        /* HARDWARE SURFACE!!!: */
        SDL_UnlockSurface(tmp);
        SDL_BlitSurface(tmp, 0, surface, 0);
        SDL_FreeSurface(tmp);
    }
    else
    {
        SDL_UnlockSurface(surface);
    }
}

void surface_shader(SDL_Surface* surface, float factor, int red, int green, int blue, int alpha)
{
    int x, y, offs;
    Uint8* pixels;
    int ifactor = int(factor * 256), inv_ifactor;

    if (ifactor < 0)
        ifactor = 0;
    if (ifactor >= 256)
        ifactor = 256;
    inv_ifactor = 256 - ifactor;

    if (surface->format->BytesPerPixel != 4)
        return;

    SDL_LockSurface(surface);
    pixels = (Uint8*)(surface->pixels);

    for (y = 0; y < surface->h; y++)
    {
        for (x = 0, offs = y * surface->pitch; x < surface->w; x++, offs += 4)
        {
            if (red >= 0)
                pixels[offs + ROFFSET] = (red * ifactor + pixels[offs + ROFFSET] * (inv_ifactor)) >> 8;
            if (green >= 0)
                pixels[offs + GOFFSET] = (green * ifactor + pixels[offs + GOFFSET] * (inv_ifactor)) >> 8;
            if (blue >= 0)
                pixels[offs + BOFFSET] = (blue * ifactor + pixels[offs + BOFFSET] * (inv_ifactor)) >> 8;
            if (alpha >= 0)
                pixels[offs + AOFFSET] = (alpha * ifactor + pixels[offs + AOFFSET] * (inv_ifactor)) >> 8;
        }
    }

    SDL_UnlockSurface(surface);
}

void surface_bicolor(SDL_Surface* surface, float factor, int r1, int g1, int b1, int a1, int r2, int g2, int b2, int a2)
{
    int x, y, offs;
    Uint8* pixels;
    int ifactor = int(factor * 256), inv_ifactor;
    int bw_color;

    if (ifactor < 0)
        ifactor = 0;
    if (ifactor >= 256)
        ifactor = 256;
    inv_ifactor = 256 - ifactor;

    if (surface->format->BytesPerPixel != 4)
        return;

    SDL_LockSurface(surface);
    pixels = (Uint8*)(surface->pixels);

    for (y = 0; y < surface->h; y++)
    {
        for (x = 0, offs = y * surface->pitch; x < surface->w; x++, offs += 4)
        {
            bw_color = (74 * pixels[offs + ROFFSET] + 154 * pixels[offs + GOFFSET] + 28 * pixels[offs + BOFFSET]) >> 8;

            if (r1 >= 0 && r2 >= 0)
                pixels[offs + BOFFSET] = (((b1 * bw_color + b2 * (256 - bw_color)) >> 8) * ifactor + pixels[offs + BOFFSET] * (inv_ifactor)) >> 8;
            if (g1 >= 0 && g2 >= 0)
                pixels[offs + GOFFSET] = (((g1 * bw_color + g2 * (256 - bw_color)) >> 8) * ifactor + pixels[offs + GOFFSET] * (inv_ifactor)) >> 8;
            if (b1 >= 0 && b2 >= 0)
                pixels[offs + ROFFSET] = (((r1 * bw_color + r2 * (256 - bw_color)) >> 8) * ifactor + pixels[offs + ROFFSET] * (inv_ifactor)) >> 8;
            if (a1 >= 0 && a2 >= 0)
                pixels[offs + AOFFSET] = (((a1 * bw_color + a2 * (256 - bw_color)) >> 8) * ifactor + pixels[offs + AOFFSET] * (inv_ifactor)) >> 8;
        }
    }

    SDL_UnlockSurface(surface);
}

void draw_rectangle(SDL_Surface* surface, int x, int y, int w, int h, Uint32 pixel)
{
    SDL_LockSurface(surface);

    for (int i = 0; i < w; i++)
    {
        if (x + i >= 0 && x + i < surface->w && y >= 0 && y < surface->h)
            putpixel(surface, x + i, y, pixel);
        if (x + i >= 0 && x + i < surface->w && y + h - 1 >= 0 && y + h < surface->h)
            putpixel(surface, x + i, y + h - 1, pixel);
    }

    for (int i = 0; i < h; i++)
    {
        if (x >= 0 && x < surface->w && y + i >= 0 && y + i < surface->h)
            putpixel(surface, x, y + i, pixel);
        if (x + w - 1 >= 0 && x + w < surface->w && y + i >= 0 && y + i < surface->h)
            putpixel(surface, x + w - 1, y + i, pixel);
    }

    SDL_UnlockSurface(surface);
}

void draw_line(SDL_Surface* sfc, int x1, int y1, int x2, int y2, Uint32 pixel)
{
    SDL_LockSurface(sfc);
    draw_line_locked(sfc, x1, y1, x2, y2, pixel);
    SDL_UnlockSurface(sfc);
}

void draw_line_locked(SDL_Surface* sfc, int x1, int y1, int x2, int y2, Uint32 pixel)
{
    long incy, rincy, incx, errterm;

    int act_x = x1;
    int act_y = y1;
    errterm = 0;
    long d_x = x2 - x1;
    long d_y = y2 - y1;

    if (d_y < 0)
    {
        incy = -1;
        rincy = -1;
        d_y = -d_y;
    }
    else
    {
        incy = 1;
        rincy = 1;
    }

    if (d_x < 0)
    {
        incx = -1;
        d_x = -d_x;
    }
    else
    {
        incx = 1;
    }

    if (d_x > d_y)
    {
        /* xline */
        for (long a = 0; a <= d_x; a++)
        {
            putpixel(sfc, act_x, act_y, pixel);
            errterm += d_y;
            if (errterm >= d_x)
            {
                errterm -= d_x;
                act_y += incy;
            }
            act_x += incx;
        }
    }
    else
    {
        /* yline */
        for (long a = 0; a <= d_y; a++)
        {
            putpixel(sfc, act_x, act_y, pixel);
            errterm += d_x;
            if (errterm >= d_y)
            {
                errterm -= d_y;
                act_x += incx;
            }
            act_y += incy;
        }
    }
}

void surface_automatic_alpha(SDL_Surface* sfc)
{
    Uint32 color;
    Uint8 r, g, b, a;

    SDL_LockSurface(sfc);

    for (int i = 0; i < sfc->w; i++)
    {
        for (int j = 0; j < sfc->h; j++)
        {
            color = getpixel(sfc, i, j);

            SDL_GetRGBA(color, sfc->format, &r, &g, &b, &a);
            if (r != 0 || g != 0 || b != 0)
                a = 255;
            else
                a = 0;
            color = SDL_MapRGBA(sfc->format, r, g, b, a);

            putpixel(sfc, i, j, color);
        }
    }

    SDL_UnlockSurface(sfc);
}

void surface_bw(SDL_Surface* sfc, int threshold)
{
    SDL_LockSurface(sfc);

    for (int i = 0; i < sfc->w; i++)
    {
        for (int j = 0; j < sfc->h; j++)
        {
            Uint32 color = getpixel(sfc, i, j);

            Uint8 r, g, b, a;
            SDL_GetRGBA(color, sfc->format, &r, &g, &b, &a);
            if (r >= threshold || g >= threshold || b >= threshold)
                a = 255;
            else
                a = 0;
            color = SDL_MapRGBA(sfc->format, a, a, a, a);

            putpixel(sfc, i, j, color);
        }
    }

    SDL_UnlockSurface(sfc);
}

void surface_mask_from_bitmap(SDL_Surface* sfc, SDL_Surface* mask, int x, int y)
{
    SDL_LockSurface(mask);
    SDL_LockSurface(sfc);

    for (int i = 0; i < sfc->w; i++)
    {
        for (int j = 0; j < sfc->h; j++)
        {
            Uint32 color = getpixel(mask, x + i, y + j);

            Uint8 r, g, b, a;
            SDL_GetRGBA(color, sfc->format, &r, &g, &b, &a);
            int mean = (r + g + b) / 3;

            color = getpixel(sfc, i, j);

            SDL_GetRGBA(color, sfc->format, &r, &g, &b, &a);
            color = SDL_MapRGBA(sfc->format, r, g, b, mean);

            putpixel(sfc, i, j, color);
        }
    }

    SDL_UnlockSurface(sfc);
    SDL_UnlockSurface(mask);
}

SDL_Surface* multiline_text_surface(char* text, int line_dist, TTF_Font* font, SDL_Color c)
{
    int i, j, y;
    int sizex = 0, sizey = 0;
    SDL_Surface *tmp, *tmp2;
    List<SDL_Surface> sfc_l;
    char text_tmp[256];

    i = j = 0;

    while (text[i] != 0)
    {
        text_tmp[j] = text[i];
        if (text[i] == '\n')
        {
            text_tmp[j] = 0;
            tmp = TTF_RenderText_Blended(font, text_tmp, c);
            if (tmp->w > sizex)
                sizex = tmp->w;

            if (sizey != 0)
                sizey += line_dist;
            sizey += tmp->h;

            sfc_l.Add(tmp);
            j = 0;
        }
        else
        {
            j++;
        }
        i++;
    }

    tmp = SDL_CreateRGBSurface(0, sizex, sizey, 32, 0, 0, 0, 0);

    y = 0;
    while (!sfc_l.EmptyP())
    {
        SDL_Rect r;

        tmp2 = sfc_l.ExtractIni();

        r.x = (sizex - tmp2->w) / 2;
        r.y = y;
        r.w = tmp2->w;
        r.h = tmp2->h;
        SDL_BlitSurface(tmp2, 0, tmp, &r);
        y += tmp2->h;
        y += line_dist;
        SDL_FreeSurface(tmp2);
    }

    return tmp;
}

SDL_Surface* multiline_text_surface2(char* text, int line_dist, TTF_Font* font, SDL_Color c1, SDL_Color c2, int line, float glow)
{
    int i, j, current_line, y;
    int sizex = 0, sizey = 0;
    SDL_Surface *tmp, *tmp2;
    List<SDL_Surface> sfc_l;
    char text_tmp[256];

    i = j = 0;
    current_line = 0;
    while (text[i] != 0)
    {
        text_tmp[j] = text[i];
        if (text[i] == '\n')
        {
            text_tmp[j] = 0;
            if (current_line == line)
                tmp = TTF_RenderText_Blended(font, text_tmp, c2);
            else
                tmp = TTF_RenderText_Blended(font, text_tmp, c1);
            if (tmp->w > sizex)
                sizex = tmp->w;

            if (sizey != 0)
                sizey += line_dist;
            sizey += tmp->h;

            sfc_l.Add(tmp);
            j = 0;
            current_line++;
        }
        else
        {
            j++;
        }
        i++;
    }

    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, sizex, sizey, 32, 0, 0, 0, 0);

    y = 0;
    current_line = 0;
    while (!sfc_l.EmptyP())
    {
        SDL_Rect r;

        tmp2 = sfc_l.ExtractIni();

        r.x = (sizex - tmp2->w) / 2;
        r.y = y;
        r.w = tmp2->w;
        r.h = tmp2->h;
        SDL_BlitSurface(tmp2, 0, tmp, &r);

        if (current_line == line)
        {
            /* Create Glow: */
            int i, j, k;
            Uint32 c;
            Uint8 *p, v, v2, v3;
            Uint8 table[256];
            float f;

            for (f = glow, k = 0; f > 0.1F && k < 8; f *= glow, k++)
            {
                for (i = 0; i < 256; i++)
                    table[i] = (Uint8)(i * f);
                for (j = r.y; j < r.y + r.h; j++)
                {
                    p = (Uint8*)tmp->pixels + j * tmp->pitch + r.x * 4;
                    for (i = r.x; i < r.x + r.w; i++, p += 4)
                    {
                        v = (unsigned char)((int(p[ROFFSET]) + (int(p[GOFFSET]) << 1) + int(p[BOFFSET])) >> 2);
                        if (v != 0)
                        {
                            v2 = table[v];
                            v3 = table[v2];
                            c = SDL_MapRGB(tmp->format, v2, v3, v3);

                            maximumpixel(tmp, i + 1, j, c);
                            maximumpixel(tmp, i - 1, j, c);
                            maximumpixel(tmp, i, j + 1, c);
                            maximumpixel(tmp, i, j - 1, c);
                        }
                    }
                }
            }
        }

        y += tmp2->h;
        y += line_dist;
        SDL_FreeSurface(tmp2);
        current_line++;
    }

    return tmp;
}
