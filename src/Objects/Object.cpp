#include "Object.h"
#include "Tile.h"

#include <SDL_mixer.h>
#include <SDL_ttf.h>

CObject::CObject()
    : state(0)
    , constitution(CONSTITUTION_NONE)
    , game(nullptr)
    , tiles(nullptr)
    , ntiles(0)
    , tile(0)
    , x(0)
    , y(0)
    , draw_x(0)
    , draw_y(0)
{
}

CObject::CObject(int nx, int ny, CTile* t, int cons, CGame* g)
    : state(0)
    , constitution(cons)
    , game(g)
    , tiles(new CTile*[1])
    , ntiles(1)
    , tile(0)
    , x(nx)
    , y(ny)
    , draw_x(nx)
    , draw_y(ny)
{
    tiles[0] = t;
}

CObject::~CObject(void)
{
    for (int i = 0; i < ntiles; i++)
        tiles[i] = nullptr;
    delete[] tiles;
}

bool CObject::cycle(unsigned char* /*keyboard*/, unsigned char* /*old_keyboard*/)
{
    return true;
}

void CObject::draw(int sx, int sy, SDL_Surface* screen)
{
    draw_x = x;
    draw_y = y;
    if (tile >= 0 && tile < ntiles)
    {
        tiles[tile]->draw(x - sx, y - sy, screen);
    }
}

bool CObject::collision(int offsx, int offsy, CObject* o)
{
    CTile* t1 = nullptr;
    CTile* t2 = nullptr;

    if (tile >= 0 && tile < ntiles)
        t1 = tiles[tile];
    if (o->tile >= 0 && o->tile < o->ntiles)
        t2 = o->tiles[o->tile];

    if (t1 == nullptr || t2 == nullptr)
        return false;
    if (t1->collision_data == nullptr || t2->collision_data == nullptr)
        return false;

    if (draw_y + t1->get_dy() < o->draw_y + offsy || o->draw_y + offsy + t2->get_dy() < draw_y)
        return false;

    {
        int tmpx = int((o->draw_x + offsx) - draw_x);
        int tmpy = int((o->draw_y + offsy) - draw_y);

        if (sge_cmcheck(t1->collision_data, 0, 0, t2->collision_data, tmpx, tmpy))
        {
            // tmpx = sge_get_cx();
            // tmpy = sge_get_cy();
            return true;
        }
    }

    return false;
}

int CObject::get_dx() const
{
    if (tile >= 0 && tile < ntiles)
        return tiles[tile]->get_dx();

    return 0;
}

int CObject::get_dy() const
{
    if (tile >= 0 && tile < ntiles)
        return tiles[tile]->get_dy();

    return 0;
}

bool CObject::constitution_test(int cons) const
{
    if ((constitution & cons) != 0)
        return true;

    return false;
}
