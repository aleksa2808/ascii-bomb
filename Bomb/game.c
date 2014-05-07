#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

#define EMPTY 0
//#define PLAYER 1
#define STONE_WALL 2
#define WALL 3
#define BOMB 4
#define FIRE 5
#define FIREWALL 6
#define FIREPOWER 7

#define POWER 11//, 12, 13, 14...

typedef struct {
	int x, y, health, speed, bombs, bomb_range, immortal_start, action;
	bool immortal;
	char name[20];
	unsigned char powers;
} Player;
typedef struct { 
	Player *owner;
	int x, y, range, start_time;
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
	int x, y, start_time;
	struct FireList *prev, *next;
};

char **screen;
struct BombList *blist_front, *blist_rear;
struct PlayerList *plist;
struct FireList *flist_front, *flist_rear;

extern void init_screen(int, int);
extern void draw(char**, struct BombList*, struct PlayerList*);
void boom(int y, int x, int range);

Player init_player(Player *player, int y, int x)
{
	struct PlayerList *p = (struct PlayerList*) malloc(sizeof(struct PlayerList));
	p->player = player;
	p->next = plist;
	plist = p;
	
	player->y = y;
	player->x = x;
	player->health = 4;
	player->bombs = 1;
	player->speed = 1;
	player->bomb_range = 2;
	player->immortal = FALSE;
	player->powers = 0;

	return *player;
}
void create_map(int m, int n, int per)
{
	int i, j, fill, mx, my;

	/* ~Map creation~ */
    /* Grass */
    for (i = 0; i < m; i++)
            for (j = 0; j < n; j++)
                    screen[i][j] = 0;
 
    /* Borders */
    for (i = 0; i < m; i++)
    {
            screen[i][0] = 2;
            screen[i][n - 1] = 2;
    }
    for (i = 0; i < n; i++)
    {
            screen[0][i] = 2;
            screen[m - 1][i] = 2;
    }
       
    /* Blocks */
    for (i = 2; i < m - 1; i += 2)
            for (j = 2; j < n - 1; j += 2)
                    screen[i][j]=2;
 
	/* -Reserving corners- */
    /* Top left */
    screen[1][2] = 1;
    screen[1][1] = 1;
    screen[2][1] = 1;
    /* Top right */
    screen[1][n - 3] = 1;
    screen[1][n - 2] = 1;
    screen[2][n - 2] = 1;
    /* Bottom left */
    screen[m - 3][1] = 1;
    screen[m - 2][1] = 1;
    screen[m - 2][2] = 1;
    /* Bottom right */
    screen[m - 2][n - 3] = 1;
    screen[m - 2][n - 2] = 1;
    screen[m - 3][n - 2] = 1;

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
            screen[my][mx] = 3;
    }
 
    /* -Clearing corners- */
    /* Top left */
    screen[1][2] = 0;
    screen[1][1] = 0;
    screen[2][1] = 0;
    /* Top right */
    screen[1][n - 3] = 0;
    screen[1][n - 2] = 0;
    screen[2][n - 2] = 0;
    /* Bottom left */
    screen[m - 3][1] = 0;
    screen[m - 2][1] = 0;
    screen[m - 2][2] = 0;
    /* Bottom right */
    screen[m - 2][n - 3] = 0;
    screen[m - 2][n - 2] = 0;
    screen[m - 3][n - 2] = 0;
}
void power_time(Player *player)
{
	switch (screen[player->y][player->x] - POWER)
	{
	case 0:
		player->bombs++;
		break;
	case 1:
		player->bomb_range += 2;
		break;
	case 2:
		player->health++;
		break;
	case 3:
		player->powers |= 0x10;
		break;
	case 4:
		//player->immortal = TRUE;
		break;
	}
	screen[player->y][player->x] = 0;
}
void gen_power(int y, int x)
{
	struct PlayerList *p;
	int r = rand() % 100;

	if (r < 45) screen[y][x] = 11;
	if (r >= 45 && r < 90) screen[y][x] = 12;
	if (r >= 90 && r < 96) screen[y][x] = 13;
	if (r >= 96) screen[y][x] = 14;

	p = plist;
	while (p != NULL)
	{
		if(screen[p->player->y][p->player->x] > 10) power_time(p->player);
		p = p->next;
	}
}
bool can_pass(Player *player, int x)
{
	switch (x)
	{
	case 2:
	case 4:
		return FALSE;
	case 3:
	case 6:
		if (player->powers & 0x10) return TRUE;
		else return FALSE;
	default:
		return TRUE;
	}
}
void move_logic(Player *player, int ydir, int xdir)
{
	int y = player->y + ydir, x = player->x + xdir;
	
	if (can_pass(player, screen[y][x]))
	{
		player->y = y;
		player->x = x;
		if (screen[y][x] > 10) power_time(player);
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
				screen[player->y][player->x] = 4;

				/* Enqueue bomb */
				b = (struct BombList*) malloc(sizeof(struct BombList));
				b->bomb = (Bomb*) malloc(sizeof(Bomb));
				b->bomb->owner = player;
				b->bomb->range = player->bomb_range;
				b->bomb->start_time = clock();
				b->bomb->x = player->x;
				b->bomb->y = player->y;
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
        return 1; //continue
    }
	return 0;
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
void fire_queue(int y, int x)
{
	struct FireList *f = (struct FireList*) malloc(sizeof(struct FireList));
	if (f == NULL) exit(1);
	f->x = x;
	f->y = y;
	f->start_time = clock();
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
	screen[b->bomb->y][b->bomb->x] = 0;
	boom(b->bomb->y, b->bomb->x, b->bomb->range);
	b->bomb->owner->bombs++;
	if (b->prev == NULL) blist_front = b->next;
	else b->prev->next = b->next;
	if (b->next == NULL) blist_rear = b->prev;
	else b->next->prev = b->prev;
	free(b);
}
int xplosion_logic(int y, int x)
{
	struct FireList *f;

	switch (screen[y][x])
	{
	case 0: /* Empty space */
		screen[y][x] = 5;
		fire_queue(y, x);
		return 1;
	case 3: /* Destructible walls and power-up generation */
		fire_queue(y, x);
		screen[y][x] = 6;
		break;
	case 4: /* Bomb chain reaction */
		recycle_bomb(get_bomb(y, x));
		break;
	case 5: /* Fire */
		f = get_fire(y, x);
		f->start_time = clock();

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
	case 11: /* Power-ups getting destroyed :'( */
	case 12:
	case 13:
	case 14:
		fire_queue(y, x);
		screen[y][x] = 7;
		break;
	}
	return 0;
}
void boom(int y, int x, int range)
{
	int i;

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
int game(void)
{
    /* ~Initialization~ */
	FILE *log = fopen("log.txt", "w");
    extern WINDOW *game_win;
    int i, ch, m = 23, n = 35, per = 80;
	bool running = TRUE;
        
	struct BombList *bpom;
	struct PlayerList *ppom;
	struct FireList *fpom;

	Player player1;

	blist_front = NULL, blist_rear = NULL, plist = NULL, flist_front = NULL, flist_rear = NULL;
	
	player1 = init_player(&player1, 1, 1);
	
	srand(time(0));

    screen = (char**) malloc(m * sizeof(char*));
    for (i = 0; i < m; i++)
    {
		screen[i] = (char*) malloc(n * sizeof(char));
    }
 
    init_screen(m, n);
 
	create_map(m, n, per);
 
	draw(screen,blist_front, plist);
 
 
 
    /* ~The Game Loop!~ */
	while (running)
    {
		player1.action = 0;
        ch = wgetch(game_win);
		switch (ch)
        {
        case KEY_RIGHT:
        case 'd':
            player1.action = 1;
            break;
        case KEY_DOWN:
        case 's':
            player1.action = 2;
            break;
        case KEY_LEFT:
        case 'a':
            player1.action = 3;
            break;
        case KEY_UP:
        case 'w':
            player1.action = 4;
            break;
        case ' ':
            player1.action = 5;
            break;
        case 't':
			running = FALSE;
        }

		/* Player update */
		ppom = plist;
		while (ppom != NULL)
		{
			if (ppom->player->action) do_action(ppom->player);

			if (screen[ppom->player->y][ppom->player->x] == 5 && ppom->player->immortal == FALSE)
			{
				ppom->player->health--;
				ppom->player->immortal = TRUE;
				ppom->player->immortal_start = clock();
				if (ppom->player->health == 0) running = FALSE; //remove him from plist
			}
			if (ppom->player->immortal && ppom->player->immortal_start + 3000 <= clock()) ppom->player->immortal = FALSE;
			ppom = ppom->next;
		}

		/* Bomb clear */
		bpom = blist_front;
		if (bpom != NULL && bpom->bomb->start_time + 1700 <= clock())	
		{
			recycle_bomb(bpom);
			bpom = blist_front;
		}
		
		/* Fire clear */
		fpom = flist_front;
		while (fpom != NULL && fpom->start_time + 900 <= clock())
		{
			/* Fire disposal */
			switch (screen[fpom->y][fpom->x])
			{
			case 6:
				if (rand() % 100 < 20) gen_power(fpom->y, fpom->x);
				else screen[fpom->y][fpom->x] = 0;
				break;
			default:
				screen[fpom->y][fpom->x] = 0;
			}
			flist_front = fpom->next;
			if (flist_front == NULL) flist_rear = NULL;
			else flist_front->prev = NULL;
			free(fpom);
			fpom = flist_front;
		}

		//flushinp();
		draw(screen,blist_front, plist);
		//Sleep(30);		
    }
 
 
 
    /* Game over */
	while (blist_front != NULL)
	{
		bpom = blist_front;
		blist_front = blist_front->next;
		free(bpom);
	}
	while (plist != NULL)
	{
		ppom = plist;
		plist = plist->next;
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

	fclose(log);
	
	clear();
	refresh();
    return 0;
}