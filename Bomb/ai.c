#include <curses.h>
#include <stdlib.h>

typedef struct {
	int id, x, y, health, bombs, bomb_range, action, immortal_start, last_move;
	bool alive, immortal;
	unsigned char powers;
} Player;
typedef struct PlayerList {
	Player *player;
	struct PlayerList *prev, *next;
};

extern char **screen;
extern int m, n;
extern struct PlayerList *plist_front;

extern bool can_pass(Player*, int);

Player *bot;

/* A bunch of self explanatory functions! =D */

int abs(int x)
{
    return (x > 0) ? x : -x;
}
bool safe(int y, int x)
{
	int i = 0, range = 100;
	if (screen[y][x] == 5)
		return FALSE;
	while (i <= range) 
	{
		switch (screen[y][x + i])
		{
		case 2:
		case 3:
		case 6:
			i = range;
			break;
		case 4:
			return FALSE;
		}
		i++;
	}
	i = 1;
	while (i <= range) 
	{
		switch (screen[y + i][x])
		{
		case 2:
		case 3:
		case 6:
			i = range;
			break;
		case 4:
			return FALSE;
		}
		i++;
	}
	i = 1;
	while (i <= range) 
	{
		switch (screen[y][x - i])
		{
		case 2:
		case 3:
		case 6:
			i = range;
			break;
		case 4:
			return FALSE;
		}
		i++;
	}
	i = 1;
	while (i <= range) 
	{
		switch (screen[y - i][x])
		{
		case 2:
		case 3:
		case 6:
			i = range;
			break;
		case 4:
			return FALSE;
		}
		i++;
	}
	return TRUE;
}
int safe_dir()
{
	int i, dir = 0, y = bot->y, x = bot->x, choices[4], num_choices = 0;
	for (i = 1; i < 5; i++){
		if (!can_pass(bot, screen[y][x + i]) || screen[y][x + i] == 5) break;
		else 
		{
			if (safe(y, x + i) || (can_pass(bot, screen[y + 1][x + i]) && safe(y + 1, x + i)) || (can_pass(bot, screen[y - 1][x + i]) && safe(y - 1, x + i))) 
			{
				dir = 1;
				//choices[num_choices] = 1;
				//num_choices++;
				break;
			}
		}
	}
	for (i = 1; i < 5; i++){
		if (!can_pass(bot, screen[y + i][x]) || screen[y + i][x] == 5) break;
		else 
		{
			if (safe(y + i, x) || (can_pass(bot, screen[y + i][x + 1]) && safe(y + i, x + 1)) || (can_pass(bot, screen[y + i][x - 1]) && safe(y + i, x - 1))) 
			{
				dir = 2;
				//choices[num_choices] = 2;
				//num_choices++;
				break;
			}
		}
	}
	for (i = 1; i < 5; i++){
		if (!can_pass(bot, screen[y][x - i]) || screen[y][x - i] == 5) break;
		else 
		{
			if (safe(y, x - i) || (can_pass(bot, screen[y + 1][x - i]) && safe(y + 1, x - i)) || (can_pass(bot, screen[y - 1][x - i]) && safe(y - 1, x - i))) 
			{
				dir = 3;
				//choices[num_choices] = 3;
				//num_choices++;
				break;
			}
		}
	}
	for (i = 1; i < 5; i++){
		if (!can_pass(bot, screen[y - i][x]) || screen[y - i][x] == 5) break;
		else 
		{
			if (safe(y - i, x) || (can_pass(bot, screen[y - i][x + 1]) && safe(y - i, x + 1)) || (can_pass(bot, screen[y - i][x - 1]) && safe(y - i, x - 1))) 
			{
				dir = 4;
				//choices[num_choices] = 4;
				//num_choices++;
				break;
			}
		}
	}
	return dir; //num_choices ? choices[rand() % num_choices] : 0;
}
int detect_powers()
{
	int pdir = 0;

	return pdir;
}
bool players_nearby()
{
	int y = bot->y, x = bot->x, range = bot->bomb_range;
	struct PlayerList *p;
	p = plist_front;

	while (p != NULL)
	{
		if (p->player->id != bot->id && abs(p->player->y - y) <= range && abs(p->player->x - x) <= range) return TRUE;
		p = p->next;
	}
	return FALSE;
}
bool should_place_bomb()
{
	bool should = FALSE;
	int i, y = bot->y, x = bot->x, range = bot->bomb_range;
	char backup = screen[bot->y][bot->x];
	screen[y][x] = 4;
	if (safe_dir())
	{
		for (i = 1; i <= range; i++) 
		{
			if (x + i < n - 1)
				switch (screen[y][x + i])
				{
				case 3:
					should = TRUE;
					i = range;
					break;
				case 0:
				case 5:
					break;
				default:
					i = range;
				}
		}
		for (i = 1; i <= range; i++) 
		{
			if (y + i < m - 1)
				switch (screen[y + i][x])
				{
				case 3:
					should = TRUE;
					i = range;
					break;
				case 0:
				case 5:
					break;
				default:
					i = range;
				}
		}
		for (i = 1; i <= range; i++) 
		{
			if (x - i > 1)
				switch (screen[y][x - i])
				{
				case 3:
					should = TRUE;
					i = range;
					break;
				case 0:
				case 5:
					break;
				default:
					i = range;
				}
		}
		for (i = 1; i <= range; i++) 
		{
			if (y - i > 1)
				switch (screen[y - i][x])
				{
				case 3:
					should = TRUE;
					i = range;
					break;
				case 0:
				case 5:
					break;
				default:
					i = range;
				}
		}
		if (players_nearby()) should = TRUE;
	}
	screen[y][x] = backup;
	return should;
}
void bot_action(Player *current_bot)
{
	int pdir, y = current_bot->y, x = current_bot->x;
	bot = current_bot;

	if (!safe(y, x)) bot->action = safe_dir();
	else if (pdir = detect_powers()) bot->action = pdir;
	else if (bot->bombs && screen[y][x] == 0 && should_place_bomb()) bot->action = 5;
	else 
	{
		switch (rand() % 5)
		{
		case 1:
			if (can_pass(bot, screen[y][x + 1]) && safe(y, x + 1)) bot->action = 1;
			break;
		case 2:
			if (can_pass(bot, screen[y + 1][x]) && safe(y + 1, x)) bot->action = 2;
			break;
		case 3:
			if (can_pass(bot, screen[y][x - 1]) && safe(y, x - 1)) bot->action = 3;
			break;
		case 4:
			if (can_pass(bot, screen[y - 1][x]) && safe(y - 1, x)) bot->action = 4;
		}
	}
}