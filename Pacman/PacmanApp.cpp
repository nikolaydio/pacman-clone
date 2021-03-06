#include "PacmanApp.h"

#include <SDL.h>
#undef main
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <cassert>

#define MSEC_PER_FRAME 16

PacmanApp::PacmanApp() {
}
bool PacmanApp::Init() {
	status = START_SCREEN;
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	TTF_Init();
	if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
	{
		printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
		return 0;
	}

	window = SDL_CreateWindow("Pacman Game", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_SIZE_X, WINDOW_SIZE_Y, 0);
	if(!window) {
		return false;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!renderer) {
		SDL_DestroyWindow(window);
		return false;
	}
	res_manager.Init(renderer, "data/");

	start_screen.Init(res_manager);
	pacman.Init(res_manager);
	logic = &start_screen;


	return true;
}

void PacmanApp::LoadGame(const char* filename) {
	TileMap* map = LoadTileMap(filename);
	if(!map) {
		printf("Failed to load %s\n", filename);
		return;
	}
	pacman.SetTilemap(map);
	logic = &pacman;
	status = IN_GAME;
}
void PacmanApp::LoadStartScreen() {
	status = START_SCREEN;
	logic = &start_screen;
}
void PacmanApp::LoadScore(bool victory, int score) {
	status = START_SCREEN;
	logic = &start_screen;
	start_screen.Stats(victory, score);
}
void PacmanApp::ExitGame() {
	status = GAME_EXITING;
}
void PacmanApp::Run() {
	int last_frame = SDL_GetTicks();
	
	while(status != GAME_EXITING) {
		int now = SDL_GetTicks();
		int delta_ms = now - last_frame;
		last_frame = now;

		if(delta_ms < MSEC_PER_FRAME) {
			SDL_Delay(MSEC_PER_FRAME - delta_ms);
		}

		int32_t input_state = 0;
		int32_t input_events = 0;

		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(e.type == SDL_QUIT) {
				status = GAME_EXITING;
			}else if(e.type == SDL_KEYDOWN) {
				if(e.key.keysym.sym == SDLK_UP) {
					input_events |= INPUT_UP;
				}else if(e.key.keysym.sym == SDLK_DOWN) {
					input_events |= INPUT_DOWN;
				}else if(e.key.keysym.sym == SDLK_LEFT) {
					input_events |= INPUT_LEFT;
				}else if(e.key.keysym.sym == SDLK_RIGHT) {
					input_events |= INPUT_RIGHT;
				}else if(e.key.keysym.sym == SDLK_SPACE) {
					input_events |= INPUT_FIRE;
				}else if(e.key.keysym.sym == SDLK_ESCAPE) {
					input_events |= INPUT_ESC;
				}
			}
		}
		const Uint8* keyboard = SDL_GetKeyboardState(0);
		if(keyboard[SDL_SCANCODE_UP]) input_state |= INPUT_UP;
		if(keyboard[SDL_SCANCODE_DOWN]) input_state |= INPUT_DOWN;
		if(keyboard[SDL_SCANCODE_LEFT]) input_state |= INPUT_LEFT;
		if(keyboard[SDL_SCANCODE_RIGHT]) input_state |= INPUT_RIGHT;
		if(keyboard[SDL_SCANCODE_SPACE]) input_state |= INPUT_FIRE;
		if(keyboard[SDL_SCANCODE_ESCAPE]) input_state |= INPUT_ESC;

		SDL_RenderClear(renderer);
		logic->Update(delta_ms / 1000.0f, input_state, input_events, this);
		logic->Render(delta_ms / 1000.0f, renderer);
		SDL_RenderPresent(renderer);
	}
}
void PacmanApp::Done() {
	res_manager.Cleanup();
	Mix_CloseAudio();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}