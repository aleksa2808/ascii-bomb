#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

#define EMPTY 0
#define PLAYER 1
#define STONE_WALL 2
#define WALL 3
#define BOMB 4
#define FIRE 5
#define FIREWALL 6
#define FIREPOWER 7

#define POWER 11//, 12, 13, 14...

typedef struct {
	int x, y, health, speed, bombs, bomb_range, immortal_start;
	bool on_bomb, on_fire, immortal;
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
}BombList;

typedef struct PlayerList {
	Player *player;
	struct PlayerList *prev, *next;
};

typedef struct FireList {
	int x, y, start_time;
	struct FireList *next;
};

char **screen;
struct BombList *blist_front = NULL, *blist_rear = NULL;
struct PlayerList *plist = NULL;
struct FireList *flist_front = NULL, *flist_rear = NULL;

Player init_player(Player *player, int y, int x)
{
	struct PlayerList *p = (struct PlayerList*) malloc(sizeof(struct PlayerList));
	p->player = player;
	p->next = plist;
	plist = p;
	
	player->y = y;
	player->x = x;
	player->health = 3;
	player->bombs = 1;
	player->speed = 1;
	player->bomb_range = 2;
	player->on_bomb = FALSE;
	player->on_fire = FALSE;
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

void power_time(Player *player, int x)
{
	switch (x - POWER)
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
		//player->speed++;
		break;
	case 4:
		//player->immortal = TRUE;
		break;
	}
}

void move_player(Player *player, int y, int x)
{
	if (player->on_bomb)
	{
		player->on_bomb = FALSE;
		screen[player->y][player->x] = 4;
	}
	else if (player->on_fire)
	{
		player->on_fire = FALSE;
		screen[player->y][player->x] = 5;
	}
	else screen[player->y][player->x] = 0;
	player->y = y;
	player->x = x;
	screen[player->y][player->x] = 1;
}

void move_logic(Player *player, int ydir, int xdir)
{
	int y = player->y + ydir, x = player->x + xdir;
	switch (screen[y][x])
	{
	case 0:
	case 1:
	case 7:
		move_player(player, y, x);
		break;
	case 5:
		move_player(player, y, x);
		player->on_fire = TRUE;
		break;
	case 11:
	case 12:
	case 13:
		power_time(player, screen[player->y + ydir][player->x + xdir]);
		move_player(player, y, x);
		break;
	}
}

int do_action(Player *player, int action)
{
	struct BombList *p;

	switch (action)
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
    case 5:                                                   /* Hmm... */
		if (player->bombs && player->on_bomb == FALSE && player->on_fire == FALSE)
		{
			player->bombs--;
			player->on_bomb = TRUE;
				
			/* Enqueue bomb */
			p = (struct BombList*) malloc(sizeof(struct BombList));
			p->bomb = (Bomb*) malloc(sizeof(Bomb));
			p->bomb->owner = player;
			p->bomb->range = player->bomb_range;
			p->bomb->start_time = clock();
			p->bomb->x = player->x;
			p->bomb->y = player->y;
			p->next = NULL;
			if (blist_rear == NULL) 
			{
				blist_front = p;
				p->prev = NULL;
			}
			else 
			{
				blist_rear->next = p;
				p->prev = blist_rear;
			}
			blist_rear = p;
        }
        return 1; //continue
    }
	return 0;
}

struct BombList* get_bomb(int y, int x)
{
	struct BombList *p = blist_front;
	while (p != NULL)
	{
		if (p->bomb->y == y && p->bomb->x == x) 
		{
			return p;
			break;
		}
		p = p->next;
	}
	return p;
}

struct PlayerList* get_player(int y, int x)
{
	struct PlayerList *p = plist;
	while (p != NULL)
	{
		if (p->player->y == y && p->player->x == x) 
		{
			return p;
			break;
		}
		p = p->next;
	}
	return p;
}

struct FireList* get_fire(int y, int x)
{
	struct FireList *p = flist_front;
	while (p != NULL)
	{
		if (p->y == y && p->x == x) 
		{
			return p;
			break;
		}
		p = p->next;
	}
	return p;
}

void fire_queue(int y, int x)
{
	struct FireList *f = (struct FireList*) malloc(sizeof(struct FireList));
	f->x = x;
	f->y = y;
	f->start_time = clock();
	f->next = NULL;
	if (flist_rear == NULL) flist_front = f;
	else flist_rear->next = f;
	flist_rear = f;
}

void boom(struct BombList*);

int xplosion_logic(char **screen, int y, int x)
{
	struct PlayerList *p;

	switch (screen[y][x])
	{
	case 0:
		screen[y][x] = 5;
		fire_queue(y, x);
		return 1;
	case 1:
		// TODO: I KILL YOU!
		p = get_player(y, x);
		if (p->player->immortal == FALSE && --(p->player->health) == 0) // Needs some work...
		{
			//exit(1);
			//if (p->prev == NULL) plist = p->next;
			//else p->prev->next = p->next;
			//if (p->next != NULL) p->next->prev = p->prev;
			//else (game over)
			//free(p);
		}
		if (p->player->immortal == FALSE)
		{
			p->player->immortal = TRUE;
			p->player->immortal_start = clock();
		}
		if (p->player->on_bomb) boom(get_bomb(y, x));
		else p->player->on_fire = TRUE;
		fire_queue(y, x);
		return 1;
	case 3: /* Destructible walls and power-up generation */
		fire_queue(y, x);
		screen[y][x] = 6;
		break;
	case 4: /* Bomb chain reaction */
		boom(get_bomb(y, x));
		break;
	case 11: /* Power-ups getting destroyed :'( */
	case 12:
	case 13:
		fire_queue(y, x);
		screen[y][x] = 7;
		break;
	}
	return 0;
}

