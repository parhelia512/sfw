#ifndef GAME_APPLICATION_H
#define GAME_APPLICATION_H

#include "render_core/application.h"

#include "game_scene.h"

class GameApplication : public Application {
public:
    GameApplication() : Application() {
        scene = new GameScene();
    }

    ~GameApplication() {
        delete scene;
    }
};


#endif // GAME_APPLICATION_H
