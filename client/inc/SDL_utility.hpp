#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <cassert>

#include "err_proc.hpp"

SDL_Surface *loadSurface(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    return surface;
}

SDL_Texture *surfaceToTexture(SDL_Renderer *renderer, SDL_Surface *surface) {
    assert(renderer);
    assert(surface);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    return texture;
}

SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string& path) {
    SDL_Surface *surface = loadSurface(path);
    if (!surface) return nullptr;
    return surfaceToTexture(renderer, surface);
}
