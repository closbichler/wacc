#include "game.h"

int client_width = 0, client_height = 0;
const int bricks_vertical = 12, bricks_horizontal = 30;
const int particle_cache = 40;

unsigned int random_seed = 0;

typedef enum
{
	Black = 0x000000,
	Pink = 0xFF00EC,
	Red = 0xFF5733,
	Yellow = 0xffc133,
	Blue = 0x33a5ff,
	Purple = 0xe033ff
} Color;

typedef struct
{
	unsigned int color;
	int offset;
	int vel;
} Brick;

typedef struct
{
	int x, y, vx, vy, s;
} Particle;

typedef struct
{
	int score, score_offset;
	int brick_size;
	Brick bricks[bricks_horizontal][bricks_vertical];
	Particle particles[20];
	int particle_length;
} Game;

Game game = {0};

int round_ceil(double x)
{
	int int_part = (int)x;
	double fract_part = x - int_part;
	if (fract_part >= 0.5)
		return int_part + 1;
	return int_part;
}

double sqrt(double square)
{
	double root = square / 3;
	int i;
	if (square <= 0)
		return 0;
	for (i = 0; i < 32; i++)
		root = (root + square / root) / 2;
	return root;
}

int abs_min(int n, int m)
{
	if (n >= 0)
	{
		if (n <= m)
			return n;
		return m;
	}
	if (n >= -m)
		return n;
	return -m;
}

char *int_to_char(int i, char *p)
{
	if (i / 10 == 0)
	{
		*p++ = '0' + i;
		*p = '\0';
		return p;
	}

	p = int_to_char(i / 10, p);
	*p++ = i % 10 + '0';
	*p = '\0';
	return p;
}

Color random_color()
{
	random_seed = random_seed * 1.5 + 1;
	int r = random_seed % 4;
	switch (r)
	{
	case 0:
		return Red;
	case 1:
		return Yellow;
	case 2:
		return Blue;
	case 3:
	default:
		return Purple;
	}
}

int random_number(int min, int max)
{
	random_seed = random_seed * 1.5 + 1;
	return min + random_seed % (max - min);
}

void draw_brick(int x, int y, Brick brick)
{
	if (brick.color == Black)
		return;

	int d = 1;
	fill_rect(x + d, y + d + brick.offset, game.brick_size - d * 2, game.brick_size - d * 2, brick.color * 2);
	d = 4;
	fill_rect(x + d, y + d + brick.offset, game.brick_size - d * 2, game.brick_size - d * 2, brick.color);
}

void draw_particle(Particle p)
{
	if (p.s == 0)
		return;

	int s = p.s + 6;
	fill_rect(p.x, p.y, s, s, Pink);
}

void spawn_particle(int i, int j, int score)
{
	if (game.particle_length >= particle_cache - 1)
	{
		game.particle_length = 0;
	}

	int x = i * game.brick_size;
	int y = game.score_offset + j * game.brick_size;
	int s = random_number(1, 2 + score);
	int vx = s % 4 * 2 - 4;
	int vy = s % 6 + 2;
	game.particles[game.particle_length] = (Particle){x, y, vx, vy, s};
	game.particle_length++;
}

int crush_brick(int i, int j, unsigned int color)
{
	Brick b = game.bricks[i][j];
	if (b.color != color || b.offset != 0)
		return 0;

	game.bricks[i][j] = (Brick){0};
	int n = 1;

	if (i > 0)
		n += crush_brick(i - 1, j, color);
	if (i < bricks_horizontal - 1)
		n += crush_brick(i + 1, j, color);
	if (j > 0)
		n += crush_brick(i, j - 1, color);
	if (j < bricks_vertical - 1)
		n += crush_brick(i, j + 1, color);

	spawn_particle(i, j, 1);
	spawn_particle(i, j, n % 6);

	return n;
}

