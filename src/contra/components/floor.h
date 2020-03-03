//
// Created by david on 5/2/20.
//

#ifndef CONTRA_FLOOR_H
#define CONTRA_FLOOR_H

#include <SDL_image.h>
#include <memory>

class Floor {
private:
    enum FloorPixel {
        AIR,
        FLOOR,
        FLOOR_NO_FALL,
        WATER
    };
    std::unique_ptr<FloorPixel> m_map;
    int m_floorWidth, m_floorHeight;
public:
    SDL_Surface *surface;

    Floor(const char *path) {
        surface = IMG_Load(path);
        SDL_PixelFormat *fmt = surface->format;

        m_floorWidth = surface->w;
        m_floorHeight = surface->h;
        int map_size = m_floorHeight * m_floorWidth;
        m_map.reset(new FloorPixel[map_size]);

        if (fmt->BitsPerPixel != 8) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading floor mask: not an 8-bit surface.\n");
            return;
        }

        // Lets loop over the surface pixels, black is air, white is sea, gray is whater
        SDL_LockSurface(surface);
        auto *pixel = (Uint8 *) surface->pixels;
        auto *map = m_map.get();
        int x = 0;
        for (auto *mappedPixel = m_map.get(); mappedPixel < map + map_size; mappedPixel++, pixel++, x++) {
            // We need to pay attention to this, it seems that SDL saves an extra pixel per line for some reason(?)
            if (x == m_floorWidth) {
                x = -1;
                mappedPixel--;
                continue;
            }
            auto color = &fmt->palette->colors[*pixel];
            if (color->r < 100) {
                *mappedPixel = AIR;
            } else if (color->r < 150) {
                *mappedPixel = WATER;
            } else if (color->r < 200) {
                *mappedPixel = FLOOR_NO_FALL;
            } else {
                *mappedPixel = FLOOR;
            }
        }
        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
    }

    int getWidth() { return m_floorWidth; }

    int getHeight() { return m_floorHeight; }

    bool IsFloor(int x, int y) {
        auto value = GetFloorPixel(x, y);
        return value == FLOOR || value == FLOOR_NO_FALL;
    }

    bool ShouldBeAbleToFall(int x, int y) {
        return GetFloorPixel(x, y) == FLOOR;
    }

    bool IsWater(int x, int y) {
        return GetFloorPixel(x, y) == WATER;
    }

    bool IsFloorOrWater(int x, int y) {
        auto value = GetFloorPixel(x, y);
        return value == FLOOR || value == WATER || value == FLOOR_NO_FALL;
    }

    void SetAir(int x0, int y0, int width, int height) {
        for (int y = y0; y < y0 + height; y++) {
            for (int x = x0; x < x0 + width; x++) {
                m_map.get()[y * m_floorWidth + x] = AIR;
            }
        }
    }
private:
    FloorPixel GetFloorPixel(int x, int y) {
        x = std::max(std::min(x, m_floorWidth), 0);
        y = std::max(std::min(y, m_floorHeight), 0);
        return m_map.get()[y * m_floorWidth + x];
    }
};

#endif //CONTRA_FLOOR_H
