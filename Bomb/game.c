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

#define POWER_CHANCE 10

#define POWER_START 11
#define BOMBS_UP 11
#define RANGE_UP 12
#define HEALTH_UP 13
#define WALL_HACK 14
#define IMMORTAL 15

#define GAME_TIME 120
#define WOD_TIME 60
#define BOMB_TIME 2
#define FIRE_TIME 0.5
#define IMMORTAL_TIME 4

typedef struct {
	int id, x, y, health, bombs, bomb_range, action, immortal_end, last_move;
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

char **screen;
struct BombList *blist_front, *blist_rear;
struct PlayerList *plist_front, *plist_rear;
struct FireList *flist_front, *flist_rear;
int iter_time, m = 13, n = 17, per = 80, num_players = 1, num_bots = 7;
extern WINDOW *game_win;

extern void init_screen(int, int);
extern void draw(char**, struct BombList*, struct PlayerList*);
extern void del_stuff(void);

extern void bot_action(Player*);

void boom(int y, int x, int range);

void create_map(int m, int n, int per)
{
	int i, j, fill, mx, my;

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
    fill = (((m - 2) / 2 + 1) * (n - 2) + ((m - 2) / 2) * ((n - 2) / 2 + 1)); // Number of empty blocks
    fill = fill * (per/100.0);
    for (i = 0; i < fill; i++)
    {
            do
            {
                    my = rand()%(m - 2) + 1;
                    mx = rand()%(n - 2) + 1;
            }
            while (screen[my][mx] != 0);
            screen[my][mx] = WALL;
    }
}
void spawn(int id, Player *player, int y, int x)
{
	screen[y][x] = EMPTY;
	if (screen[y][x + 1] != 2) screen[y][x + 1] = EMPTY;
	if (screen[y + 1][x] != 2) screen[y + 1][x] = EMPTY;
	if (screen[y][x - 1] != 2) screen[y][x - 1] = EMPTY;
	if (screen[y - 1][x] != 2) screen[y - 1][x] = EMPTY;

	player->id = id;
	player->y = y;
	player->x = x;
	player->health = 1;
	player->bombs = 1;
	player->bomb_range = 2;
	player->immortal = FALSE;
	player->powers = 0;
	player->last_move = 0;
	player->action = 0;
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
void init_players()
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
void pause(void)
{
	int pause_start = iter_time;
	PlaySound(TEXT("sounds/pause.wav"), NULL, SND_ASYNC | SND_FILENAME);
	nodelay(game_win, FALSE);
	while (getch() != 10);
	nodelay(game_win, TRUE);
}
void power_time(Player *player)
{
	switch (screen[player->y][player->x])
	{
	case BOMBS_UP:
		player->bombs++;
		break;
	case RANGE_UP:
		player->bomb_range++;
		break;
	case HEALTH_UP:
		player->health++;
		break;
	case WALL_HACK:
		player->powers |= 0x10;
		break;
	case IMMORTAL:
		player->immortal = TRUE;
		player->immortal_end = iter_time + IMMORTAL_TIME * CLOCKS_PER_SEC;
		break;
	}
	screen[player->y][player->x] = EMPTY;
}
void gen_power(int y, int x)
{
	int r = rand() % 100;

	if (r < 45) screen[y][x] = BOMBS_UP;
	if (r >= 45 && r < 90) screen[y][x] = RANGE_UP;
	if (r >= 90 && r < 94) screen[y][x] = HEALTH_UP;
	if (r >= 94 && r < 98) screen[y][x] = WALL_HACK;
	if (r >= 98) screen[y][x] = IMMORTAL;
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
	return player->last_move + 200 <= iter_time;
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

	if (screen[y][x] == 4 && 1)
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
		if (screen[y][x] >= POWER_START) power_time(player);
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
	case BOMBS_UP: /* Power-ups getting destroyed :'( */
	case RANGE_UP:
	case HEALTH_UP:
	case WALL_HACK:
	case IMMORTAL:
		fire_queue(y, x);
		screen[y][x] = POWER_ON_FIRE;
		break;
	}
	return 0;
}
void boom(int y, int x, int range)
{
	int i;
	
	PlaySound(TEXT("sounds/boom.wav"), NULL, SND_ASYNC | SND_FILENAME);

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
int game(int mod)
{
    /* ~Initialization~ */
	//FILE *log = fopen("log.txt", "w");
    int i, j, ch, chl = 0, last_iter = clock(), time_start, time_end, wody = m - 1, wodx = 1, woddir = 4, wodinc = 0, wod_last = clock(), py, px, empty_spots;
	bool running = TRUE, wodstop = FALSE, flagger = FALSE;
        
	struct BombList *bpom, *bpompom;
	struct PlayerList *ppom, *ppompom;
	struct FireList *fpom;

	blist_front = NULL, blist_rear = NULL, plist_front = NULL, plist_rear = NULL, flist_front = NULL, flist_rear = NULL;
	
	srand(time(0));
	if (mod==2){
		num_players++;
		num_bots--;
	}
	if (mod==0){
		num_players--;
		num_bots;
	}

    screen = (char**) malloc(m * sizeof(char*));
    for (i = 0; i < m; i++)
    {
		screen[i] = (char*) malloc(n * sizeof(char));
    }
 
    init_screen(m, n);
 
	create_map(m, n, per);

	init_players();
 
	draw(screen,blist_front, plist_front);
 
	time_start = clock();
	time_end = time_start + GAME_TIME * CLOCKS_PER_SEC;

    /* ~The Game Loop!~ */
	while (running)
    {
		iter_time = clock();
		//last_iter = iter_time - last_iter;
		//fprintf(log, "%d\n", last_iter);
		//last_iter = iter_time;
		
		ch = wgetch(game_win);
		//if (ch) chl = ch;
		//flushinp();

		/* Function keys */
		switch (ch)
		{
		case 27:
			running = FALSE;
			break;
		case 10:
			pause();
		}

		/* Player update */
		ppom = plist_front;
		while (ppom != NULL)
		{
			ppom->player->action = 0;

			if (ppom->player->id <= num_players)//can_move(ppom->player)
			{
				if(ppom->player->id==1)
				switch (ch)
				{
				case KEY_RIGHT:
					ppom->player->action = 1;
					break;
				case KEY_DOWN:
					ppom->player->action = 2;
					break;
				case KEY_LEFT:
					ppom->player->action = 3;
					break;
				case KEY_UP:
					ppom->player->action = 4;
					break;
				case ' ':
					ppom->player->action = 5;
					break;
				}
				if(ppom->player->id==2 || mod==1)
				switch (ch)
				{
				case 'd':
					ppom->player->action = 1;
					break;
				case 's':
					ppom->player->action = 2;
					break;
				case 'a':
					ppom->player->action = 3;
					break;
				case 'w':
					ppom->player->action = 4;
					break;
				case 'g':
					ppom->player->action = 5;
					break;
				//chl = 0;
				}
				if (ppom->player->action && ppom->player->action != 5) ppom->player->last_move = iter_time;
			}
			
			if (ppom->player->id > num_players && can_move(ppom->player))
			{
				bot_action(ppom->player);
				if (ppom->player->action && ppom->player->action != 5) ppom->player->last_move = iter_time;
			}

			ppom = ppom->next;
		}

		ppom = plist_front;
		while (ppom != NULL)
		{
			if (ppom->player->action) 
				do_action(ppom->player);
			
			if (ppom->player->immortal && ppom->player->immortal_end <= iter_time) ppom->player->immortal = FALSE;
			if (screen[ppom->player->y][ppom->player->x] == FIRE && ppom->player->immortal == FALSE)
			{
				ppom->player->health--;
				ppom->player->immortal = TRUE;
				ppom->player->immortal_end = iter_time + IMMORTAL_TIME * CLOCKS_PER_SEC;
				if (ppom->player->health == 0)
				{
					bpom = blist_front;
					while (bpom != NULL) 
					{
						if (bpom->bomb->owner && bpom->bomb->owner->id == ppom->player->id) bpom->bomb->owner = NULL;
						bpom = bpom->next;
					}

					if (ppom->prev == NULL) plist_front = ppom->next;
					else ppom->prev->next = ppom->next;
					if (ppom->next == NULL) plist_rear = ppom->prev;
					else ppom->next->prev = ppom->prev;
					ppompom = ppom;
					ppom = ppom->next;
					free(ppompom->player);
					free(ppompom);

					/* Dropping powerups */
					empty_spots = 0;
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
					ppompom = plist_front;
					while (ppompom != NULL)
					{
						if(screen[ppompom->player->y][ppompom->player->x] > POWER_START) power_time(ppompom->player);
						ppompom = ppompom->next;
					}
					
					/* Last man standing? */
					if (plist_front && plist_front->next == NULL)
					{
						plist_front->player->immortal = TRUE;
						plist_front->player->immortal_end = iter_time + 3600 * CLOCKS_PER_SEC;
						wodstop = TRUE;
					}
					continue;
				}
			}
			ppom = ppom->next;
		}
		//if (plist_front == NULL) running = FALSE;

		/* Bomb update */
		bpom = blist_front;
		while (bpom != NULL)	
		{
			if (bpom->bomb->end_time <= iter_time)
			{
				screen[bpom->bomb->y][bpom->bomb->x] = EMPTY;
				boom(bpom->bomb->y, bpom->bomb->x, bpom->bomb->range);
				bpompom = bpom->next; // D:<
				recycle_bomb(bpom);
				bpom = bpompom;
			}
			else 
			{   // UNDER CONSTRUCTION
				if ((bpom->bomb->ydir || bpom->bomb->xdir) && bpom->bomb->last_move + 0.01 * CLOCKS_PER_SEC <= iter_time)
				{
					switch (screen[bpom->bomb->y + bpom->bomb->ydir][bpom->bomb->x + bpom->bomb->xdir])
					{
					case 0:
						ppom = plist_front;
						while (ppom != NULL)
						{
							if(ppom->player->y == bpom->bomb->y + bpom->bomb->ydir && ppom->player->x == bpom->bomb->x + bpom->bomb->xdir) break;
							ppom = ppom->next;
						}
						if (ppom == NULL)
						{
							screen[bpom->bomb->y][bpom->bomb->x] = 0;
							bpom->bomb->y += bpom->bomb->ydir;
							bpom->bomb->x += bpom->bomb->xdir;
							screen[bpom->bomb->y][bpom->bomb->x] = 4;
							bpom->bomb->last_move = iter_time;
						}
						else
						{
							bpom->bomb->ydir = 0;
							bpom->bomb->xdir = 0;
						}
						bpom = bpom->next;
						break;
					case 5:
						screen[bpom->bomb->y][bpom->bomb->x] = EMPTY;
						boom(bpom->bomb->y + bpom->bomb->ydir, bpom->bomb->x + bpom->bomb->xdir, bpom->bomb->range);
						bpompom = bpom->next; // D:<
						recycle_bomb(bpom);
						bpom = bpompom;
						break;
					default:
						bpom->bomb->ydir = 0;
						bpom->bomb->xdir = 0;
						bpom = bpom->next;
						break;
					}
				}
				else bpom = bpom->next;
			}
		}
		
		/* Fire update */
		fpom = flist_front;
		while (fpom != NULL && fpom->end_time <= iter_time)
		{
			/* Fire disposal */
			switch (screen[fpom->y][fpom->x])
			{
			case WALL_ON_FIRE:
				if (rand() % 100 < POWER_CHANCE) 
				{
					gen_power(fpom->y, fpom->x);
					
					ppom = plist_front;
					while (ppom != NULL)
					{
						if(screen[ppom->player->y][ppom->player->x] > POWER_START) power_time(ppom->player);
						ppom = ppom->next;
					}
				}
				else screen[fpom->y][fpom->x] = EMPTY;
				break;
			default:
				screen[fpom->y][fpom->x] = EMPTY;
			}
			flist_front = fpom->next;
			if (flist_front == NULL) flist_rear = NULL;
			else flist_front->prev = NULL;
			free(fpom);
			fpom = flist_front;
		}

		/* Wall of Death */
		while (!wodstop && time_start + WOD_TIME * CLOCKS_PER_SEC <= iter_time && wod_last + 0.2 * CLOCKS_PER_SEC <= iter_time)
		{
			switch (woddir)
			{
			case 1:
				if (wodx < n - 2 - wodinc) wodx++;
				else
				{
					woddir = 2;
					wody++;
				}
				break;
			case 2:
				if (wody < m - 2 - wodinc) wody++;
				else
				{
					woddir = 3;
					wodx--;
					wodinc++;
				}
				break;
			case 3:
				if (wodx > 1 + wodinc) wodx--;
				else
				{
					if (wodinc == 2) 
						wodstop = TRUE;
					else
					{
						woddir = 4;
						wody--;
					}
				}
				break;
			case 4:
				if (wody > 1 + wodinc) wody--;
				else
				{
					woddir = 1;
					wodx++;
				}
				break;
			}

			if (screen[wody][wodx] == 2) continue;
			
			ppom = plist_front;
			while (ppom != NULL)
			{
				if (ppom->player->y == wody && ppom->player->x == wodx)
				{
					bpom = blist_front;
					while (bpom != NULL) 
					{
						if (bpom->bomb->owner && bpom->bomb->owner->id == ppom->player->id) bpom->bomb->owner = NULL;
						bpom = bpom->next;
					}

					if (ppom->prev == NULL) plist_front = ppom->next;
					else ppom->prev->next = ppom->next;
					if (ppom->next == NULL) plist_rear = ppom->prev;
					else ppom->next->prev = ppom->prev;
					ppompom = ppom;
					ppom = ppom->next;
					free(ppompom->player);
					free(ppompom);
					
					/* Dropping powerups */
					empty_spots = 0;
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
					ppompom = plist_front;
					while (ppompom != NULL)
					{
						if(screen[ppompom->player->y][ppompom->player->x] > POWER_START) power_time(ppompom->player);
						ppompom = ppompom->next;
					}

					/* Last man standing? */
					if (plist_front && plist_front->next == NULL) // needs fixin!
					{
						plist_front->player->immortal = TRUE;
						plist_front->player->immortal_end = iter_time + 3600 * CLOCKS_PER_SEC;
						wodstop = TRUE;
					}
					continue;
				}
				ppom = ppom->next;
			}
			//if (plist_front == NULL) running = FALSE;

			switch (screen[wody][wodx])
			{
			case BOMB:
				bpom = get_bomb(wody, wodx);
				recycle_bomb(bpom);
				break;
			case FIRE:
			case WALL_ON_FIRE:
			case POWER_ON_FIRE:
				fpom = get_fire(wody, wodx);
				if (fpom->prev == NULL) flist_front = fpom->next;
				else fpom->prev->next = fpom->next;
				if (fpom->next == NULL) flist_rear = fpom->prev;
				else fpom->next->prev = fpom->prev;
				free(fpom);
			}
			screen[wody][wodx] = STONE_WALL;

			wod_last = iter_time;

			break;
		}
		
		if (time_end <= iter_time) running = FALSE; // time up!

		draw(screen,blist_front, plist_front);

		if (clock() - iter_time <= 33) Sleep(33 - (clock() - iter_time)); // 30 FPS
    }
 
 
 
    /* Game over */
	while (blist_front != NULL)
	{
		bpom = blist_front;
		blist_front = blist_front->next;
		free(bpom->bomb);
		free(bpom);
	}
	while (plist_front != NULL)
	{
		ppom = plist_front;
		plist_front = plist_front->next;
		free(ppom->player);
		free(ppom);
	}
	while (flist_front != NULL)
	{
		fpom = flist_front;
		flist_front = flist_front->next;
		free(fpom);
	}

    for (i = 0; i < m; i++)
    {
        free(screen[i]);
    }
    free(screen);
	num_players = 1;
	num_bots= 7;
	del_stuff();
	//fclose(log);
	
	clear();
	refresh();
    return 0;
}