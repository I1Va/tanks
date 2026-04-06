#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdexcept>
#include <source_location>

struct SDLException : std::runtime_error {
    SDLException(std::string msg=SDL_GetError(),
                 const std::source_location& loc = std::source_location::current())
      : std::runtime_error(format(msg, loc)) {}

private:
    static std::string format(const std::string& m, const std::source_location& loc) {
        return std::string(loc.file_name()) + ":" +
               std::to_string(loc.line()) + " (" +
               loc.function_name() + "): " + m;
    }
};

struct TTFException : std::runtime_error {
    TTFException(std::string msg = TTF_GetError(),
                 const std::source_location& loc = std::source_location::current())
      : std::runtime_error(format(msg, loc)) {}

private:
    static std::string format(const std::string& m, const std::source_location& loc) {
        return std::string(loc.file_name()) + ":" +
               std::to_string(loc.line()) + " (" +
               loc.function_name() + "): " + m;
    }
};

[[noreturn]]
inline void throw_invalid_argument(
    const std::string& msg,
    std::source_location loc = std::source_location::current())
{
    throw std::invalid_argument(
        msg + " (" +
        loc.file_name() + ":" +
        std::to_string(loc.line()) + " in " +
        loc.function_name() + ")"
    );
}

inline void requireTTFCondition(bool cond, std::string msg = TTF_GetError(), std::source_location loc = std::source_location::current()) {
    if (!(cond)) throw TTFException(msg, loc); 
}

inline void requireSDLCondition(bool cond, std::string msg = SDL_GetError(), std::source_location loc = std::source_location::current()) {
    if (!(cond)) throw SDLException(msg, loc); 
}