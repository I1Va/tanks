#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <cassert>

#include "err_proc.hpp"

SDL_Surface *load_surface(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    return surface;
}

SDL_Texture *surface_to_texture(SDL_Renderer *renderer, SDL_Surface *surface) {
    assert(renderer);
    assert(surface);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    return texture;
}

SDL_Texture *load_texture(SDL_Renderer *renderer, const std::string& path) {
    SDL_Surface *surface = load_surface(path);
    if (!surface) return nullptr;
    return surface_to_texture(renderer, surface);
}

void draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, int w, int h) {
    SDL_Rect dstRect = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
}
