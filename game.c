#include "game.h"

int client_width = 0, client_height = 0;

// TODO: dependent on screen size
const int bricks_horizontal_max = 60, bricks_vetical_max = 60;
const int particle_cache = 40;
unsigned int random_seed = 0;

typedef enum
{
	Black = 0x000000,
	White = 0xFFFFFF,
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
	char content;
} Brick;

typedef struct
{
	int x, y, vx, vy, s;
} Particle;

typedef struct
{
	int score, score_offset;

	float scale;
	int brick_offset;
	int brick_size, bricks_vetical, bricks_horizontal;
	Brick bricks[bricks_horizontal_max][bricks_vetical_max];

	Particle particles[particle_cache];
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

int min(int n, int m)
{
	if (n <= m)
		return n;
	return m;
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

void draw_brick(int x, int y, Brick brick)
{
	if (brick.color == Black)
		return;

	int d = 1;
	fill_rect(x + d, y + d + brick.offset, game.brick_size - d * 2, game.brick_size - d * 2, brick.color * 2);
	d = 4;
	fill_rect(x + d, y + d + brick.offset, game.brick_size - d * 2, game.brick_size - d * 2, brick.color);

	if (brick.content == '*')
	{
		d = game.brick_size/3;
		fill_rect(x + d, y + d + brick.offset, game.brick_size - d * 2, game.brick_size - d * 2, White);
	}
}

void draw_particle(Particle p)
{
	if (p.s == 0)
		return;

	int s = p.s + 6;
	fill_rect(p.x, p.y, s * game.scale, s * game.scale, White);
}

void spawn_particle(int i, int j, int score)
{
	if (game.particle_length >= particle_cache - 1)
		game.particle_length = 0;

	int x = i * game.brick_size;
	int y = game.score_offset + j * game.brick_size;
	int s = rand(1, 2 + score);
	int vx = ((s % 3) - 1) * 8;
	int vy = s % 6 + 8;
	game.particles[game.particle_length] = (Particle){x, y, vx, vy, s};
	game.particle_length++;
}

int explode_bricks(int i, int j)
{
	for (int i=0; i<10; i++)
	{
		spawn_particle(i, j, 10);
	}
	return 20;
}

int crush_brick(int i, int j, unsigned int color)
{
	Brick b = game.bricks[i][j];
	if (b.color != color || b.offset != 0)
		return 0;

	if (b.content == '*')
	{
		explode_bricks(i, j);
	}

	game.bricks[i][j] = (Brick){0};
	int n = 1;

	if (i > 0)
		n += crush_brick(i - 1, j, color);
	if (i < game.bricks_horizontal - 1)
		n += crush_brick(i + 1, j, color);
	if (j > 0)
		n += crush_brick(i, j - 1, color);
	if (j < game.bricks_vetical - 1)
		n += crush_brick(i, j + 1, color);

	spawn_particle(i, j, 1);
	spawn_particle(i, j, n % 6);

	return n;
}

void spawn_bricks(int i, int n)
{
	for (int j = 0; j < n; j++)
	{
		char content = 0;
		if (rand(1, 60) == 1)
			content = '*';
		game.bricks[i][j] = (Brick){random_color(), game.brick_size * -(8 + n - j), 0, content};
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

	p->x += p->vx * game.scale;
	p->y += p->vy * game.scale;
	p->vx *= 0.92;
	p->vy *= 0.92;

	// TODO: rewrite particle force logic
	float dx = client_width / 2 - p->x;
	float dy = game.score_offset / 2 - p->y;
	float norm = sqrt(dx * dx + dy * dy);
	float ux = dx / norm;
	float uy = dy / norm;
	p->vx += (int)((3.0f / norm * norm) * ux);
	p->vy += (int)((3.0f / norm * norm) * uy);

	if (norm < 70 || p->y < 0)
	{
		game.score += p->s;
		p->s = 0;
	}
}

void on_mouse_down(int x, int y)
{
	y -= game.brick_offset;

	int brick_i = x / game.brick_size;
	int brick_j = y / game.brick_size;

	if (brick_i >= 0 && brick_i < game.bricks_horizontal &&
			brick_j >= 0 && brick_j < game.bricks_vetical)
	{
		crush_brick(brick_i, brick_j, game.bricks[brick_i][brick_j].color);
	}
}

void game_init(int width, int height, int devicePixelSize)
{
	client_width = width;
	client_height = height;
	game.scale = devicePixelSize;

	game.brick_size = 40 * game.scale;
	game.score_offset = 100;
	game.bricks_horizontal = min(client_width / game.brick_size, bricks_horizontal_max);
	game.bricks_vetical = min((client_height - game.score_offset) / game.brick_size, bricks_vetical_max);
	game.brick_offset = client_height - game.bricks_vetical * game.brick_size;
}

void game_update(double dt)
{
	for (int i = 0; i < game.bricks_horizontal; i++)
	{
		int all_settled = 1;
		for (int j = 0; j < game.bricks_vetical; j++)
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
			for (; spawn_n < game.bricks_vetical; spawn_n++)
			{
				if (game.bricks[i][spawn_n].color != Black)
					break;
			}
			spawn_bricks(i, spawn_n);
		}
	}

	for (int i = 0; i < game.bricks_horizontal; i++)
	{
		for (int j = 0; j < game.bricks_vetical; j++)
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
			else if (j < game.bricks_vetical - 1 && game.bricks[i][j].offset == 0 &&
							 j < game.bricks_vetical - 1 && game.bricks[i][j + 1].color != 0)
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

	for (int i = 0; i < game.bricks_horizontal; i++)
	{
		for (int j = 0; j < game.bricks_vetical; j++)
		{
			int x = i * game.brick_size;
			int y = client_height - (game.bricks_vetical - j) * game.brick_size;
			draw_brick(x, y, game.bricks[i][j]);
		}
	}

	for (int i = 0; i < game.particle_length; i++)
	{
		draw_particle(game.particles[i]);
	}

	char score_str[10];
	int_to_char(game.score, score_str);
	fill_text_centered(client_width / 2, game.score_offset * 2 / 3, score_str, 50, Pink);
}