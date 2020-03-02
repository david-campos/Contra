#include "avancezlib.h"
#include <SDL_image.h>

std::unordered_map<int, unsigned int> current_sound_ids;
unsigned int next_sound_id = 1;

void channel_finished_callback(int channel) {
    current_sound_ids[channel] = 0;
}

std::function<void()> SoundEffect::Play(short times) {
    if (effect) {
        int channel = Mix_PlayChannel(-1, effect, times - 1);
        if (channel > 0) {
            int sound_id = next_sound_id++;
            current_sound_ids[channel] = sound_id;
            return [channel, sound_id]() {
                if (current_sound_ids[channel] == sound_id) {
                    Mix_HaltChannel(channel);
                }
            };
        }
    }
    return [](){};
}

// Creates the main window. Returns true on success.
bool AvancezLib::init(int width, int height) {
    SDL_Log("Initializing the engine...\n");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL failed the initialization: %s\n", SDL_GetError());
        return false;
    }

    //Create window
    window = SDL_CreateWindow("aVANCEZ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
            SDL_WINDOW_SHOWN);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    //Initialize PNG loading
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    //Create renderer for window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    TTF_Init();
    font = TTF_OpenFont("data/contra-famicom-nes.ttf", 32); //this opens a font style and sets a size
    if (font == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "font cannot be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // initialize the keys
    key.fire = false;
    key.left = false;
    key.right = false, key.esc = false;

    //Initialize renderer color
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);

    //Clear screen
    SDL_RenderClear(renderer);

    //Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    } else {
        Mix_ChannelFinished(channel_finished_callback);
        audioOpen = true;
    }

    SDL_Log("Engine up and running...\n");
    return true;
}


// Destroys the avancez library instance
void AvancezLib::destroy() {
    SDL_Log("Shutting down the engine\n");

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_CloseFont(font);

    TTF_Quit();
    SDL_Quit();
}

void AvancezLib::quit() {
    destroy();
    exit(0);
}


void AvancezLib::processInput() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                case SDLK_q:
                    key.esc = true;
                    break;
                case SDLK_z:
                case SDLK_COMMA:
                    key.fire = true;
                    break;
                case SDLK_x:
                case SDLK_PERIOD:
                    key.jump = true;
                    break;
                case SDLK_LEFT:
                case SDLK_KP_4:
                    key.left = true;
                    break;
                case SDLK_RIGHT:
                case SDLK_KP_6:
                    key.right = true;
                    break;
                case SDLK_DOWN:
                case SDLK_KP_5:
                    key.down = true;
                    break;
                case SDLK_UP:
                case SDLK_KP_8:
                    key.up = true;
                    break;
                case SDLK_f:
                    key.fire2 = true;
                    break;
                case SDLK_g:
                    key.jump2 = true;
                    break;
                case SDLK_a:
                    key.left2 = true;
                    break;
                case SDLK_d:
                    key.right2 = true;
                    break;
                case SDLK_w:
                    key.up2 = true;
                    break;
                case SDLK_s:
                    key.down2 = true;
                    break;
                case SDLK_0:
                    key.debug = true;
                    break;
                case SDLK_p:
                    key.pause = true;
                    break;
                case SDLK_RETURN:
                    key.start = true;
                    break;
            }
        }

        if (event.type == SDL_KEYUP) {
            switch (event.key.keysym.sym) {
                case SDLK_z:
                case SDLK_COMMA:
                    key.fire = false;
                    break;
                case SDLK_x:
                case SDLK_PERIOD:
                    key.jump = false;
                    break;
                case SDLK_LEFT:
                case SDLK_KP_4:
                    key.left = false;
                    break;
                case SDLK_RIGHT:
                case SDLK_KP_6:
                    key.right = false;
                    break;
                case SDLK_DOWN:
                case SDLK_KP_5:
                    key.down = false;
                    break;
                case SDLK_UP:
                case SDLK_KP_8:
                    key.up = false;
                    break;
                case SDLK_f:
                    key.fire2 = false;
                    break;
                case SDLK_g:
                    key.jump2 = false;
                    break;
                case SDLK_a:
                    key.left2 = false;
                    break;
                case SDLK_d:
                    key.right2 = false;
                    break;
                case SDLK_w:
                    key.up2 = false;
                    break;
                case SDLK_s:
                    key.down2 = false;
                    break;
                case SDLK_0:
                    key.debug = false;
                    break;
                case SDLK_p:
                    key.pause = false;
                    break;
                case SDLK_RETURN:
                    key.start = false;
                    break;
            }
        }

        if (event.type == SDL_QUIT) {
            key.esc = true;
        }
    }
}

