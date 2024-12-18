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

    for (int i = 0; i < 1000; i++) {
        SDL_FPoint newPoint = point;
        newPoint.x += direction.x * i;
        newPoint.y += direction.y * i;
        if (newPoint.x > 1.0 || newPoint.x < 0.0 ||
            newPoint.y > 1.0 || newPoint.y < 0.0) {
            break;
        }
        // printf("new pointy %f,%f\n", newPoint.x, newPoint.y);
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