void spawn_bricks(int i, int n)
{
	for (int j = 0; j < n; j++)
	{
		game.bricks[i][j] = (Brick){random_color(), game.brick_size * -(8 + n - j)};
	}
}

void sink_brick(Brick *brick)
{
	if (brick->offset <= 0)
	{
		brick->offset += brick->vel;
		brick->vel += 1;
	}
	else
	{
		brick->offset = 0;
	}
}

void update_particle(Particle *p)
{
	if (p->s == 0)
		return;

	p->x += p->vx;
	p->y += p->vy;

	float dx = client_width / 2 - p->x;
	float dy = game.score_offset / 2 - p->y;
	float norm = sqrt(dx * dx + dy * dy);
	float ux = dx / norm;
	float uy = dy / norm;
	p->vx += (int)((2.5f / norm * norm) * ux);
	p->vy += (int)((2.5f / norm * norm) * uy);

	p->vx = abs_min(p->vx, 20);
	p->vy = abs_min(p->vy, 20);

	if (norm < 70)
	{
		game.score += p->s;
		p->s = 0;
	}
}

void on_mouse_down(int x, int y)
{
	y -= game.score_offset;

	int brick_i = x / game.brick_size;
	int brick_j = y / game.brick_size;

	if (brick_i >= 0 && brick_i < bricks_horizontal &&
			brick_j >= 0 && brick_j < bricks_vertical)
	{
		crush_brick(brick_i, brick_j, game.bricks[brick_i][brick_j].color);
	}
}

void game_init(int width, int height)
{
	client_width = width;
	client_height = height;

	game.brick_size = client_width / bricks_horizontal;
	game.score_offset = 100;

	game.particles[0] = (Particle){200, client_height - 50, 0, 0, 20};
}

void game_update(double dt)
{
	for (int i = 0; i < bricks_horizontal; i++)
	{
		int all_settled = 1;
		for (int j = 0; j < bricks_vertical; j++)
		{
			if (game.bricks[i][j].color != Black && game.bricks[i][j].offset != 0)
			{
				all_settled = 0;
				break;
			}
		}

		if (all_settled == 1)
		{
			int spawn_n = 0;
			for (; spawn_n < bricks_vertical; spawn_n++)
			{
				if (game.bricks[i][spawn_n].color != Black)
					break;
			}
			spawn_bricks(i, spawn_n);
		}
	}

	for (int i = 0; i < bricks_horizontal; i++)
	{
		for (int j = 0; j < bricks_vertical; j++)
		{
			if (j > 0 && game.bricks[i][j].color == 0 && game.bricks[i][j - 1].offset >= 0)
			{
				game.bricks[i][j] = game.bricks[i][j - 1];
				game.bricks[i][j].offset += -game.brick_size + game.bricks[i][j].vel;
				game.bricks[i][j - 1].color = Black;
			}
			else if (game.bricks[i][j].offset != 0)
			{
				sink_brick(&(game.bricks)[i][j]);
			}
			else if (j < bricks_vertical - 1 && game.bricks[i][j].offset == 0 &&
							 j < bricks_vertical - 1 && game.bricks[i][j + 1].color != 0)
			{
				game.bricks[i][j].vel = 0;
			}
		}
	}

	for (int i = 0; i < particle_cache; i++)
	{
		update_particle(&(game.particles)[i]);
	}
}

void game_render()
{
	fill_rect(0, 0, client_width, client_height, Black);

	for (int i = 0; i < bricks_horizontal; i++)
	{
		for (int j = 0; j < bricks_vertical; j++)
		{
			draw_brick(i * game.brick_size, game.score_offset + j * game.brick_size, (game.bricks)[i][j]);
		}
	}

	for (int i = 0; i < game.particle_length; i++)
	{
		draw_particle(game.particles[i]);
	}

	// draw score
	char score_str[10];
	int_to_char(game.score, score_str);
	fill_text(client_width / 2, 50, score_str, 50, Pink);
}