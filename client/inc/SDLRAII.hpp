#pragma once
#include <memory>
#include <cassert>
#include <stdexcept>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace client::raii
{

struct SDLWindowDeleter   { void operator()(::SDL_Window   *p) const noexcept { if (p) SDL_DestroyWindow(p); } };
struct SDLRendererDeleter { void operator()(::SDL_Renderer *p) const noexcept { if (p) SDL_DestroyRenderer(p); } };
struct SDLSurfaceDeleter  { void operator()(::SDL_Surface  *p) const noexcept { if (p) SDL_FreeSurface(p); } };
struct SDLTextureDeleter  { void operator()(::SDL_Texture  *p) const noexcept { if (p) SDL_DestroyTexture(p); } };
struct TTFFontDeleter     { void operator()(::TTF_Font     *p) const noexcept { if (p) TTF_CloseFont(p); }};
struct SDL_RWopsDeleter   { void operator()(::SDL_RWops    *p) const noexcept { if (p) SDL_RWclose(p); }}; // !!!!!!! `0 on success or a negative error code on failure; call SDL_GetError() for more information.`

using SDL_Window   = std::unique_ptr<::SDL_Window,  SDLWindowDeleter>;
using SDL_Renderer = std::unique_ptr<::SDL_Renderer,SDLRendererDeleter>;
using SDL_Surface  = std::unique_ptr<::SDL_Surface, SDLSurfaceDeleter>;
using SDL_Texture  = std::unique_ptr<::SDL_Texture, SDLTextureDeleter>;
using TTF_Font     = std::unique_ptr<::TTF_Font,    TTFFontDeleter>;
using SDL_RWops    = std::unique_ptr<::SDL_RWops,   SDL_RWopsDeleter>;


SDL_Window   SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);
SDL_Renderer SDL_CreateRenderer(const SDL_Window &w, int index, Uint32 flags);
SDL_Texture  SDL_CreateTexture(const SDL_Renderer &renderer, Uint32 format, int access, int w, int h);
SDL_Surface  TTF_RenderUTF8_Blended(const TTF_Font &font, const char* text, SDL_Color color);
SDL_Texture  SDL_CreateTextureFromSurface(const SDL_Renderer &renderer, const SDL_Surface &surface);
SDL_Surface  SDL_CreateRGBSurfaceWithFormat(int flags, int width, int height, int depth, Uint32 format);

TTF_Font TTF_OpenFont(const char* file, int ptsize);
TTF_Font TTF_OpenFontRW(::SDL_RWops* src, int freesrc, int ptsize);

SDL_RWops SDL_RWFromConstMem(const void* buffer, size_t size);

}