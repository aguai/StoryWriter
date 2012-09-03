#ifndef DEF_ENGINE_VARIABLES
#define DEF_ENGINE_VARIABLES

#include <windows.h>
#include <gdiplus.h>
#include <string>

// constants for the message loop
#define SCENE_FINISHED 0x8002
#define SCENE_CREATED  0x8003
#define GAME_FINISHED  0x8004

// structure throught which game states are changed
struct state_changes {std::string name; int value; bool replace;};	

// window coordinates
const int window_width  = 1280; 
const int window_height = 720;
const int upper_left_corner_X = 0; // used for button positioning 
const int upper_left_corner_Y = 0; //

// properties for the GDI+ Graphics
const Gdiplus::Color key_color(0,0,0); // color of the background - will be removed
const Gdiplus::Color text_surrounding_color(32,32,32);
const Gdiplus::Color text_fill_color(255,255,255);
const Gdiplus::Color nonactive_fill_color(159,159,159);
const Gdiplus::REAL text_size = 24;

#endif
