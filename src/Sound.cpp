// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#include "Sound.hpp"

SoundFX::SoundFX()
{
}

SoundFX::~SoundFX(){

    for (auto it=fxMap.begin(); it!=fxMap.end(); ++it){
        Mix_FreeChunk(it->second);
    }

    Mix_CloseAudio();
    SDL_Quit();
}

void SoundFX::initSDL(){
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 ){
        std::cerr << "ERROR initializing SDL subsystems\n";
        return;
    }

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ){
        std::cerr << "ERROR initializing SDL mixer\n";
        return;
    }

}

void SoundFX::loadFile(std::string path, std::string name){

    Mix_Chunk* fx = Mix_LoadWAV (path.c_str());
    if (fx == NULL){
        std::cerr << "ERROR loading " << path << std::endl;
        std::cerr << Mix_GetError() << std::endl;
    }

    fxMap[name] = fx;
}

void SoundFX::playSound(std::string name){
    if (Mix_PlayChannel(-1, fxMap[name], 0) == -1){
        std::cerr << "ERROR playing " << name << std::endl;

    }
}
