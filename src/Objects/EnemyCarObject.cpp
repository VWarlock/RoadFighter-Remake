#include "EnemyCarObject.h"
#include "Auxiliar.h"
#include "ExplosionObject.h"
#include "Game.h"

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

#include <algorithm>

extern int ENEMY_SPEED;
extern int ENEMY_HSPEED;
extern int PLAYING_WINDOW;

CEnemyCarObject::CEnemyCarObject(int nx, int ny, CTile* t, int start_delay, CGame* g)
    : CCarObject(nx, ny, t, g)
{
    state_timmer = start_delay;

    following_right_border = true;
    distance_to_border = -1;
    last_collision = 0;

    slide_timmer = 0;
}

CEnemyCarObject::~CEnemyCarObject()
{
}

bool CEnemyCarObject::cycle(unsigned char* keyboard, unsigned char* old_keyboard)
{
    CCarObject::cycle(keyboard, old_keyboard);

    if (state_timmer > 0)
        state_timmer--;
    if (state == 0 && state_timmer == 0)
        state = 1;

    if (last_collision != 0)
    {
        if (last_collision->get_x() < x)
            slide_direction = 1;
        else
            slide_direction = -1;
        slide_speed = std::min(last_collision->get_y_speed(), y_speed);
        slide_timmer = 8;

        last_collision = 0;
    }

    if (state != 0)
    {
        y_speed = -ENEMY_SPEED;
        x_speed = 0;
        if (game->object_collision(8, 0, this, CONSTITUTION_SOLID) != 0)
        {
            x_speed = -ENEMY_HSPEED;
            slide_timmer = 0;
        }
        else
        {
            if (game->object_collision(-8, 0, this, CONSTITUTION_SOLID) != 0)
            {
                x_speed = ENEMY_HSPEED;
                slide_timmer = 0;
            }
        }

        if (slide_timmer > 0)
        {
            slide_timmer--;
            if (slide_direction == 1)
                x_speed = int(ENEMY_HSPEED);
            if (slide_direction == -1)
                x_speed = -int(ENEMY_HSPEED);
            y_speed = slide_speed;
        }

        if (x_speed == 0 && distance_to_border != -1)
        {
            int i;
            int distance = -1;
            for (i = 0; distance == -1 && i <= distance_to_border + 8; i += 8)
            {
                if (following_right_border)
                {
                    if (game->object_collision(i, 0, this, CONSTITUTION_SOLID) != 0)
                        distance = i - 8;
                }
                else
                {
                    if (game->object_collision(-i, 0, this, CONSTITUTION_SOLID) != 0)
                        distance = i - 8;
                }
            }
            if (distance == -1)
                distance = distance_to_border + 8;
            if (following_right_border)
            {
                if (distance <= distance_to_border - 8)
                    x_speed = -ENEMY_HSPEED;
                if (distance >= distance_to_border + 8)
                    x_speed = ENEMY_HSPEED;
            }
            else
            {
                if (distance <= distance_to_border - 8)
                    x_speed = ENEMY_HSPEED;
                if (distance >= distance_to_border + 8)
                    x_speed = -ENEMY_HSPEED;
            }
        }

        {
            CObject* o = 0;

            o = game->object_collision(0, 0, this, CONSTITUTION_SOLID);
            if (o != 0 && o->constitution_test(CONSTITUTION_CAR))
            {
                CObject* o = new CExplosionObject(x - 16, y - 32, &game->explosion_tiles, 0, 11, game);
                game->objects.Add(o);
                return false;
            }
            else
            {
                /* Car collision: */
                o = game->object_collision(0, 0, this, CONSTITUTION_CAR);
                if (o != 0)
                {
                    CCarObject* co = (CCarObject*)o;

                    co->car_collision(this);
                    car_collision(co);
                }
            }
        }
    }
    else
    {
        y_speed = 0;
    }

    if (game->min_distance_to_players(y) > PLAYING_WINDOW)
        return false;

    return true;
}
