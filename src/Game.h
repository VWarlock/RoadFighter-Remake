#pragma once

#include "List.h"
#include "Sound.h"
#include "Tile.h"

#include <SDL.h>
#include <SDL_ttf.h>

#define SEMAPHORE_TIME 25

#define QUICK_PARTS 256
#define BIGGEST_OBJECT 384 /* This is used to compute the Quick parts */

class CCarObject;
class CObject;

struct Mix_Chunk;

class CTyreMark
{
public:
    long int x, y, x2, y2;
};

class CGame final
{
    friend class CPlayerCarObject;
    friend class CEnemyCarObject;
    friend class CEnemyRacerCarObject;
    friend class CEnemyFastCarObject;
    friend class CEnemySlidderCarObject;
    friend class CEnemyTruckObject;
    friend class CFuelObject;
    friend class CSemaphoreObject;

    friend class CRoadFighter;

public:
    CGame(const char* mapname, int mode, int left_key, int right_key, int fire_key, int score, int current_level, bool extras);
    CGame(const char* mapname, int mode,
          int left_key1, int right_key1, int fire_key1,
          int left_key2, int right_key2, int fire_key2,
          int score1, int score2, int current_level, bool extras);
    ~CGame();

    bool cycle(unsigned char* keyboard, unsigned char* old_keyboard);
    void draw(SDL_Surface* sfc, SDL_Rect vp);

    bool level_completed() const;

    void get_speeds(List<int>* l) const;
    void get_fuels(List<int>* l) const;
    void get_positions(List<float>* l) const;
    void get_scores(List<int>* l) const;
    int get_game_timmer() const
    {
        return game_timmer;
    };

    CObject* object_collision(int xoffs, int yoffs, CObject* o, int constitution);

    int get_dx() const
    {
        return dx;
    };

    int get_dy() const
    {
        return dy;
    };

    bool esc_pressed;
    bool backspace_pressed;

private:
    void init_game(const char* mapname);

    bool load_map(const char* mapname);
    void draw(SDL_Surface* sfc, SDL_Rect vp, CCarObject* focusing, float* sy, int* cp_delay);

    void init_quick_tables();
    void delete_quick_tables();
    void get_quick_min_max(CObject* o, long yoffs, int* min, int* max);
    void get_quick_min_max(long ymin, long ymax, int* min, int* max);

    CObject* add_enemy_car(int type, long y);
    CObject* add_obstacle(int type, long y);
    long min_distance_to_players(long y);
    long min_distance_to_car(long y);
    CObject* find_closest_player(long x, long y);
    long min_distance_to_other_players(long y, CObject* player);
    bool first_player(CObject* player);

    int dx;
    int dy;
    int game_state; /* 0: appearing & playing, 1: disappearing */
    int game_timmer;
    int game_mode;
    int current_level;
    bool game_remake_extras;
    bool paused;

    /* Graphics: */
    List<TILE_SOURCE> tile_sources;
    List<CTile> tiles[256];

    SDL_Surface *player1_car, *player2_car, *enemy_cars, *fuel_sfc;
    SDL_Surface *empty_sfc, *fuelscores_sfc, *obstacles_sfc;
    SDL_Surface /**start_sfc,*/ *checkpoint_sfc, *goal_sfc;
    SDL_Surface *pause_sfc, *explosion_sfc;

    List<CTile> player_tiles;
    List<CTile> enemy_tiles;
    List<CTile> extra_tiles;
    List<CTile> explosion_tiles;

    TTF_Font* font;

    /* Sound data: */
    Mix_Chunk* S_takefuel;
    Mix_Chunk* S_redlight;
    Mix_Chunk* S_greenlight;
    Mix_Chunk* S_crash;
    Mix_Chunk* S_water;
    Mix_Chunk* S_carstart;
    Mix_Chunk* S_fuelempty;
    Mix_Chunk* S_caradvance;
    Mix_Chunk* S_collision;
    Mix_Chunk* S_truck;
    Mix_Chunk* S_carengine;
    Mix_Chunk* S_carskid;

    /* Background: */
    List<CObject> background;
    List<CObject> middleground;
    List<CObject> foreground;

    /* Tyre marks: */
    List<CTyreMark> tyre_marks;

    /* Quick back and fore access: */
    List<CObject> quick_background[QUICK_PARTS];
    List<CObject> quick_middleground[QUICK_PARTS];
    List<CObject> quick_foreground[QUICK_PARTS];

    /* Weather: */

    /* Objects: */
    List<CObject> objects, todelete;

    /* Focusing objects: */
    List<CObject> focusing_objects;
    List<int> checkpoint_delay;
    List<float> focusing_fy;
    List<int> focusing_next_car;
    List<int> focusing_next_car_index;

    int start_delay;  /* Delay due to a semaphore, etc. at the start of the game */
    int start_delay2; /* Delay that the music has to wait before starting (due to semaphores) */
                      /* It is also used for the START logo	*/
    int fastcar_counter;
};
