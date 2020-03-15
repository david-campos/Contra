#pragma once

#ifndef CONTRA_AVANCEZLIB_H
#define CONTRA_AVANCEZLIB_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <functional>
#include <set>

void channel_finished_callback(int channel);

class Sprite {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
public:

    Sprite(SDL_Renderer *renderer, SDL_Texture *texture);

    // Destroys the sprite instance
    ~Sprite();

    [[nodiscard]] int getWidth() const;

    // Draw the sprite at the given position.
    void draw(int x, int y);

    // Draw a part of the sprite at the given position,
    // the params sx, sy, sw, sh allow to define the part
    // of the sprite to draw
    void draw(int x, int y, int tw, int th, int sx, int sy, int sw, int sh, bool mirrorHorizontal = false);
};

/**
 * The Music class encapsulates the background music. Calling Play after music loading error does
 * not cause any crashing or other errors.
 */
class Music final {
public:
    Music(Mix_Music *mMusic) : mMusic(mMusic) {}

    void Play(short times = 0) {
        if (mMusic) {
            Mix_HaltMusic();
            Mix_PlayMusic(mMusic, times - 1);
        }
    }

    ~Music() {
        if (mMusic)
            Mix_FreeMusic(mMusic);
        mMusic = nullptr;
    }

private:
    Mix_Music *mMusic = nullptr;
};

/**
 * The SoundEffect class encapsulates short sound effects playing. Calling Play after sound loading error does
 * not cause any crashing or other errors.
 */
class SoundEffect final {
public:
    SoundEffect(Mix_Chunk *effect) : effect(effect) {}

    /**
     * Plays the sound and returns a callback which will stop the sound in case it is
     * still playing when the callback is called. Set to 0 times to loop forever.
     * If the current number of channels is not enough to support one sound
     * more playing, new channels will be allocated.
     *
     * @return A callback to Stop the channel (if it is still playing)
     */
    std::function<void()> Play(short times = 1);

    ~SoundEffect() {
        if (effect)
            Mix_FreeChunk(effect);
    }

private:
    Mix_Chunk *effect;
};


class AvancezLib {
public:
    enum TextAlign {
        TEXT_ALIGN_LEFT_TOP,
        TEXT_ALIGN_LEFT_MIDDLE,
        TEXT_ALIGN_LEFT_BOTTOM,
        TEXT_ALIGN_CENTER_TOP,
        TEXT_ALIGN_CENTER_MIDDLE,
        TEXT_ALIGN_CENTER_BOTTOM,
        TEXT_ALIGN_RIGHT_TOP,
        TEXT_ALIGN_RIGHT_MIDDLE,
        TEXT_ALIGN_RIGHT_BOTTOM
    };

    // Destroys the avancez library instance
    void destroy();

    // Destroys the avancez library instance and exits
    void quit();

    // Creates the main window. Returns true on success.
    bool init(int width, int height);

    // Clears the screen and draws all sprites and texts which have been drawn
    // since the last update call.
    // If update returns false, the application should terminate.
    void processInput();

    void swapBuffers();

    void clearWindow();

    // Create a sprite given a string.
    // All sprites are 32*32 pixels.
    Sprite *createSprite(const char *name);

    Music *createMusic(const char *path);

    SoundEffect *createSound(const char *path);

    bool isMusicPlaying() { return Mix_PlayingMusic(); }
    void StopMusic() {Mix_HaltMusic();}
    void FadeOutMusic(int ms = 1000);

    // Draws the given text.
    void drawText(int x, int y, const char *msg, SDL_Color color = {255, 255, 255},
                  const TextAlign textAlign = TEXT_ALIGN_LEFT_TOP);

    // Fills a square
    void fillSquare(int x, int y, int side, SDL_Color color);

    void strokeSquare(int tl_x, int tl_y, int br_x, int br_y, SDL_Color color);

    // Return the total time spent in the game, in seconds.
    float getElapsedTime();

    struct KeyStatus {
        bool fire; // Z || ,
        bool jump; // X || .
        bool left; // left arrow || num pad 4
        bool right; // right arrow || num pad 6
        bool up; // up arrow || num pad 8
        bool down; // down arrow || num pad 5

        bool fire2; // F
        bool jump2; // G
        bool left2; // A
        bool right2; // D
        bool up2; // W
        bool down2; // S

        bool esc; // escape button
        bool pause; // pause button
        bool mute; // mute audio
        bool start; // enter
        bool debug; // godmode (0)
    };

    // Returns the keyboard status. If a flag is set, the corresponding key is being held down.
    void getKeyStatus(KeyStatus &keys);

    void ToggleSounds();

    void ToggleMusic();

private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool audioOpen = false;

    TTF_Font *font;

    KeyStatus key;
};

#endif