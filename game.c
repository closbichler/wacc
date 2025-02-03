#include "game.h"

static int width = 0, height = 0;

static int x = 0, y = 0;
static int vx = 2, vy = 2;
static int w = 40, h = 40;

static int score = 0;

void game_init(int client_width, int client_height) 
{
	width = client_width;
	height = client_height;
	x = width/2;
	y = height/2;
}

void game_update(float timestmap) 
{
	x += vx;
	y += vy;

	if (x <= 0 || x+w >= width) 
	{
		vx *= -1;
		score++;
	}
	if (y <= 0 || y+h >= height)
	{
		vy *= -1;
		score++;
	}
}

void game_render() 
{
	// clear canvas
	fill_rect(width, height, 0, 0, 0xFFFFFF);
	
	fill_rect(w, h, x, y, 0xFF0000);
	fill_text(score, 10, 10, 0x0);
}