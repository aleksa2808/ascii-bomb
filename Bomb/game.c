#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

#define EMPTY 0
#define STONE_WALL 2
#define WALL 3
#define BOMB 4
#define FIRE 5
#define WALL_ON_FIRE 6
#define POWER_ON_FIRE 7
#define EXIT 8

#define POWER_START 11
#define BOMBS_UP 11
#define RANGE_UP 12
#define HEALTH_UP 13
#define WALL_HACK 14
#define IMMORTAL 15
#define BOMB_PUSH 16

#define STORY_TIME 300
#define GAME_TIME 120
#define WOD_TIME 60
#define BOMB_TIME 2
#define FIRE_TIME 0.5
#define IMMORTAL_TIME 4

#define POWER_CHANCE 10

typedef struct {
	int id, type, x, y, health, bombs, bomb_range, action, last_action, immortal_end, last_move, speed;
	bool immortal;
	unsigned char powers;
} Player;
typedef struct { 
	Player *owner;
	int x, y, range, end_time, xdir, ydir, last_move;
} Bomb;
typedef struct BombList {
	Bomb *bomb;
	struct BombList *prev, *next;
};
typedef struct PlayerList {
	Player *player;
	struct PlayerList *prev, *next;
};
typedef struct FireList {
	int x, y, end_time;
	struct FireList *prev, *next;
};
typedef struct WallOfDeath
{
	int wod_start, wody, wodx, woddir, wodinc, wod_last;
	bool wodstop;
};

char **screen;
struct BombList *blist_front, *blist_rear;
struct PlayerList *plist_front, *plist_rear;
struct MobList *mlist_front, *mlist_rear;
struct FireList *flist_front, *flist_rear;
int time_end, iter_time, m, n, per, mode;

// for lack of a better global // story mode stuff
int points, health, bombs, range, powers, exitx, exity;

extern WINDOW *game_win;

extern int sdon;

extern void init_screen(int, int, int);
extern void draw(char**, struct BombList*, struct PlayerList*, struct MobList*);
extern void scoreboard(int *scores, int num);
extern void del_stuff(void);

extern void bot_action(Player*);
extern void mob_action(Player*);

void boom(int y, int x, int range);

