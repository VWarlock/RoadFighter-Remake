#pragma once

#include "Sound.h"

#include <SDL/SDL.h>
#include <cstdio>

#define QUIT_STATE -1
#define KONAMI_STATE 0
#define MENU_STATE 1
#define DEMO_STATE 2
#define PLAYING_STATE 3
#define INTERLEVEL_STATE 4
#define GAMEOVER_STATE 5

#define PRESENTATION_STATE 6

#define FONT_SIZE 24

class CGame;
class CTile;

struct Mix_Chunk;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

class CRoadFighter final
{
public:
    CRoadFighter();
    ~CRoadFighter();

    bool cycle();
    void draw(SDL_Surface* screen);

    /* Individual state functions: */
    int presentation_cycle();
    void presentation_draw(SDL_Surface* screen);

    int konami_cycle();
    void konami_draw(SDL_Surface* screen);

    int menu_cycle();
    void menu_draw(SDL_Surface* screen);

    int interlevel_cycle();
    void interlevel_draw(SDL_Surface* screen);

    int playing_cycle();
    void playing_draw(SDL_Surface* screen);

    int gameover_cycle();
    void gameover_draw(SDL_Surface* screen);

    void load_configuration();
    void save_configuration();
    void default_configuration();

private:
    void scoreboard_draw(int x, int y, SDL_Surface* screen);

private:
    int state, state_timmer;
    int current_level;
    int high_score;
    CGame* game;

    /* keyboard: */
    int left_key, right_key, fire_key;
    int left2_key, right2_key, fire2_key;

    /* Graphic data: */
    SDL_Surface* disclaimer_sfc;
    SDL_Surface* retroremakes_sfc;
    SDL_Surface* konami1_sfc;
    SDL_Surface* konami2_sfc;
    SDL_Surface* tittle_sfc;
    SDL_Surface* arrow_sfc;
    SDL_Surface* scoreboard_sfc;
    SDL_Surface* scoreboard2_sfc;
    SDL_Surface* scoreboardleft_sfc;
    SDL_Surface* gamemap_sfc;
    SDL_Surface* minicar1_sfc;
    SDL_Surface* minicar2_sfc;
    SDL_Surface* levelintro_sfc;
    SDL_Surface* gameover_sfc;
    SDL_Surface* credits_sfc;
    SDL_Surface* credits2_sfc;
    SDL_Surface* wild_spike_sfc;
    CTile* minicar1_tile;
    CTile* minicar2_tile;

    /* Sound data: */
    Mix_Chunk* S_menu_move;
    Mix_Chunk* S_menu_select;
    Mix_Chunk* S_menu_in;
    Mix_Chunk* S_menu_out;
    // Mix_Chunk* M_menu;
    // Mix_Chunk* M_ingame;
    // Mix_Chunk* M_start;
    // Mix_Chunk* M_gameover;
    // Mix_Chunk* M_levelcomplete;

    /* Individual state data: */
    int presentation_state;
    int presentation_timmer;

    int konami_state;
    int konami_timmer;

    int menu_current_menu;
    int menu_state;
    int menu_effect;
    int menu_timmer;
    char* menu_tittle_text;
    char* menu_options_text;
    int menu_nitems;
    int menu_item;
    int menu_redefining_key;
    int menu_credits_timmer;

    int game_mode;
    int n_players;
    bool game_remake_extras;

    int interlevel_state;
    int interlevel_timmer;
    int scoreboard_x, desired_scoreboard_x;
    int scoreboard_fade_timmer;

    int gameover_state;
    int gameover_timmer;

    int endsequence_state;
    int endsequence_timmer;

    bool playing_reachedend;

    unsigned char* keyboard;
    unsigned char old_keyboard[SDLK_LAST];

    TTF_Font* font1;
    TTF_Font* font2big;
    TTF_Font* font2medium;
    TTF_Font* font2small;

    /* REPLAYS: */
    FILE* replay_fp;
};
