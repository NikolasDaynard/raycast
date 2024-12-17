#pragma once
#include <SDL3/SDL.h>

/**
 * Raycasts in the given angle
 * Returns first point of collision
 */
SDL_FPoint raycastDir(SDL_FPoint point, SDL_FPoint direction);