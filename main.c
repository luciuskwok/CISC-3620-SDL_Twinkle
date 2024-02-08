// main.c

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

// Globals
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
uint32_t* pixels;
int pixels_w, pixels_h;
SDL_Rect window_rect;
bool is_running;

// Global state for rendering
uint32_t frame_index;

// Functions

void clear(uint32_t color) {
	for (int i = 0; i < pixels_w * pixels_h; i++) {
		pixels[i] = color;
	}
}

void set_pixel(int x, int y, uint32_t color) {
	if (x < 0 || x >= pixels_w) return;
	if (y < 0 || y >= pixels_h) return;
	pixels[x + y * pixels_w] = color;
}

void draw_grid(int center_x, int center_y, int spacing, uint32_t color) {
	// Draw vertical lines
	int x, y;
	for (x = center_x; x < pixels_w; x += spacing) {
		for (y = 0; y < pixels_h; y++) {
			set_pixel(x, y, color);
		}
	}
	for (x = center_x - spacing; x >= 0; x -= spacing) {
		for (y = 0; y < pixels_h; y++) {
			set_pixel(x, y, color);
		}
	}

	// Draw horizontal lines
	for (y = center_y; y < pixels_h; y += spacing) {
		for (x = 0; x < pixels_w; x++) {
			set_pixel(x, y, color);
		}
	}
	for (y = center_y - spacing; y >= 0; y -= spacing) {
		for (x = 0; x < pixels_w; x++) {
			set_pixel(x, y, color);
		}
	}
}

void draw_rect(int center_x, int center_y, int w, int h, uint32_t color) {
	int top = center_y - h / 2;
	int left = center_x - w / 2;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			set_pixel(left + x, top + y, color);
		}
	}
}

bool initialize_windowing_system() {

	// Set up SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL_Init() failed!\n");
		return false;
	}

	// Full screen
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window_rect.x = 0;
	window_rect.y = 0;
	window_rect.w = display_mode.w;
	window_rect.h = display_mode.h;

	// Window
	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_rect.w, window_rect.h, SDL_WINDOW_BORDERLESS);
	// options: SDL_WINDOWPOS_CENTERED, SDL_WINDOW_BORDERLESS
	if (!window) {
		fprintf(stderr, "SDL_CreateWindow() failed!\n");
		return false;
	}

	// Renderer
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "SDL_CreateRenderer() failed!\n");
		return false;
	}

	// Texture
	pixels_w = window_rect.w;
	pixels_h = window_rect.h;
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
		SDL_TEXTUREACCESS_STREAMING, pixels_w, pixels_h);
	if (!texture) {
		fprintf(stderr, "SDL_CreateTexture() failed!\n");
		return false;
	}

	// Allocate frame buffer
	pixels = (uint32_t*)malloc(pixels_w * pixels_h * sizeof(uint32_t));
	if (!pixels) {
		fprintf(stderr, "malloc() failed!\n");
		return false;
	}
	clear(0x000000FF);

	// Set up the renderer
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0); // Use no interpolation
	// Clear the window
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	// Reset the frame index
	frame_index = 0;

	return true;
}

void clean_up_windowing_system() {
	free(pixels);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void process_keyboard_input() {
	SDL_Event event;
	SDL_PollEvent(&event);

	// Keyboard interaction
	switch (event.type) {
	case SDL_QUIT: // when 'X' button is pressed in window titlebar
		// do something
		is_running = false;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE) {
			is_running = false;
		}
		break;
	}
}

void update_state() {

}

void run_render_pipeline() {
	// Clear frame buffer
	clear(0x000000FF);

	// Draw 3 rects behind grid
	draw_rect(pixels_w / 2, pixels_h * 7 / 16, pixels_w * 3 / 4, pixels_h / 16, 0x5fcde4ff);
	draw_rect(pixels_w * 5 / 16, pixels_h * 7 / 16, pixels_w * 1 / 8, pixels_h * 17 / 32, 0x5b6ee1ff);
	draw_rect(pixels_w * 8 / 16, pixels_h * 11 / 16, pixels_w * 15 / 16, pixels_h * 7 / 32, 0x1a5e42ff); // green lower

	// Draw grid
	int grid_spacing = 32;
	int grid_x = pixels_w / 2 + (frame_index / 2) % 32;
	int grid_y = pixels_w / 2 + (frame_index / 1) % 32;
	draw_grid(grid_x, grid_y, grid_spacing, 0x808080FF);

	// Draw 2 rects in front of grid
	draw_rect(pixels_w * 11 / 16, pixels_h * 3 / 16, pixels_w * 7 / 16, pixels_h * 5 / 16, 0x005ff3ff);
	draw_rect(pixels_w * 9 / 16, pixels_h * 13 / 16, pixels_w * 7 / 16, pixels_h * 5 / 16, 0x99e550ff); // green lower


	// Render frame buffer
	SDL_UpdateTexture(texture, NULL, pixels, pixels_w * sizeof(uint32_t));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, &window_rect);
	SDL_RenderPresent(renderer);

	// Update frame index
	frame_index++;
}

int main(int argc, char* argv[]) {
	printf("Started program.\n");

	if (!initialize_windowing_system()) return 0;

	// Game loop
	is_running = true;
	while (is_running) {
		process_keyboard_input();
		update_state();
		run_render_pipeline();

		// Wait for 1/60 second = (1000 / 60)
		SDL_Delay(1000 / 60);
	}

	clean_up_windowing_system();

	return 0;
}