void AvancezLib::swapBuffers() {
    //Update screen
    SDL_RenderPresent(renderer);
}

void AvancezLib::clearWindow() {
    //Clear screen
    SDL_RenderClear(renderer);
}


Sprite *AvancezLib::createSprite(const char *path) {
    SDL_Surface *surf = IMG_Load(path);
    if (surf == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s! SDL_image Error: %s\n", path,
                SDL_GetError());
        return NULL;
    }

    //Create texture from surface pixels
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture from %s! SDL Error: %s\n", path,
                SDL_GetError());
        return NULL;
    }
    //Get rid of old loaded surface
    SDL_FreeSurface(surf);
    Sprite *sprite = new Sprite(renderer, texture);
    return sprite;
}

void AvancezLib::drawText(int x, int y, const char *msg, SDL_Color color, const TextAlign textAlign) {
    SDL_Surface *surf = TTF_RenderText_Solid(font, msg, color);
    // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

    SDL_Texture *msg_texture = SDL_CreateTextureFromSurface(renderer, surf); //now you can convert it into a texture

    int w = 0;
    int h = 0;
    SDL_QueryTexture(msg_texture, NULL, NULL, &w, &h);
    if (textAlign == TEXT_ALIGN_CENTER_TOP
        || textAlign == TEXT_ALIGN_CENTER_MIDDLE
        || textAlign == TEXT_ALIGN_CENTER_BOTTOM) {
        x -= w / 2;
    } else if (textAlign == TEXT_ALIGN_RIGHT_TOP
               || textAlign == TEXT_ALIGN_RIGHT_MIDDLE
               || textAlign == TEXT_ALIGN_RIGHT_BOTTOM) {
        x -= w;
    }
    if (textAlign == TEXT_ALIGN_LEFT_MIDDLE
        || textAlign == TEXT_ALIGN_CENTER_MIDDLE
        || textAlign == TEXT_ALIGN_RIGHT_MIDDLE) {
        y -= h / 2;
    } else if (textAlign == TEXT_ALIGN_LEFT_BOTTOM
               || textAlign == TEXT_ALIGN_CENTER_BOTTOM
               || textAlign == TEXT_ALIGN_RIGHT_BOTTOM) {
        y -= h;
    }
    SDL_Rect dst_rect = {x, y, w, h};

    SDL_RenderCopy(renderer, msg_texture, NULL, &dst_rect);

    SDL_DestroyTexture(msg_texture);
    SDL_FreeSurface(surf);
}

void AvancezLib::fillSquare(int x, int y, int side, SDL_Color color) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = side;
    rect.h = side;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 255);
}

void AvancezLib::strokeSquare(int tl_x, int tl_y, int br_x, int br_y, SDL_Color color) {
    SDL_Rect rect;
    rect.x = tl_x;
    rect.y = tl_y;
    rect.w = br_x - tl_x;
    rect.h = br_y - tl_y;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 255);
}

float AvancezLib::getElapsedTime() {
    return SDL_GetTicks() / 1000.f;
}

void AvancezLib::getKeyStatus(KeyStatus &keys) {
    memcpy(&keys, &key, sizeof(KeyStatus));
}

SoundEffect *AvancezLib::createSound(const char *path) {
    auto *sound = Mix_LoadWAV(path);
    if (!sound) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load sound! SDL_mixer Error: %s\n", Mix_GetError());
    }
    return new SoundEffect(sound);
}

Music *AvancezLib::createMusic(const char *path) {
    auto *music = Mix_LoadMUS(path);
    if (!music) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }
    return new Music(music);
}


Sprite::Sprite(SDL_Renderer *renderer, SDL_Texture *texture) {
    this->renderer = renderer;
    this->texture = texture;
}


void Sprite::draw(int x, int y) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &(rect.w), &(rect.h));
    //Render texture to screen
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}

void Sprite::draw(int x, int y, int tw, int th, int sx, int sy, int sw, int sh, bool mirrorHorizontal) {
    SDL_Rect tgtRect, srcRect;
    tgtRect.x = x;
    tgtRect.y = y;
    tgtRect.w = tw;
    tgtRect.h = th;

    srcRect.x = sx;
    srcRect.y = sy;
    srcRect.w = sw;
    srcRect.h = sh;
    //Render texture to screen
    if (mirrorHorizontal)
        SDL_RenderCopyEx(renderer, texture, &srcRect, &tgtRect,
                0, nullptr, SDL_FLIP_HORIZONTAL);
    else SDL_RenderCopy(renderer, texture, &srcRect, &tgtRect);
}

Sprite::~Sprite() {
    SDL_DestroyTexture(texture);
}

int Sprite::getWidth() const {
    int w;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, nullptr);
    return w;
}
