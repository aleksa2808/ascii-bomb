#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include "structs.h"
#include "functions.h"

char **screen;
struct BombList *blist_front, *blist_rear;
struct PlayerList *plist_front, *plist_rear;
struct FireList *flist_front, *flist_rear;
int time_end, iter_time, m, n, per, mode, difficulty;
WallOfDeath *w;

// for lack of a better global // story mode stuff
int lives, points, health, bombs, range, powers, exitx, exity, exit_spawn = 0;
extern WINDOW *hud_win;
extern WINDOW *game_win;
extern void bot_action(Player*, int);
extern void mob_action(Player*);
extern int sdon;
extern int transon;

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
		if (transon){
			draw(screen,NULL,NULL);
			Sleep(15);
		}
    }
    for (i = 0; i < n; i++)
    {
		screen[0][i] = STONE_WALL;
		screen[m - 1][i] = STONE_WALL;
		if (transon){
			draw(screen,NULL,NULL);
			Sleep(15);
		}
    }
       
    /* Blocks */
	if (level != -1)
		for (i = 2; i < m - 1; i += 2)
			for (j = 2; j < n - 1; j += 2){
				screen[i][j] = STONE_WALL;
				if (transon){
					draw(screen,NULL,NULL);
					Sleep(15);
				}
			}
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
			if (transon){
				draw(screen,NULL,NULL);
				Sleep(15);
			}
		}
	}
	flushinp();
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
WallOfDeath* init_wod(WallOfDeath *w)
{
	w = (WallOfDeath*) malloc(sizeof(WallOfDeath));
	w->y = m - 1;
	w->x = 1;
	w->dir = 4;
	w->inc = 0;
	w->last_move = clock();
	w->alive = FALSE;
	return w;
}
void spawn(int id, Player *player, int y, int x)
{
	if (screen[y][x] == WALL) screen[y][x] = EMPTY;
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
	switch (difficulty)
	{
	case 1:
		player->speed = 0.3 * TIME_SLICE;
		break;
	case 2:
		player->speed = 0.25 * TIME_SLICE;
		break;
	case 3:
		player->speed = 0.2 * TIME_SLICE;
		break;
	}
	
	strcpy(player->gene, "000000000000000000000000000000000000000000000000000000000000"); // CHROMO_LENGTH!!!
	strcpy(player->code, "03614257"); // subject to change
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
	int i, xi, yi;
	Player *player;
	int x[8] = {1, n - 2, n - 2, 1}, y[8] = {1, m - 2, 1, m - 2};
	for (i = 0; i < num_players; i++)
	{
		player = (Player*) malloc(sizeof(Player));
		player_queue(player);
		
		switch (i % 4)
		{
		case 0:
			xi = x[i % 4] + (i / 4) * 4;
			yi = y[i % 4] + (i / 4) * 2;
			break;
		case 1:
			xi = x[i % 4] - (i / 4) * 4;
			yi = y[i % 4] - (i / 4) * 2;
			break;
		case 2:
			xi = x[i % 4] - (i / 4) * 4;
			yi = y[i % 4] + (i / 4) * 2;
			break;
		case 3:
			xi = x[i % 4] + (i / 4) * 4;
			yi = y[i % 4] - (i / 4) * 2;
			break;
		}

		spawn(i + 1, player, yi, xi);
	}
	for (i = 0; i < num_bots; i++)
	{
		player = (Player*) malloc(sizeof(Player));
		player_queue(player);

		switch ((num_players + i) % 4)
		{
		case 0:
			xi = x[(num_players + i) % 4] + ((num_players + i) / 4) * 4;
			yi = y[(num_players + i) % 4] + ((num_players + i) / 4) * 2;
			break;
		case 1:
			xi = x[(num_players + i) % 4] - ((num_players + i) / 4) * 4;
			yi = y[(num_players + i) % 4] - ((num_players + i) / 4) * 2;
			break;
		case 2:
			xi = x[(num_players + i) % 4] - ((num_players + i) / 4) * 4;
			yi = y[(num_players + i) % 4] + ((num_players + i) / 4) * 2;
			break;
		case 3:
			xi = x[(num_players + i) % 4] + ((num_players + i) / 4) * 4;
			yi = y[(num_players + i) % 4] - ((num_players + i) / 4) * 2;
			break;
		}

		spawn(num_players + i + 1, player, yi, xi);
	}
}
void init_mobs(int level)
{
	int i, j, dir, mob_num = level % 5 + 2 + level / 5;
	Player *player;
	int x[8] = {n - 4, n - 2, 11, 5, 1, n - 8, n - 6, 7}, y[8] = {m - 8, 1, m - 2, m - 6, 9, 3, m - 4, 7};
	int bias = rand() % 20;

	for (i = 0; i < mob_num; i++)
	{
		player = (Player*) malloc(sizeof(Player));
		player_queue(player);
		player->id = i + 2;
		player->type = i > 3 ? i > 5 ? 3 : 2 : 1;
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
		if (player->type == 3) player->powers = 0x10;
		else player->powers = 0;
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
		player->speed = 0.5 * TIME_SLICE;
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
	wattron(hud_win,COLOR_PAIR(15*16));
	mvwprintw(hud_win,6,COLS/2-3,"PAUSE");
	wattroff(hud_win,COLOR_PAIR(15*16));
	wrefresh(hud_win);
	while ((ch = wgetch(game_win)) != 10)
		if (ch == 27) 
		{
			*running = FALSE;
			break;
		}
	nodelay(game_win, TRUE);
	
	pause_time = clock() - iter_time;
	
	time_end += pause_time;
	if (w)
	{
		w->wod_start += pause_time;
		w->last_move += pause_time;
	}

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
		//player->health++;
		if (mode == 1) lives++;
		break;
	case WALL_HACK:
		player->powers |= 0x10;
		if (mode == 1) powers |= 0x10;;
		break;
	case IMMORTAL:
		player->immortal = TRUE;
		player->immortal_end = iter_time + IMMORTAL_TIME * TIME_SLICE;
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
		return FALSE;
	case BOMB:
		if (mode == 3) return TRUE;
		else return FALSE;
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
void do_action(Player *player)
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
				b->bomb->end_time = iter_time + BOMB_TIME * TIME_SLICE;
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
}
void fire_queue(int y, int x)
{
	struct FireList *f = (struct FireList*) malloc(sizeof(struct FireList));
	f->x = x;
	f->y = y;
	f->end_time = iter_time + FIRE_TIME * TIME_SLICE;
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
		f->end_time = iter_time + FIRE_TIME * TIME_SLICE;

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
		if (exit_spawn <= iter_time)
		{
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
			player->immortal_end = iter_time + IMMORTAL_TIME * TIME_SLICE;
			player->powers = 0;
			player->last_move = 0;
			player->action = 0;
			if (can_pass(player, screen[y][x + 1])) player->last_action = 1;
			else if (can_pass(player, screen[y][x = 1])) player->last_action = 3;
			else if (can_pass(player, screen[y + 1][x])) player->last_action = 2;
			else player->last_action = 4;
			player->speed = 0.5 * TIME_SLICE;

			exit_spawn = iter_time + 2 * TIME_SLICE;
		}
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
	
	if (sdon && mode != 3) PlaySound(TEXT("sounds/boom.wav"), NULL, SND_ASYNC | SND_FILENAME);

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
	empty_spots = min(empty_spots, 3);
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
			bot_action(p->player, difficulty);
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
			p->player->immortal_end = iter_time + IMMORTAL_TIME * TIME_SLICE;
			if (p->player->health == 0 || screen[p->player->y][p->player->x] == STONE_WALL)
			{
				// death_animation();

				b = blist_front;
				while (b != NULL) 
				{
					if (b->bomb->owner && b->bomb->owner->id == p->player->id) b->bomb->owner = NULL;
					b = b->next;
				}

				if (mode == 1 && p->player->id > 1)
				{
					if (p->player->type == 0) points += 200;
					else points += p->player->type * 20 + 30;
				}

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
		{
			if ((b->bomb->ydir || b->bomb->xdir) && b->bomb->last_move + 0.01 * TIME_SLICE <= iter_time)
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
void wod_update(WallOfDeath *w)
{
	struct BombList *b;
	struct FireList *f;

	if (!w->alive && w->wod_start <= iter_time) w->alive = TRUE;
	else while (w->alive && w->last_move + 0.2 * TIME_SLICE <= iter_time)
		{
			switch (w->dir)
			{
			case 1:
				if (w->x < n - 2 - w->inc) w->x++;
				else
				{
					w->dir = 2;
					w->y++;
				}
				break;
			case 2:
				if (w->y < m - 2 - w->inc) w->y++;
				else
				{
					w->dir = 3;
					w->x--;
					w->inc++;
				}
				break;
			case 3:
				if (w->x > 1 + w->inc) w->x--;
				else
				{
					if (w->inc == 2) 
						w->alive = FALSE;
					else
					{
						w->dir = 4;
						w->y--;
					}
				}
				break;
			case 4:
				if (w->y > 1 + w->inc) w->y--;
				else
				{
					w->dir = 1;
					w->x++;
				}
				break;
			}

			if (screen[w->y][w->x] == 2) continue;

			switch (screen[w->y][w->x])
			{
			case BOMB:
				b = get_bomb(w->y, w->x);
				recycle_bomb(b);
				break;
			case FIRE:
			case WALL_ON_FIRE:
			case POWER_ON_FIRE:
				f = get_fire(w->y, w->x);
				if (f->prev == NULL) flist_front = f->next;
				else f->prev->next = f->next;
				if (f->next == NULL) flist_rear = f->prev;
				else f->next->prev = f->prev;
				free(f);
			}
			screen[w->y][w->x] = STONE_WALL;

			w->last_move = iter_time;

			break;
		}
}
void update_scores(void)
{
	WINDOW *input_win;
	FILE *high_scores;
	struct ScoreList *slist = NULL, *s = NULL, *t, *p = NULL;
	int i, j, chksum, sum, k;
	char ch, *pts;

	// da li neko vara? :)
	high_scores = fopen("data/highscores.txt", "r");
	if (high_scores)
	{
		sum = 0;
		for (i = 0; i < 10; i++)
		{
			while ((ch = fgetc(high_scores)) != '\n')
			{
				sum += ch;
			}
		}
		fscanf(high_scores, "%d", &chksum);

		if (sum == chksum)
		{
			fseek(high_scores, 0, SEEK_SET);

			// citanje liste
			for (i = 0; i < 10; i++)
			{
				t = (struct ScoreList*) malloc(sizeof(struct ScoreList));

				while ((ch = fgetc(high_scores)) != ' ');

				t->name = NULL;

				j = 0;
				while ((ch = fgetc(high_scores)) != ' ')
				{
					if (j % 10 == 0) t->name = realloc(t->name, (j + 10) * sizeof(char));
					t->name[j++] = ch;
				}
				if (j % 10 == 0) t->name = realloc(t->name, (j + 1) * sizeof(char));
				t->name[j] = '\0';

				pts = NULL;

				j = 0;
				while ((ch = fgetc(high_scores)) != '\n')
				{
					if (j % 10 == 0) pts = realloc(pts, (j + 10) * sizeof(char));
					pts[j++] = ch;
				}
				if (j % 10 == 0) pts = realloc(pts, (j + 1) * sizeof(char));
				pts[j] = '\0';

				t->points = atoi(pts);
				t->next = NULL;

				if (!s) slist = t;
				else s->next = t;
				s = t;
			}
		}
		fclose(high_scores);
	}

	// ubacivanje u listu
	i = 1;
	s = slist;
	while (s && points <= s->points)
	{
		p = s;
		s = s->next;
		i++;
	}
	t = (struct ScoreList*) malloc(sizeof(struct ScoreList));

	t->name = (char*) malloc(21 * sizeof(char));
	if (i <= 10)
	{
		input_win = newwin(5, 30, LINES / 2 - 3, COLS / 2 - 15);
		box(input_win, 0, 0);
		mvwprintw(input_win, 2, 2, "Name:");
		
		j = 0;
		while ((ch = mvwgetch(input_win, 2, 8 + j)) != 10)
		{
			if (ch == 27)
			{
				j = 0;
				break;
			}

			if (ch == 8) j = max(j--, 0);
			else if (j < 20 && ch >= 32 && ch <= 126) t->name[j++] = ch == ' ' ? '_' : ch;
			t->name[j] = '\0';

			mvwprintw(input_win, 2, 2, "Name: %s", t->name);
			for (k = j; k < 20; k++) wprintw(input_win, " ");
		}
		if (j == 0) t->name = "<unnamed_player>";

		delwin(input_win);
	}
	else t->name = "newb!";

	t->points = points;
	t->next = s;
	if (!p) slist = t;
	else p->next = t;

	// ispis liste
	high_scores = fopen("data/highscores.txt", "w");
		
	s = slist;
	for (i = 1; i <= 10; i++)
	{
		if (s)
		{
			fprintf(high_scores, "%d. %s %d\n", i, s->name, s->points);
			s = s->next;
		}
		else fprintf(high_scores, "%d. ----- 0\n", i);
	}
	
	high_scores = freopen("data/highscores.txt", "r", high_scores);
	sum = 0;
	for (i = 0; i < 10; i++)
	{
		while ((ch = fgetc(high_scores)) != '\n')
		{
			sum += ch;
		}
	}

	high_scores = freopen("data/highscores.txt", "a", high_scores);
	fprintf(high_scores, "%d", sum);
	
	fclose(high_scores);
	while (slist)
	{
		s = slist;
		slist = slist->next;
		free(s);
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
    int ch, level = 1, win;
	bool running, end = FALSE;
	
	blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, flist_front = NULL, flist_rear = NULL;
	
	mode = 1, m = 11, n = 15, per = 50, difficulty = 2;
	lives = 5, bombs = 1, range = 1, powers = 0, health = 1, points = 0;

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
	draw(screen, blist_front, plist_front);
	
	time_end = clock() + STORY_TIME * TIME_SLICE;

	while(1)
	{
		win = -1;
		running = TRUE;

		plist_front->player->immortal = TRUE;
		plist_front->player->immortal_end = clock() + IMMORTAL_TIME * TIME_SLICE;

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
				break;
			case 'f':
				running = FALSE;
				win = 1;
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
					
			draw(screen, blist_front, plist_front);

			if (clock() - iter_time <= 33) Sleep(33 - (clock() - iter_time)); // 30 FPS
		}

		if (win == -1) break;
		else if (win) 
		{
			points += ((time_end - iter_time) / TIME_SLICE) * 5; // 5pts/sec

			level++;

			if (level == 16)
			{
				points += 2000;
				end = TRUE;
				// HUZZAH! GAME COMPLETE!
				break;
			}
			else
			{
				free_stuff();

				blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, flist_front = NULL, flist_rear = NULL;
				
				if (level == 6) 
				{
					del_stuff();
					init_screen(m, n, 2);
				}
				if (level == 11)
				{
					del_stuff();
					init_screen(m, n, 3);
				}
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
					plist_front->next->player->bomb_range = 2 + level / 5;
				}
				else init_players(1, 0);
								
				plist_front->player->health = health;
				plist_front->player->bombs = bombs;
				plist_front->player->bomb_range = range;
				plist_front->player->powers = powers;

				init_mobs(level);
				spawn_exit(level);
				draw(screen, blist_front, plist_front);
	
				time_end = clock() + STORY_TIME * TIME_SLICE;
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
				if (level % 5 == 0)
				{
					plist_front->player->y = m - 4;
					plist_front->player->x = n / 2;
				}
			}
			else 
			{
				// GAME OVER
				break;
			}
		}
	}

	/* ~High-score update~ */
	if (points) update_scores();
	
	free_stuff();
	del_stuff();
	
	clear();
	refresh();

	if (end) fun();
    return 0;
}
int battle(int num_players, int num_bots, int req_wins, int diff)
{
    /* ~Initialization~ */
    int ch, winner, arena = rand() % 3 + 1;
	bool running;

	int *scores = (int*) calloc(num_players + num_bots, sizeof(int));

	mode = 2;
	difficulty = diff;
	
	if (num_players + num_bots > 4) m = 13, n = 17, per = 80;
	else m = 11, n = 15, per = 60;
	init_screen(m, n, arena);

	while (1)
	{
		winner = -1;
		running = TRUE;
		
		w = NULL, blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, flist_front = NULL, flist_rear = NULL;
	
		create_map(0);
		w = init_wod(w);
		init_players(num_players, num_bots);

		draw(screen, blist_front, plist_front);
 
		// calm before the storm
		Sleep(500);

		time_end = clock() + GAME_TIME * TIME_SLICE;
		w->wod_start = time_end - (GAME_TIME / 2) * TIME_SLICE;

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
				if (num_players) pause(&running);
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
				w->alive = FALSE;

				if (plist_front == NULL || time_end <= iter_time) winner = 0; // no winner
				else if (plist_front->next == NULL)
				{
					winner = plist_front->player->id;
					plist_front->player->immortal = TRUE;
					plist_front->player->immortal_end = iter_time + 3600 * TIME_SLICE;
				}
			}
		
			draw(screen, blist_front, plist_front);

			if (clock() - iter_time <= 33) Sleep(33 - (clock() - iter_time)); // 30 FPS
		}

		/* Game over */
		// transition();
		free_stuff();

		if (winner == -1) break;
		else if (winner > 0) 
		{
			scores[winner - 1]++;
			scoreboard(scores, num_players+num_bots, winner);
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

char* random_bits(void)
{
	int i;
	char bits[CHROMO_LENGTH + 1];

	for (i = 0; i < CHROMO_LENGTH; i++)
	{
		if (RANDOM_NUM > 0.5f) bits[i] = '1';
		else bits[i] = '0';
	}
	bits[CHROMO_LENGTH] = '\0';

	return bits;
}
char bin_to_dec(char bits[GENE_LENGTH])
{
	int i, val = 0, value_to_add = 1;

	for (i = GENE_LENGTH; i > 0; i--)
	{
		if (bits[i - 1] == '1') val += value_to_add;
		value_to_add *= 2;
	}

	return val + '0';
}
char* decode_gene(char gene[CHROMO_LENGTH + 1])
{
	int i, j, k, l, dec;
	bool f;
	char code[(CHROMO_LENGTH / GENE_LENGTH) + 1], buffer[GENE_LENGTH];
	
	for (i = 0, j = 0, k = 0; i < CHROMO_LENGTH; i++)
	{
		buffer[j++] = gene[i];
		if (j == GENE_LENGTH)
		{
			dec = bin_to_dec(buffer);

			f = TRUE;
			for (l = 0; l < k; l++)
			{
				if (code[l] == dec)
				{
					f = FALSE;
					break;
				}
			}

			if (f) code[k++] = dec;
			j = 0;
		}
	}
	
	code[k] = '\0';
	return code;
}
void mutate(char* bits)
{
	int i;
	for (i = 0; i < CHROMO_LENGTH; i++)
	{
		if (RANDOM_NUM < MUTATION_RATE)
		{
			if (bits[i] == '1')
				bits[i] = '0';
			else
				bits[i] = '1';
		}
	}
}
void crossover(char* offspring1, char* offspring2)
{
	int cross, i;
	char t;

	if (RANDOM_NUM < CROSSOVER_RATE)
	{
		cross = (int) (RANDOM_NUM * CHROMO_LENGTH);
	
		for (i = cross; i < CHROMO_LENGTH; i++)
		{
			t = offspring1[i];
			offspring1[i] = offspring2[i];
			offspring2[i] = t;
		}			  
	}
}
char* roulette(int total_fitness, char population[POP_SIZE][CHROMO_LENGTH + 1], int* scores)
{
	int i;
	float fitness = 0.0f, slice = (float)(RANDOM_NUM * total_fitness);

	for (i = 0; i < POP_SIZE; i++)
	{
		fitness += scores[i];
		
		if (fitness >= slice)
			return population[i];
	}

	return "";
}
int fact(int x)
{
	int res = x;
	while (--x) res *= x;
	return res;
}
int training_area(void)
{
	/* ~Initialization~ */
	int i, num, cpop, generation = 1, max;
	float total_fitness;
	char offspring1[CHROMO_LENGTH + 1], offspring2[CHROMO_LENGTH + 1], population[POP_SIZE][CHROMO_LENGTH + 1], temp[POP_SIZE][CHROMO_LENGTH + 1], fittest[(CHROMO_LENGTH / GENE_LENGTH) + 1];

    int ch, arena = rand() % 3 + 1, winner, tasty_pie = fact(POP_SIZE), slice_o_pie;
	bool running;
	struct PlayerList *p;

	int *scores = (int*) calloc(POP_SIZE, sizeof(int));
	
	w = NULL, blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, flist_front = NULL, flist_rear = NULL;

	mode = 2;
	difficulty = 3;
	
	m = 13, n = 17, per = 80;
	init_screen(m, n, arena);
    update_hud(generation, "0", 0);


	for (i = 0; i < POP_SIZE; i++) strcpy(population[i], random_bits());

	while (1)
	{
		winner = -1;
		running = TRUE;
		
		w = NULL, blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, flist_front = NULL, flist_rear = NULL;

		create_map(0);
		w = init_wod(w);
		init_players(0, POP_SIZE);

		p = plist_front;
		for (i = 0; i < POP_SIZE; i++)
		{
			strcpy(p->player->gene, population[i]);
			strcpy(p->player->code, decode_gene(population[i]));

			p = p->next;
		}

		draw(screen, blist_front, plist_front);
 
		time_end = clock() + GAME_TIME * TIME_SLICE;
		w->wod_start = time_end - (GAME_TIME / 2) * TIME_SLICE;

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
				break;
			case 'r':
				running = FALSE;
				winner = 0;
			}

			player_action(ch, 0, POP_SIZE);
			player_update();
			bomb_update();
			fire_update();
			wod_update(w);

			/* Game over? */
			if (plist_front == NULL || plist_front->next == NULL || time_end <= iter_time)
			{
				running = FALSE;
				w->alive = FALSE;

				if (plist_front == NULL || time_end <= iter_time) winner = 0; // no winner
				else if (plist_front->next == NULL)
				{
					winner = plist_front->player->id;
					plist_front->player->immortal = TRUE;
					plist_front->player->immortal_end = iter_time + 3600 * TIME_SLICE;
				}
			}
		
			draw(screen, blist_front, plist_front);

			if (clock() - iter_time <= 33) Sleep(33 - (clock() - iter_time)); // 30 FPS
		}

		if (winner == -1) break;
		else 
		{
			if (winner) scores[winner - 1] += tasty_pie;
			else if (p = plist_front)
			{
				num = 0;
				while (p)
				{
					num++;
					p = p->next;
				}

				slice_o_pie = tasty_pie / num;

				p = plist_front;
				while (p)
				{
					scores[p->player->id - 1] += slice_o_pie;
					p = p->next;
				}
			}

			max = 0;
			for (i = 0; i < POP_SIZE; i++)
			{
				if (scores[i] >= max) 
				{
					max = scores[i];
					strcpy(fittest, decode_gene(population[i]));
				}
			}
		
			update_hud(generation, fittest, max);

			if (max > 5 * tasty_pie)
			{
				// CHAMPION!
				
				generation++;

				/* Breeding a new generation */
				total_fitness = 0;
				for (i = 0; i < POP_SIZE; i++) total_fitness += scores[i];

				cpop = 0;
	  
				while (cpop < POP_SIZE)
				{
					strcpy(offspring1, roulette(total_fitness, population, scores));
					strcpy(offspring2, roulette(total_fitness, population, scores));

					crossover(offspring1, offspring2);
	
					mutate(offspring1);
					mutate(offspring2);

					strcpy(temp[cpop++], offspring1);
					strcpy(temp[cpop++], offspring2);
				}

				for (i = 0; i < POP_SIZE; i++) strcpy(population[i], temp[i]);

				// organizin' a new tournament
				for (i = 0; i < POP_SIZE; i++) scores[i] = 0;
				arena = arena % 3 + 1;

				del_stuff();
				init_screen(m, n, arena);
			}
		}

		free_stuff();
	}

	free(scores);
	del_stuff();
	free_stuff();
	clear();
	refresh();
    return 0;
}

int fun(void)
{
	/* ~Initialization~ */
    int i, j, ch, arena = rand() % 3 + 1, last_wave = 0, speed = 250;
	bool running;

	struct BombList *b, *bb;

	FILE *input = fopen("data/fun.txt", "r");

	char mat[7][90];

	for (i = 0; i < 7; i++)
	{
		for (j = 0; j < 90; j++)
		{
			mat[i][j] = fgetc(input);
		}
		fgetc(input);
	}

	fclose(input);
	
	if (sdon) PlaySound(TEXT("sounds/what_is_f.wav"), NULL, SND_ASYNC | SND_FILENAME);

	mode = 3;
	
	m = 9, n = 15, per = 0;
	init_screen(m, n, arena);
	j = -10;

	running = TRUE;
		
	w = NULL, blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, flist_front = NULL, flist_rear = NULL;
	
	create_map(-1);
	init_players(1, 0);
	plist_front->player->y = 4;
	plist_front->player->x = 2;

	draw(screen, blist_front, plist_front);

	/* ~The Game Loop!~ */
	while (running)
	{
		iter_time = clock();
		
		ch = wgetch(game_win);

		/* Function keys */
		switch (ch)
		{
		case 27:
			if (sdon) PlaySound(TEXT("sounds/boom.wav"), NULL, SND_ASYNC | SND_FILENAME);
			running = FALSE;
			break;
		}

		plist_front->player->action = 0;
		switch (ch)
		{
		case KEY_RIGHT:
			plist_front->player->action = 1;
			break;
		case KEY_DOWN:
			plist_front->player->action = 2;
			break;
		case KEY_LEFT:
			plist_front->player->action = 3;
			break;
		case KEY_UP:
			plist_front->player->action = 4;
			break;
		}

		if (plist_front->player->action) do_action(plist_front->player);

		if (plist_front->player->immortal && plist_front->player->immortal_end < iter_time) plist_front->player->immortal = FALSE;

		b = blist_front;
		while (b != NULL)	
		{
			if ((b->bomb->ydir || b->bomb->xdir) && b->bomb->last_move + speed <= iter_time)
			{
				switch (screen[b->bomb->y + b->bomb->ydir][b->bomb->x + b->bomb->xdir])
				{
				case EMPTY:
					screen[b->bomb->y][b->bomb->x] = EMPTY;
					b->bomb->y += b->bomb->ydir;
					b->bomb->x += b->bomb->xdir;
					screen[b->bomb->y][b->bomb->x] = BOMB;
					b->bomb->last_move = iter_time;
					b = b->next;
					break;
				case STONE_WALL:
					screen[b->bomb->y][b->bomb->x] = EMPTY;
					bb = b->next;
					recycle_bomb(b);
					b = bb;
					break;
				}
			}
			else b = b->next;
		}

		if (last_wave + speed <= iter_time)
		{
			if (j >= 0)
				for (i = 0; i < 7; i++)
				{
					if (mat[i][j] == '1')
					{
						screen[i + 1][13] = BOMB;

						/* Enqueue bomb */
						b = (struct BombList*) malloc(sizeof(struct BombList));
						b->bomb = (Bomb*) malloc(sizeof(Bomb));
						b->bomb->owner = NULL;
						b->bomb->range = 3;
						b->bomb->y = i + 1;
						b->bomb->x = 13;
						b->bomb->xdir = -1;
						b->bomb->ydir = 0;
						b->bomb->last_move = clock();
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
					}
				}
		
			if (++j == 90)
			{
				j = 0;
				plist_front->player->id++;
				plist_front->player->immortal = TRUE;
				plist_front->player->immortal_end = iter_time + 2 * TIME_SLICE;
				speed -= 30;
				if (speed < 0) speed = 0;
			}

			last_wave = iter_time;
		}

		/* Game over? */
		if (screen[plist_front->player->y][plist_front->player->x] == BOMB) 
		{
			if (sdon) PlaySound(TEXT("sounds/boom.wav"), NULL, SND_ASYNC | SND_FILENAME);
			
			b = blist_front;
			while (b != NULL)	
			{
				screen[b->bomb->y][b->bomb->x] = EMPTY;
				boom(b->bomb->y, b->bomb->x, b->bomb->range);
				bb = b->next;
				recycle_bomb(b);
				b = bb;
			}
			draw(screen, blist_front, plist_front);
			Sleep(500);
			running = FALSE;
		}
		
		draw(screen, blist_front, plist_front);

		if (clock() - iter_time <= 33) Sleep(33 - (clock() - iter_time)); // 30 FPS
	}

	/* Game over */
	free_stuff();

	del_stuff();

	clear();
	refresh();
    return 0;
}