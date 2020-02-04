#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

class Sprite
{
	SDL_Renderer * renderer;
	SDL_Texture * texture;
public:

	Sprite(SDL_Renderer * renderer, SDL_Texture * texture);

	// Destroys the sprite instance
	~Sprite();

	// Draw the sprite at the given position.
	void draw(int x, int y);
	// Draw a part of the sprite at the given position,
	// the params sx, sy, sw, sh allow to define the part
	// of the sprite to draw
	void draw(int x, int y, int tw, int th, int sx, int sy, int sw, int sh, bool mirrorHorizontal = false);
};


class AvancezLib
{
public:
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
	Sprite* createSprite(const char* name);
	// Creates a new sprite horizontally mirroring the given one
	Sprite* mirrorSprite(Sprite* sprite);

	// Draws the given text.
	void drawText(int x, int y, const char* msg);

	// Return the total time spent in the game, in seconds.
	float getElapsedTime();

	struct KeyStatus
	{
		bool fire; // Z
		bool jump; // X
		bool left; // left arrow
		bool right; // right arrow
		bool up; // up arrow
		bool down; // down arrow
		bool esc; // escape button
	};

	// Returns the keyboard status. If a flag is set, the corresponding key is being held down.
	void getKeyStatus(KeyStatus& keys);

private:
	SDL_Window * window;
	SDL_Renderer * renderer;
	SDL_Surface * m_windowSurface;

	TTF_Font* font;

	KeyStatus key;
};

