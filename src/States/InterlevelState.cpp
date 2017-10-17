#include "Auxiliar.h"
#include "Debug.h"
#include "Game.h"
#include "List.h"
#include "RoadFighter.h"

#include <algorithm>
#include <SDL_image.h>

#if defined(EMSCRIPTEN)
#define SDL_DisplayFormat SDL_DisplayFormatAlpha
#endif

int interlevel_time = 25;
int nlevels = 6;
extern int SCREEN_X;
extern int SCREEN_Y;
extern int start_level;

const char* maps[6] = { "maps/level1.mg2",
                        "maps/level2.mg2",
                        "maps/level3.mg2",
                        "maps/level4.mg2",
                        "maps/level5.mg2",
                        "maps/level6.mg2" };

int CRoadFighter::interlevel_cycle()
{
    if (state_timmer == 0)
    {
        output_debug_message("CRoadFighter::interlevel_cycle: cycle 0.\n");

        int score1 = 0;
        int score2 = 0;

        if (levelintro_sfc != nullptr)
            SDL_FreeSurface(levelintro_sfc);

        Sound_music_volume(MIX_MAX_VOLUME);
        if (current_level > start_level && game != 0)
        {
            List<int> l;
            game->get_scores(&l);
            score1 = *l[0];
            if (n_players > 1 && l.Length() > 1)
                score2 = *l[1];
        }
        else
        {
            Sound_create_music("sound/start", 0);
        }

        if (current_level > nlevels)
        {
            current_level = 1;
            game_mode = std::min<int>(game_mode + 1, 3);
        }

        if (scoreboard2_sfc != nullptr)
        {
            SDL_FreeSurface(scoreboard2_sfc);
        }
        if (current_level < 6)
        {
            scoreboard2_sfc = loadImage(n_players > 1 ? "graphics/s_board12p.bmp" : "graphics/s_board11p.bmp");
        }
        else
        {
            scoreboard2_sfc = loadImage(n_players > 1 ? "graphics/s_board22p.bmp" : "graphics/s_board21p.bmp");
        }

        desired_scoreboard_x = SCREEN_X - scoreboard_sfc->w;
        if (n_players > 1)
            desired_scoreboard_x = SCREEN_X - 128;
        else
            desired_scoreboard_x = SCREEN_X - 144;

        const char* Stages[] =
        {
            "graphics/stage1.jpg",
            "graphics/stage2.jpg",
            "graphics/stage3.jpg",
            "graphics/stage4.jpg",
            "graphics/stage5.jpg",
            "graphics/stage6.jpg",
        };
        levelintro_sfc = loadImage(Stages[current_level - 1]);

        /* Create the game: */
        delete game;
        if (n_players == 1)
        {
            game = new CGame(maps[current_level - 1], game_mode, left_key, right_key, fire_key, score1, current_level, game_remake_extras);
        }
        else
        {
            game = new CGame(maps[current_level - 1], game_mode, left_key, right_key, fire_key,
                             left2_key, right2_key, fire2_key, score1, score2, current_level, game_remake_extras);
        }
        interlevel_state = 0;
        interlevel_timmer = 0;
        if (current_level == 1)
            interlevel_state = 3;

        output_debug_message("CRoadFighter::interlevel_cycle: game created.\n");
    }

    switch (interlevel_state)
    {
    case 0: /* Appearing map */
        if (state_timmer >= interlevel_time)
        {
            output_debug_message("CRoadFighter::interlevel_cycle: going to state 1.\n");
            interlevel_state = 1;
            interlevel_timmer = 0;
        }
        else
        {
            interlevel_timmer++;
        }
        break;
    case 1: /* Advancing car */
        if (state_timmer >= interlevel_time * 4 || (interlevel_timmer >= (interlevel_time * 2) && ((keyboard[fire_key] && !old_keyboard[fire_key]) || (keyboard[SDLK_ESCAPE] && !old_keyboard[SDLK_ESCAPE]))))
        {
            output_debug_message("CRoadFighter::interlevel_cycle: going to state 2.\n");
            interlevel_state = 2;
            if (interlevel_timmer >= interlevel_time)
                interlevel_timmer = interlevel_time;
        }
        else
        {
            interlevel_timmer++;
        }
        break;
    case 2: /* Disappearing map */
        if (interlevel_timmer <= 0)
        {
            output_debug_message("CRoadFighter::interlevel_cycle: going to state 3.\n");
            interlevel_state = 3;
            interlevel_timmer = 0;
        }
        else
        {
            interlevel_timmer--;
        }
        break;
    case 3: /* Appearing text */
        if (interlevel_timmer >= interlevel_time * 5 || (interlevel_timmer >= interlevel_time && ((keyboard[fire_key] && !old_keyboard[fire_key]) || (keyboard[SDLK_ESCAPE] && !old_keyboard[SDLK_ESCAPE]))))
        {
            output_debug_message("CRoadFighter::interlevel_cycle: going to state 4.\n");
            interlevel_state = 4;
            if (interlevel_timmer >= interlevel_time)
                interlevel_timmer = interlevel_time;
        }
        else
        {
            interlevel_timmer++;
        }
        break;
    case 4: /* Disappearing text */
        Sound_music_volume((interlevel_timmer * MIX_MAX_VOLUME) / interlevel_time);
        if (interlevel_timmer <= 0)
        {
            output_debug_message("CRoadFighter::interlevel_cycle: going to playing state.\n");
            Sound_release_music();
            Sound_music_volume(MIX_MAX_VOLUME);
            playing_reachedend = false;
            return PLAYING_STATE;
        }
        else
            interlevel_timmer--;
        break;
    }

    return INTERLEVEL_STATE;
}

