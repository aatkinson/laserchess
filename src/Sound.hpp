// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#ifndef SOUND_HPP
#define SOUND_HPP

#include <iostream>
#include <map>
#include <string>
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

class SoundFX {
public:
    SoundFX();
    ~SoundFX();
    void initSDL();
    void loadFile(std::string path, std::string name);
    void playSound(std::string name);
private:
    std::map<std::string, Mix_Chunk*> fxMap;
};

#endif
