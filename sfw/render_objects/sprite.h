#ifndef SPRITE_H
#define SPRITE_H

#include "object_2d.h"

#include "transform_2d.h"
#include "mesh_instance_2d.h"

class Sprite : public Object2D {
public:
    void render();
    void update_mesh();

    Sprite();
    ~Sprite();

    Transform2D transform;

    MeshInstance2D *mesh_instance;

    float width;
    float height;

    float region_x;
    float region_y;
    float region_width;
    float region_height;
};


#endif // SPRITE_H