void CRoadFighter::interlevel_draw(SDL_Surface* screen)
{
    SDL_Rect r;

    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

    switch (interlevel_state)
    {
    case 0: // Appearing map
    {
        float f = float(interlevel_timmer) / float(interlevel_time);
        r.x = (desired_scoreboard_x) / 2 - (gamemap_sfc->w) / 2;
        r.y = (screen->h) / 2 - (gamemap_sfc->h) / 2;
        r.w = gamemap_sfc->w;
        r.h = gamemap_sfc->h;
        SDL_BlitSurface(gamemap_sfc, 0, screen, &r);

        // Draw the car:
        if (((interlevel_timmer >> 3) & 0x01) != 0)
        {
            int car_x = (desired_scoreboard_x) / 2 - minicar1_tile->get_dx() / 2;
            int car_y = (screen->h) / 2 + (gamemap_sfc->h) / 2 - (current_level - 2) * ((gamemap_sfc->h) / 6) - minicar1_tile->get_dy() / 2;

            minicar1_tile->draw(car_x, car_y, screen);
        }

        if (f < 1.0)
            surface_fader(screen, f, f, f, 0);
    }
    break;
    case 1: // Advancing car
        r.x = (desired_scoreboard_x) / 2 - (gamemap_sfc->w) / 2;
        r.y = (screen->h) / 2 - (gamemap_sfc->h) / 2;
        r.w = gamemap_sfc->w;
        r.h = gamemap_sfc->h;
        SDL_BlitSurface(gamemap_sfc, 0, screen, &r);
        // Draw the car:
        if (((interlevel_timmer >> 3) & 0x01) != 0)
        {
            int car_x = (desired_scoreboard_x) / 2 - minicar1_tile->get_dx() / 2;
            int car_y = (screen->h) / 2 + (gamemap_sfc->h) / 2 - (current_level - 2) * ((gamemap_sfc->h) / 6) - minicar1_tile->get_dy() / 2;
            int v = interlevel_timmer;

            if (v >= interlevel_time * 2)
                v = interlevel_time * 2;

            car_y -= (v * ((gamemap_sfc->h) / 6)) / (interlevel_time * 2);
            minicar1_tile->draw(car_x, car_y, screen);
        }
        break;
    case 2: // Disappearing map
    {
        float f = float(interlevel_timmer) / float(interlevel_time);
        r.x = (desired_scoreboard_x) / 2 - (gamemap_sfc->w) / 2;
        r.y = (screen->h) / 2 - (gamemap_sfc->h) / 2;
        r.w = gamemap_sfc->w;
        r.h = gamemap_sfc->h;
        SDL_BlitSurface(gamemap_sfc, 0, screen, &r);

        // Draw the car:
        if (((interlevel_timmer >> 3) & 0x01) != 0)
        {
            int car_x = (desired_scoreboard_x) / 2 - minicar1_tile->get_dx() / 2;
            int car_y = (screen->h) / 2 + (gamemap_sfc->h) / 2 - (current_level - 1) * ((gamemap_sfc->h) / 6) - minicar1_tile->get_dy() / 2;

            minicar1_tile->draw(car_x, car_y, screen);
        }

        if (f < 1.0)
            surface_fader(screen, f, f, f, 0);
    }
    break;

    case 3: // Appearing text
    case 4: // Disappearing text
    {
        SDL_Color c;
        SDL_Surface* text_sfc;
        char text[80];
        int v = 255;
        float f = 1.0;

        if (levelintro_sfc != 0)
        {
            text_sfc = SDL_DisplayFormat(levelintro_sfc);

            f = float(interlevel_timmer) / interlevel_time;
            if (f >= 1.0)
                f = 1.0;

            if (f < 1.0)
                surface_fader(text_sfc, f, f, f, 0);
            r.x = (desired_scoreboard_x) / 2 - text_sfc->w / 2;
            r.y = (screen->h / 2) - text_sfc->h;
            r.w = text_sfc->w;
            r.h = text_sfc->h;
            SDL_BlitSurface(text_sfc, 0, screen, &r);
            SDL_FreeSurface(text_sfc);
        }
        else
        {
            sprintf(text, "STAGE %.2i", current_level);

            v = (interlevel_timmer * 255) / interlevel_time;
            if (v >= 255)
                v = 255;
            c.r = c.g = c.b = v;
            text_sfc = TTF_RenderText_Blended(font2big, text, c);
            r.x = (desired_scoreboard_x) / 2 - text_sfc->w / 2;
            r.y = (screen->h / 2) - text_sfc->h;
            r.w = text_sfc->w;
            r.h = text_sfc->h;
            SDL_BlitSurface(text_sfc, 0, screen, &r);
            SDL_FreeSurface(text_sfc);
        } // if
    }
    break;
    } // switch

    /* Draw Scoreboard: */
    if (scoreboard_x == -1)
    {
        scoreboard_x = screen->w;
    }
    else
    {
        if (scoreboard_x > desired_scoreboard_x)
        {
            scoreboard_x -= int((screen->w - desired_scoreboard_x) / interlevel_time);
            if (scoreboard_fade_timmer < 25)
                scoreboard_fade_timmer++;
        }
        if (scoreboard_x < desired_scoreboard_x)
            scoreboard_x = desired_scoreboard_x;
    }
    scoreboard_draw(scoreboard_x, 0, screen);
}
