#include "avancezlib.h"
#include <SDL_image.h>

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

	m_windowSurface = SDL_GetWindowSurface(window);

	TTF_Init();
	font = TTF_OpenFont("data/space_invaders.ttf", 12); //this opens a font style and sets a size
	if (font == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "font cannot be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// initialize the keys
	key.fire = false;
	key.left = false;
	key.right = false, key.esc = false;

	//Initialize renderer color
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Clear screen
	SDL_RenderClear(renderer);

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
					key.fire = true;
					break;
			    case SDLK_x:
			        key.jump = true;
			        break;
				case SDLK_LEFT:
					key.left = true;
					break;
				case SDLK_RIGHT:
					key.right = true;
					break;
			    case SDLK_DOWN:
			        key.down = true;
			        break;
			    case SDLK_UP:
			        key.up = true;
			        break;
			}
		}

		if (event.type == SDL_KEYUP) {
			switch (event.key.keysym.sym) {
                case SDLK_z:
                    key.fire = false;
                    break;
                case SDLK_x:
                    key.jump = false;
                    break;
				case SDLK_LEFT:
					key.left = false;
					break;
				case SDLK_RIGHT:
					key.right = false;
					break;
                case SDLK_DOWN:
                    key.down = false;
                    break;
                case SDLK_UP:
                    key.up = false;
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
	SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 0xFF, 0xFF, 0xFF));

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

void AvancezLib::drawText(int x, int y, const char *msg) {
	SDL_Color black = {0, 0,
					   0};  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color

	SDL_Surface *surf = TTF_RenderText_Solid(font, msg,
											 black); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

	SDL_Texture *msg_texture = SDL_CreateTextureFromSurface(renderer, surf); //now you can convert it into a texture

	int w = 0;
	int h = 0;
	SDL_QueryTexture(msg_texture, NULL, NULL, &w, &h);
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

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 255);
}

float AvancezLib::getElapsedTime() {
	return SDL_GetTicks() / 1000.f;
}

void AvancezLib::getKeyStatus(KeyStatus &keys) {
	keys.fire = key.fire;
	keys.jump = key.jump;
	keys.left = key.left;
	keys.right = key.right;
	keys.up = key.up;
	keys.down = key.down;
	keys.esc = key.esc;
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
	if (mirrorHorizontal) SDL_RenderCopyEx(renderer, texture, &srcRect, &tgtRect,
	        0, nullptr,SDL_FLIP_HORIZONTAL);
	else SDL_RenderCopy(renderer, texture, &srcRect, &tgtRect);
}

Sprite::~Sprite() {
	SDL_DestroyTexture(texture);
}

