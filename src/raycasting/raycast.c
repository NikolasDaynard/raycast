#include "raycast.h"

bool pointInRect(SDL_FRect rect, SDL_FPoint point) {
    return (rect.x + rect.w > point.x) && (point.x > rect.x)
        && (rect.y + rect.h > point.y) && (point.y > rect.y);
}

SDL_FPoint raycastDir(SDL_FPoint point, SDL_FPoint direction) {
    // foo geometry
    SDL_FRect rect;
    rect.x = .2;
    rect.y = .2;
    rect.w = .4;
    rect.h = .1;

    for (int i = 0; i < 1; i++) {
        SDL_FPoint newPoint = point;
        newPoint.x += direction.x * i;
        newPoint.y += direction.y * i;
        if (pointInRect(rect, newPoint)) {
            return newPoint;
        }   
    }

    //oob
    SDL_FPoint oob;
    oob.x = -1.0;
    oob.y = -1.0;

    return oob;
}