void boom(struct BombList *bomb_to_boom)
{
	int i, b_range = bomb_to_boom->bomb->range;
	struct BombList *p;
	
    if (bomb_to_boom->bomb->y == bomb_to_boom->bomb->owner->y && bomb_to_boom->bomb->x == bomb_to_boom->bomb->owner->x) bomb_to_boom->bomb->owner->on_bomb = FALSE; 
	else screen[bomb_to_boom->bomb->y][bomb_to_boom->bomb->x] = 0;
	for (i = 0; i <= b_range; i++)
	{
		if (xplosion_logic(screen, bomb_to_boom->bomb->y, bomb_to_boom->bomb->x + i)) continue;
		break;
	}
	for (i = 1; i <= b_range; i++)
	{
		if (xplosion_logic(screen, bomb_to_boom->bomb->y + i, bomb_to_boom->bomb->x)) continue;
		break;
	}
	for (i = 1; i <= b_range; i++)
	{
		if (xplosion_logic(screen, bomb_to_boom->bomb->y, bomb_to_boom->bomb->x - i)) continue;
		break;
	}
	for (i = 1; i <= b_range; i++)
	{
		if (xplosion_logic(screen, bomb_to_boom->bomb->y - i, bomb_to_boom->bomb->x)) continue;
		break;
	}
            
	/* Bomb disposal */
	p = bomb_to_boom;
	p->bomb->owner->bombs++;
	if (p->prev == NULL) blist_front = p->next;
	else p->prev->next = p->next;
	if (p->next == NULL) blist_rear = p->prev;
	else p->next->prev = p->prev;
	free(p);
}
 
extern void init_screen(int, int);
extern void draw(char**,BombList*);
 
int game(void)
{
    /* ~Initialization~ */
    extern WINDOW *game_win;
    int i, j, ch, mx, my, fill, m = 23, n = 35, per = 80, pl_action;
        
	struct BombList *bfree;
	struct PlayerList *pfree;
	struct FireList *ffree;

	Player player1 = init_player(&player1, 1, 1);
	//Player bot1 = init_player(&bot1, 1, 2);
	
	srand(time(0));

    screen = (char**) malloc(m * sizeof(char*));
    for (i = 0; i < m; i++)
    {
		screen[i] = (char*) malloc(n * sizeof(char));
    }
 
    init_screen(m, n);
 
	create_map(m, n, per);
 
    /* -Spawning players- */
    /* Player 1 */
	screen[player1.y][player1.x] = 1;

	/* Bot 1 */
	//screen[bot1.y][bot1.x] = 1;
 
    draw(screen,blist_front);
 
 
 
    /* ~The Game Loop!~ */
	while (1)
    {
		pl_action = 0;
        ch = wgetch(game_win);
		switch (ch)
        {
        case KEY_RIGHT:
        case 'd':
            pl_action = 1;
            break;
        case KEY_DOWN:
        case 's':
            pl_action = 2;
            break;
        case KEY_LEFT:
        case 'a':
            pl_action = 3;
            break;
        case KEY_UP:
        case 'w':
            pl_action = 4;
            break;
        case ' ':
            pl_action = 5;
            break;
        case 't':
            clear();
            refresh();
            return 0;
        }

		/* Player update */
		//if (pl_action) if (do_action(&player1, pl_action)) continue;
		if (pl_action) do_action(&player1, pl_action);
		//do_action(screen, &bot1, bot_action(&bot1));

		/* Bomb update */
		if (blist_front != NULL && blist_front->bomb->start_time + 1500 <= clock())	boom(blist_front);
		
		/* Fire update */
		while (flist_front != NULL && flist_front->start_time + 100 <= clock())
		{
			/* Fire disposal */
			ffree = flist_front;
			switch (screen[ffree->y][ffree->x])
			{
			case 1:
				get_player(ffree->y, ffree->x)->player->on_fire = FALSE;
				break;
			case 6:
				if (rand() | 1) screen[ffree->y][ffree->x] = 11 + rand() % 3;
				else screen[ffree->y][ffree->x] = 0;
				break;
			default:
				screen[ffree->y][ffree->x] = 0;
			}
			flist_front = ffree->next;
			if (flist_front == NULL) flist_rear = NULL;
			free(ffree);
		}

		/* Player status update */
		pfree = plist;
		while (pfree != NULL)
		{
			if (pfree->player->immortal && pfree->player->immortal_start + 3000 <= clock()) pfree->player->immortal = FALSE;
			pfree = pfree->next;
		}

		//flushinp();
        draw(screen,blist_front);
		//Sleep(10);		
    }
 
 
 
    /* Game over */
	while (blist_front != NULL)
	{
		bfree = blist_front;
		blist_front = blist_front->next;
		free(bfree);
	}
	while (plist != NULL)
	{
		pfree = plist;
		plist = plist->next;
		free(pfree);
	}
	while (flist_front != NULL)
	{
		ffree = flist_front;
		flist_front = flist_front->next;
		free(ffree);
	}

    for (i = 0; i < m; i++)
    {
        free(screen[i]);
    }
    free(screen);
    return 0;
}