void create_map(int level)
{
	int i, j, fill, mx, my;

    screen = (char**) malloc(m * sizeof(char*));
    for (i = 0; i < m; i++)
    {
		screen[i] = (char*) malloc(n * sizeof(char));
    }

	/* ~Map creation~ */
    /* Grass */
    for (i = 0; i < m; i++)
            for (j = 0; j < n; j++)
                    screen[i][j] = EMPTY;
 
    /* Borders */
    for (i = 0; i < m; i++)
    {
            screen[i][0] = STONE_WALL;
            screen[i][n - 1] = STONE_WALL;
    }
    for (i = 0; i < n; i++)
    {
            screen[0][i] = STONE_WALL;
            screen[m - 1][i] = STONE_WALL;
    }
       
    /* Blocks */
    for (i = 2; i < m - 1; i += 2)
            for (j = 2; j < n - 1; j += 2)
                    screen[i][j] = STONE_WALL;

    /* Destructibles */
	if (!(mode == 1 && level % 5 == 0)) // if not boss level
	{
		fill = (((m - 2) / 2 + 1) * (n - 2) + ((m - 2) / 2) * ((n - 2) / 2 + 1)); // Number of empty blocks
		fill = fill * (per/100.0);
		for (i = 0; i < fill; i++)
		{
				do
				{
						my = rand()%(m - 2) + 1;
						mx = rand()%(n - 2) + 1;
				}
				while (screen[my][mx] != EMPTY);
				screen[my][mx] = WALL;
		}
	}
}
void spawn_exit(int level)
{
	int my, mx;

	if (level % 5 != 0)
	{
		do
		{
				my = rand()%(m - 2) + 1;
				mx = rand()%(n - 2) + 1;
		}
		while (!(screen[my][mx] == WALL && my > 3 && mx > 3));
		exity = my, exitx = mx;
	}
	else
	{
		exity = 0;
		exitx = 0;
	}
}
struct WallOfDeath* init_wod(struct WallOfDeath *w)
{
	w = (struct WallOfDeath*) malloc(sizeof(struct WallOfDeath));
	w->wody = m - 1;
	w->wodx = 1;
	w->woddir = 4;
	w->wodinc = 0;
	w->wod_last = clock();
	w->wodstop = FALSE;
	return w;
}
void spawn(int id, Player *player, int y, int x)
{
	screen[y][x] = EMPTY;
	if (screen[y][x + 1] == WALL) screen[y][x + 1] = EMPTY;
	if (screen[y + 1][x] == WALL) screen[y + 1][x] = EMPTY;
	if (screen[y][x - 1] == WALL) screen[y][x - 1] = EMPTY;
	if (screen[y - 1][x] == WALL) screen[y - 1][x] = EMPTY;

	player->id = id;
	player->type = 0;
	player->y = y;
	player->x = x;
	player->health = 1;
	player->bombs = 1;
	player->bomb_range = 2;
	player->immortal = FALSE;
	player->powers = 0;
	player->last_move = 0;
	player->action = 0;
	player->last_action = 0;
	player->speed = 300;
}
void player_queue(Player *player)
{
	struct PlayerList *p;
	p = (struct PlayerList*) malloc(sizeof(struct PlayerList));
	p->player = player;
	p->next = NULL;
	if (plist_rear == NULL) 
	{
		plist_front = p;
		p->prev = NULL;
	}
	else 
	{
		plist_rear->next = p;
		p->prev = plist_rear;
	}
	plist_rear = p;
}
void init_players(int num_players, int num_bots)
{
	int i;
	Player *player;
	int x[8] = {1, n - 2, n - 2, 1, 5, n - 6, n - 6, 5}, y[8] = {1, m - 2, 1, m - 2, 3, m - 4, 3, m - 4};
	for (i = 0; i < num_players; i++)
	{
		player = (Player*) malloc(sizeof(Player));
		player_queue(player);
		spawn(i + 1, player, y[i], x[i]);	
	}
	for (i = 0; i < num_bots; i++)
	{
		player = (Player*) malloc(sizeof(Player));
		player_queue(player);
		spawn(num_players + i + 1, player, y[num_players + i], x[num_players + i]);
	}
}
void init_mobs(int level)
{
	int i, j, dir, mob_num = level % 5 + 2;
	Player *player;
	int x[8] = {n - 4, n - 2, 11, 5, 1, n - 8}, y[8] = {m - 8, 1, m - 2, m - 6, 9, 3};
	int bias = rand() % 20;

	for (i = 0; i < mob_num; i++)
	{
		player = (Player*) malloc(sizeof(Player));
		player_queue(player);
		player->id = i + 2;
		player->type = i > 4 ? 2 : 1;
		player->y = y[(i + bias) % 6];
		player->x = x[(i + bias) % 6];
		if (dir = rand() % 2)
		{
			j = 0;
			while (j < 3 && screen[player->y][player->x + j] != STONE_WALL)
			{
				screen[player->y][player->x + j] = EMPTY;
				j++;
			}
			j = 1;
			while (j < 3 && screen[player->y][player->x - j] != STONE_WALL)
			{
				screen[player->y][player->x - j] = EMPTY;
				j++;
			}
		}
		else
		{
			j = 0;
			while (j < 3 && screen[player->y + j][player->x] != STONE_WALL)
			{
				screen[player->y + j][player->x] = EMPTY;
				j++;
			}
			j = 1;
			while (j < 3 && screen[player->y - j][player->x] != STONE_WALL)
			{
				screen[player->y - j][player->x] = EMPTY;
				j++;
			}
		}
		player->health = player->type;
		player->bombs = 0;
		player->bomb_range = 0;
		player->immortal = FALSE;
		player->powers = 0;
		player->last_move = 0;
		player->action = 0;
		if (dir)
		{
			if (rand() % 2)
				player->last_action = 1;
			else
				player->last_action = 3;
		}
		else
		{
			if (rand() % 2)
				player->last_action = 2;
			else
				player->last_action = 4;
		}
		player->speed = 500;
	}
}
void pause(bool *running)
{
	int pause_time;
	int ch;
	struct PlayerList *p;
	struct BombList *b;
	struct FireList *f;

	if (sdon) PlaySound(TEXT("sounds/pause.wav"), NULL, SND_ASYNC | SND_FILENAME);
	nodelay(game_win, FALSE);
	while ((ch = wgetch(game_win)) != 10)
		if (ch == 27) 
		{
			*running = FALSE;
			break;
		}
	nodelay(game_win, TRUE);
	
	pause_time = clock() - iter_time;
	
	time_end += pause_time;
	
	p = plist_front;
	while (p)
	{
		p->player->immortal_end += pause_time;
		p->player->last_move += pause_time;

		p = p->next;
	}

	b = blist_front;
	while (b)
	{
		b->bomb->end_time += pause_time;
		b->bomb->last_move += pause_time;

		b = b->next;
	}

	f = flist_front;
	while (f)
	{
		f->end_time += pause_time;

		f = f->next;
	}

	iter_time = clock();
}
void power_time(Player *player)
{
	switch (screen[player->y][player->x])
	{
	case BOMBS_UP:
		player->bombs++;
		if (mode == 1) bombs++;
		break;
	case RANGE_UP:
		player->bomb_range++;
		if (mode == 1) range++;
		break;
	case HEALTH_UP:
		player->health++;
		if (mode == 1) health++;
		break;
	case WALL_HACK:
		player->powers |= 0x10;
		if (mode == 1) powers |= 0x10;;
		break;
	case IMMORTAL:
		player->immortal = TRUE;
		player->immortal_end = iter_time + IMMORTAL_TIME * CLOCKS_PER_SEC;
		break;
	case BOMB_PUSH:
		player->powers |= 0x1;
		if (mode == 1) powers |= 0x1;;
	}
	screen[player->y][player->x] = EMPTY;
}
void gen_power(int y, int x)
{
	int r = rand() % 100;

	/* "Loot tables" */
	if (mode == 1)
	{
		if (r < 50) screen[y][x] = BOMBS_UP;
		if (r >= 50 && r < 80) screen[y][x] = RANGE_UP;
		if (r >= 80 && r < 90) screen[y][x] = BOMB_PUSH;
		if (r >= 90 && r < 94) screen[y][x] = HEALTH_UP;
		if (r >= 94 && r < 98) screen[y][x] = WALL_HACK;
		if (r >= 98) screen[y][x] = IMMORTAL;		
	}
	else if (mode == 2)
	{
		if (r < 50) screen[y][x] = BOMBS_UP;
		if (r >= 50 && r < 90) screen[y][x] = RANGE_UP;
		if (r >= 90) screen[y][x] = BOMB_PUSH;
	}
}
struct BombList* get_bomb(int y, int x)
{
	struct BombList *b = blist_front;
	while (b != NULL)
	{
		if (b->bomb->y == y && b->bomb->x == x) 
		{
			return b;
			break;
		}
		b = b->next;
	}
	return b;
}
struct FireList* get_fire(int y, int x)
{
	struct FireList *f = flist_front;
	while (f != NULL)
	{
		if (f->y == y && f->x == x) 
		{
			return f;
			break;
		}
		f = f->next;
	}
	return f;
}
bool can_move(Player *player) {
	return player->last_move + player->speed <= iter_time;
}
bool can_pass(Player *player, int x)
{
	switch (x)
	{
	case STONE_WALL:
	case BOMB:
		return FALSE;
	case WALL:
	case WALL_ON_FIRE:
		if (player->powers & 0x10) return TRUE;
		else return FALSE;
	default:
		return TRUE;
	}
}
void move_logic(Player *player, int ydir, int xdir)
{
	int y = player->y + ydir, x = player->x + xdir;
	struct BombList *b;

	if (screen[y][x] == 4 && (player->powers & 0x1))
	{
		b = get_bomb(y, x);
		b->bomb->ydir = ydir;
		b->bomb->xdir = xdir;
		b->bomb->last_move = 0;
	}

	if (can_pass(player, screen[y][x]))
	{
		player->y = y;
		player->x = x;
		if (player->type == 0 && screen[y][x] >= POWER_START) power_time(player);
	}
}
int do_action(Player *player)
{
	struct BombList *b;

	switch (player->action)
    {
	case 1:
        move_logic(player, 0, 1);
        break;
    case 2:
        move_logic(player, 1, 0);
		break;
    case 3:
        move_logic(player, 0, -1);
        break;
    case 4:
        move_logic(player, -1, 0);
        break;
    case 5:
		if (player->bombs)
		{
			switch (screen[player->y][player->x])
			{
			case 0:
				player->bombs--;
				screen[player->y][player->x] = BOMB;

				/* Enqueue bomb */
				b = (struct BombList*) malloc(sizeof(struct BombList));
				b->bomb = (Bomb*) malloc(sizeof(Bomb));
				b->bomb->owner = player;
				b->bomb->range = player->bomb_range;
				b->bomb->end_time = iter_time + BOMB_TIME * CLOCKS_PER_SEC;
				b->bomb->x = player->x;
				b->bomb->y = player->y;
				b->bomb->xdir = 0;
				b->bomb->ydir = 0;
				b->next = NULL;
				if (blist_rear == NULL) 
				{
					blist_front = b;
					b->prev = NULL;
				}
				else 
				{
					blist_rear->next = b;
					b->prev = blist_rear;
				}
				blist_rear = b;
				break;
			case 5:
				boom(player->y, player->x, player->bomb_range);
			}
        }
    }
	return 0;
}
void fire_queue(int y, int x)
{
	struct FireList *f = (struct FireList*) malloc(sizeof(struct FireList));
	f->x = x;
	f->y = y;
	f->end_time = iter_time + FIRE_TIME * CLOCKS_PER_SEC;
	f->next = NULL;
	if (flist_rear == NULL) 
	{
		flist_front = f;
		f->prev = NULL;
	}
	else 
	{
		flist_rear->next = f;
		f->prev = flist_rear;
	}
	flist_rear = f;
}
void recycle_bomb(struct BombList *b)
{
	if (b->bomb->owner) b->bomb->owner->bombs++;
	if (b->prev == NULL) blist_front = b->next;
	else b->prev->next = b->next;
	if (b->next == NULL) blist_rear = b->prev;
	else b->next->prev = b->prev;
	free(b->bomb);
	free(b);
}
int xplosion_logic(int y, int x)
{
	Player *player;
	struct BombList *b;
	struct FireList *f;

	switch (screen[y][x])
	{
	case EMPTY:
		screen[y][x] = FIRE;
		fire_queue(y, x);
		return 1;
	case WALL:
		fire_queue(y, x);
		screen[y][x] = WALL_ON_FIRE;
		break;
	case BOMB:
		b = get_bomb(y, x);
		if (b->bomb->end_time > iter_time + 50) b->bomb->end_time = iter_time + 50;
		break;
	case FIRE:
		f = get_fire(y, x);
		f->end_time = iter_time + FIRE_TIME * CLOCKS_PER_SEC;

		/* "We should take this fire...
			and push it somewhere else!" */
		if (f->prev == NULL) flist_front = f->next;
		else f->prev->next = f->next;
		if (f->next == NULL) flist_rear = f->prev;
		else f->next->prev = f->prev;
		
		f->next = NULL;
		f->prev = flist_rear;
		if (flist_rear == NULL) flist_front = f;
		else flist_rear->next = f;
		flist_rear = f;
		return 1;
	case EXIT:
		player = (Player*) malloc(sizeof(Player));
		player_queue(player);
		player->id = -1;
		player->type = 1;
		player->y = y;
		player->x = x;
		player->health = 1;
		player->bombs = 0;
		player->bomb_range = 0;
		player->immortal = TRUE;
		player->immortal_end = iter_time + IMMORTAL_TIME * CLOCKS_PER_SEC;
		player->powers = 0;
		player->last_move = 0;
		player->action = 0;
		if (can_pass(player, screen[y][x + 1])) player->last_action = 1;
		else if (can_pass(player, screen[y][x = 1])) player->last_action = 3;
		else if (can_pass(player, screen[y + 1][x])) player->last_action = 2;
		else player->last_action = 4;
		player->speed = 500;
		break;
	case BOMBS_UP: /* Power-ups getting destroyed :'( */
	case RANGE_UP:
	case HEALTH_UP:
	case WALL_HACK:
	case IMMORTAL:
	case BOMB_PUSH:
		fire_queue(y, x);
		screen[y][x] = POWER_ON_FIRE;
		break;
	}
	return 0;
}
void boom(int y, int x, int range)
{
	int i;
	
	if (sdon) PlaySound(TEXT("sounds/boom.wav"), NULL, SND_ASYNC | SND_FILENAME);

	for (i = 0; i <= range; i++)
	{
		if (xplosion_logic(y, x + i)) continue;
		break;
	}
	for (i = 1; i <= range; i++)
	{
		if (xplosion_logic(y + i, x)) continue;
		break;
	}
	for (i = 1; i <= range; i++)
	{
		if (xplosion_logic(y, x - i)) continue;
		break;
	}
	for (i = 1; i <= range; i++)
	{
		if (xplosion_logic(y - i, x)) continue;
		break;
	}
}
void pinata(void)
{
	int empty_spots = 0, i, j, py, px;
	struct PlayerList *p;

	for (i = 1; i < m - 1; i++)
		for (j = 1; j < n - 1; j++)
			if (screen[i][j] == EMPTY) empty_spots++;
	empty_spots = empty_spots > 5 ? 5 : empty_spots; // needs fixin!
	for (i = 0; i < empty_spots; i++)
	{
		do
		{
				py = rand()%(m - 2) + 1;
				px = rand()%(n - 2) + 1;
		}
		while (screen[py][px] != EMPTY);
		gen_power(py, px);
						
	}
	p = plist_front;
	while (p != NULL)
	{
		if(p->player->type == 0 && screen[p->player->y][p->player->x] >= POWER_START) power_time(p->player);
		p = p->next;
	}
}
void player_action(int ch, int num_players, int num_bots)
{
	struct PlayerList *p;

	p = plist_front;
	while (p != NULL)
	{
		p->player->action = 0;

		if (p->player->id <= num_players)//can_move(ppom->player)
		{
			if(p->player->id == 1)
				switch (ch)
				{
				case KEY_RIGHT:
					p->player->action = 1;
					break;
				case KEY_DOWN:
					p->player->action = 2;
					break;
				case KEY_LEFT:
					p->player->action = 3;
					break;
				case KEY_UP:
					p->player->action = 4;
					break;
				case ' ':
					p->player->action = 5;
					break;
				}
			if(p->player->id == 2 || num_players == 1)
				switch (ch)
				{
				case 'd':
					p->player->action = 1;
					break;
				case 's':
					p->player->action = 2;
					break;
				case 'a':
					p->player->action = 3;
					break;
				case 'w':
					p->player->action = 4;
					break;
				case 'g':
					p->player->action = 5;
					break;
				}
			if (p->player->action && p->player->action != 5) p->player->last_move = iter_time;
		}
			
		if (p->player->id > num_players && ((mode == 2 && p->player->id <= num_players + num_bots) || (mode == 1 && p->player->type == 0 && p->player->id > 4)) && can_move(p->player))
		{
			bot_action(p->player);
			if (p->player->action && p->player->action != 5) p->player->last_move = iter_time;
		}

		if (p->player->type > 0 && can_move(p->player))
		{
			mob_action(p->player);
			if (p->player->action) 
			{
				p->player->last_move = iter_time;
				p->player->last_action = p->player->action;
			}
		}

		p = p->next;
	}
}
void player_update(void)
{
	struct BombList *b;
	struct PlayerList *p, *pp, *mb;
	bool on_mob;

	p = plist_front;
	while (p != NULL)
	{
		if (p->player->action) 
			do_action(p->player);
		
		on_mob = FALSE;
		if (mode == 1 && p->player->id == 1)
		{
			mb = plist_front;
			while (mb)
			{
				if (mb->player->type > 0 && mb->player->y == p->player->y && mb->player->x == p->player->x)
					on_mob = TRUE;
				mb = mb->next;
			}
		}

		if (p->player->immortal && p->player->immortal_end <= iter_time) p->player->immortal = FALSE;
		if (((screen[p->player->y][p->player->x] == FIRE || on_mob) && p->player->immortal == FALSE) || screen[p->player->y][p->player->x] == STONE_WALL)
		{
			p->player->health--;
			p->player->immortal = TRUE;
			p->player->immortal_end = iter_time + IMMORTAL_TIME * CLOCKS_PER_SEC;
			if (p->player->health == 0 || screen[p->player->y][p->player->x] == STONE_WALL)
			{
				// death_animation();

				b = blist_front;
				while (b != NULL) 
				{
					if (b->bomb->owner && b->bomb->owner->id == p->player->id) b->bomb->owner = NULL;
					b = b->next;
				}

				if (mode == 1 && p->player->id > 0)
					points += p->player->type * 50;

				if (p->prev == NULL) plist_front = p->next;
				else p->prev->next = p->next;
				if (p->next == NULL) plist_rear = p->prev;
				else p->next->prev = p->prev;
				pp = p;
				p = p->next;
				free(pp->player);
				free(pp);
					
				/* Dropping powerups */
				if (mode == 2)
					pinata();

				continue;
			}
		}

		p = p->next;
	}
}
void bomb_update(void)
{
	struct BombList *b, *bb;
	struct PlayerList *p;

	b = blist_front;
	while (b != NULL)	
	{
		if (b->bomb->end_time <= iter_time)
		{
			screen[b->bomb->y][b->bomb->x] = EMPTY;
			boom(b->bomb->y, b->bomb->x, b->bomb->range);
			bb = b->next;
			recycle_bomb(b);
			b = bb;
		}
		else 
		{   // UNDER CONSTRUCTION
			if ((b->bomb->ydir || b->bomb->xdir) && b->bomb->last_move + 10 <= iter_time)
			{
				switch (screen[b->bomb->y + b->bomb->ydir][b->bomb->x + b->bomb->xdir])
				{
				case 0:
					p = plist_front;
					while (p != NULL)
					{
						if(p->player->y == b->bomb->y + b->bomb->ydir && p->player->x == b->bomb->x + b->bomb->xdir) break;
						p = p->next;
					}
					if (p == NULL)
					{
						screen[b->bomb->y][b->bomb->x] = 0;
						b->bomb->y += b->bomb->ydir;
						b->bomb->x += b->bomb->xdir;
						screen[b->bomb->y][b->bomb->x] = 4;
						b->bomb->last_move = iter_time;
					}
					else
					{
						b->bomb->ydir = 0;
						b->bomb->xdir = 0;
					}
					b = b->next;
					break;
				case 5:
					screen[b->bomb->y][b->bomb->x] = EMPTY;
					boom(b->bomb->y + b->bomb->ydir, b->bomb->x + b->bomb->xdir, b->bomb->range);
					bb = b->next;
					recycle_bomb(b);
					b = bb;
					break;
				default:
					b->bomb->ydir = 0;
					b->bomb->xdir = 0;
					b = b->next;
					break;
				}
			}
			else b = b->next;
		}
	}
}
void fire_update(void)
{
	struct PlayerList *p;
	struct FireList *f;

	f = flist_front;
	while (f != NULL && f->end_time <= iter_time)
	{
		/* Fire disposal */
		if (screen[f->y][f->x] == WALL_ON_FIRE)
		{
			if (mode == 1 && (exity == f->y && exitx == f->x))
				screen[f->y][f->x] = EXIT;
			else if (rand() % 100 < POWER_CHANCE) 
			{
				gen_power(f->y, f->x);
					
				p = plist_front;
				while (p != NULL)
				{
					if(p->player->type == 0 && screen[p->player->y][p->player->x] >= POWER_START) power_time(p->player);
					p = p->next;
				}
			}
			else screen[f->y][f->x] = EMPTY;
		}
		else screen[f->y][f->x] = EMPTY;

		flist_front = f->next;
		if (flist_front == NULL) flist_rear = NULL;
		else flist_front->prev = NULL;
		free(f);
		f = flist_front;
	}
}
void wod_update(struct WallOfDeath *w)
{
	struct BombList *b;
	struct FireList *f;

	while (!w->wodstop && w->wod_start <= iter_time && w->wod_last + 0.2 * CLOCKS_PER_SEC <= iter_time)
	{
		switch (w->woddir)
		{
		case 1:
			if (w->wodx < n - 2 - w->wodinc) w->wodx++;
			else
			{
				w->woddir = 2;
				w->wody++;
			}
			break;
		case 2:
			if (w->wody < m - 2 - w->wodinc) w->wody++;
			else
			{
				w->woddir = 3;
				w->wodx--;
				w->wodinc++;
			}
			break;
		case 3:
			if (w->wodx > 1 + w->wodinc) w->wodx--;
			else
			{
				if (w->wodinc == 2) 
					w->wodstop = TRUE;
				else
				{
					w->woddir = 4;
					w->wody--;
				}
			}
			break;
		case 4:
			if (w->wody > 1 + w->wodinc) w->wody--;
			else
			{
				w->woddir = 1;
				w->wodx++;
			}
			break;
		}

		if (screen[w->wody][w->wodx] == 2) continue;

		switch (screen[w->wody][w->wodx])
		{
		case BOMB:
			b = get_bomb(w->wody, w->wodx);
			recycle_bomb(b);
			break;
		case FIRE:
		case WALL_ON_FIRE:
		case POWER_ON_FIRE:
			f = get_fire(w->wody, w->wodx);
			if (f->prev == NULL) flist_front = f->next;
			else f->prev->next = f->next;
			if (f->next == NULL) flist_rear = f->prev;
			else f->next->prev = f->prev;
			free(f);
		}
		screen[w->wody][w->wodx] = STONE_WALL;

		w->wod_last = iter_time;

		break;
	}
}
void free_stuff(void)
{
	int i;
	struct BombList *b;
	struct PlayerList *p;
	struct FireList *f;

	while (blist_front != NULL)
	{
		b = blist_front;
		blist_front = blist_front->next;
		free(b->bomb);
		free(b);
	}
	while (plist_front != NULL)
	{
		p = plist_front;
		plist_front = plist_front->next;
		free(p->player);
		free(p);
	}
	while (flist_front != NULL)
	{
		f = flist_front;
		flist_front = flist_front->next;
		free(f);
	}

    for (i = 0; i < m; i++)
    {
        free(screen[i]);
    }
    free(screen);
}
int campaign(void)
{
	/* ~Initialization~ */
    int ch, lives = 5, level = 1, win;
	bool running;
	
	blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, mlist_front = NULL, mlist_rear = NULL, flist_front = NULL, flist_rear = NULL;
	
	mode = 1, m = 11, n = 15, per = 50;
	bombs = 1, range = 1, powers = 0, health = 1;
	points = 0;

	init_screen(m, n, 1);
	create_map(level);
	init_players(1, 0);
	//
	plist_front->player->health = health;
	plist_front->player->bombs = bombs;
	plist_front->player->bomb_range = range;
	plist_front->player->powers = powers;
	//
	init_mobs(level);
	spawn_exit(level);
	draw(screen, blist_front, plist_front, mlist_front);
	
	time_end = clock() + STORY_TIME * CLOCKS_PER_SEC;

	while(1)
	{
		win = -1;
		running = TRUE;

		plist_front->player->immortal = TRUE;
		plist_front->player->immortal_end = clock() + IMMORTAL_TIME * CLOCKS_PER_SEC;

		/* ~The Game Loop!~ */
		while (running)
		{
			iter_time = clock();
		
			ch = wgetch(game_win);

			/* Function keys */
			switch (ch)
			{
			case 27:
				running = FALSE;
				break;
			case 10:
				pause(&running);
			}
			
			player_action(ch, 1, level % 5 == 0);
			player_update();
			bomb_update();
			fire_update();

			/* Game over? */
			if (plist_front == NULL || plist_front->player->id != 1 || time_end <= iter_time)
			{
				win = 0;
				running = FALSE;
			}
			else if (plist_front->next == NULL && (screen[plist_front->player->y][plist_front->player->x] == EXIT || level % 5 == 0))
			{
				win = 1;
				running = FALSE;
			}
					
			draw(screen, blist_front, plist_front, mlist_front);

			if (clock() - iter_time <= 33) Sleep(33 - (clock() - iter_time)); // 30 FPS
		}

		if (win == -1) break;
		else if (win) 
		{
			points += (time_end - iter_time) * 10;
			free_stuff();

			level++;
			//transition();

			if (level == 21)
			{
				points += 500;
				// HUZZAH! GAME COMPLETE!
				break;
			}
			else
			{
				blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, mlist_front = NULL, mlist_rear = NULL, flist_front = NULL, flist_rear = NULL;
				
				if (level == 6) init_screen(m, n, 2);
				//if (level == 11) init_screen(m, n, 3);
				//if (level == 16) init_screen(m, n, 4);
				create_map(level);
			
				// Boss?
				if (level % 5 == 0)
				{
					init_players(1, 1);
					plist_front->player->y = m - 4;
					plist_front->player->x = n / 2;

					plist_front->next->player->id = 4 + level / 5;
					plist_front->next->player->y = 3;
					plist_front->next->player->x = n / 2;
					plist_front->next->player->health = 2;
					plist_front->next->player->bombs = 2 + level / 5; 
					plist_front->next->player->bomb_range = 1 + level / 5;
				}
				else init_players(1, 0);
								
				plist_front->player->health = health;
				plist_front->player->bombs = bombs;
				plist_front->player->bomb_range = range;
				plist_front->player->powers = powers;

				init_mobs(level);
				spawn_exit(level);
				draw(screen, blist_front, plist_front, mlist_front);
	
				time_end = clock() + STORY_TIME * CLOCKS_PER_SEC;
			}
		}
		else
		{
			if (lives) 
			{
				lives--;
				health = 1;
				powers = 0;
				init_players(1, 0);

				// move to front
				plist_rear->next = plist_front;
				plist_front->prev = plist_rear;
				plist_front = plist_front->prev;
				plist_rear = plist_rear->prev;
				plist_rear->next = NULL;
				plist_front->prev = NULL;

				plist_front->player->health = health;
				plist_front->player->bombs = bombs;
				plist_front->player->bomb_range = range;
				plist_front->player->powers = powers;
			}
			else 
			{
				// GAME OVER
				break;
			}
		}
	}
	
	free_stuff();
	del_stuff();
	
	clear();
	refresh();
    return 0;
}
int battle(int num_players, int num_bots, int req_wins)
{
    /* ~Initialization~ */
    int ch, winner, arena = rand() % 2 + 1;
	bool running;
	
	struct WallOfDeath *w = NULL;

	int *scores = (int*) calloc(num_players + num_bots, sizeof(int));

	mode = 2;
	
	if (num_players + num_bots > 4) m = 13, n = 17, per = 80;
	else m = 11, n = 15, per = 60;

	while (1)
	{
		winner = -1;
		running = TRUE;
		
		blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, mlist_front = NULL, mlist_rear = NULL, flist_front = NULL, flist_rear = NULL;
	
		init_screen(m, n, arena);
		create_map(0);
		w = init_wod(w);
		init_players(num_players, num_bots);
		draw(screen, blist_front, plist_front, mlist_front);
 
		time_end = clock() + GAME_TIME * CLOCKS_PER_SEC;
		w->wod_start = time_end - WOD_TIME * CLOCKS_PER_SEC;

		/* ~The Game Loop!~ */
		while (running)
		{
			iter_time = clock();
		
			ch = wgetch(game_win);

			/* Function keys */
			switch (ch)
			{
			case 27:
				running = FALSE;
				break;
			case 10:
				pause(&running);
			}

			player_action(ch, num_players, num_bots);
			player_update();
			bomb_update();
			fire_update();
			wod_update(w);

			/* Game over? */
			if (plist_front == NULL || plist_front->next == NULL || time_end <= iter_time)
			{
				running = FALSE;
				w->wodstop = TRUE;

				if (plist_front == NULL || time_end <= iter_time) winner = 0; // no winner
				else if (plist_front->next == NULL)
				{
					winner = plist_front->player->id;
					plist_front->player->immortal = TRUE;
					plist_front->player->immortal_end = iter_time + 3600 * CLOCKS_PER_SEC;
				}
			}
		
			draw(screen, blist_front, plist_front, mlist_front);

			if (clock() - iter_time <= 33) Sleep(33 - (clock() - iter_time)); // 30 FPS
		}

		/* Game over */
		// transition();
		free_stuff();

		if (winner == -1) break;
		else if (winner > 0) 
		{
			scores[winner - 1]++;
			scoreboard(scores, num_players+num_bots);
			if (scores[winner - 1] == req_wins)
			{
				// CHAMPION!
				break;
			}
		}
	}

	free(scores);
	del_stuff();

	clear();
	refresh();
    return 0;
}