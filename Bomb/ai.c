#include <curses.h>
#include <stdlib.h>
#include <math.h>
#include "structs.h"

extern char **screen;
extern int m, n;
extern struct PlayerList *plist_front;
extern int iter_time;
extern WallOfDeath *w;
extern bool can_pass(Player*, int);

Player *bot;

/* A bunch of self explanatory functions! =D */

bool safe(int y, int x)
{
	int i = 0, range = bot->bomb_range + 2;
	
	// avoiding fire
	if (screen[y][x] == 5)
		return FALSE;

	// avoiding the Wall
	if (w)
	{
		if (!w->alive && w->wod_start < iter_time + 5000 && y == 1) return FALSE;
		else if (w->alive)
		{
			switch (w->dir)
			{
			case 1:
				if (y - 1 <= w->inc) return FALSE;
				if ((n - 2) - x <= w->inc) return FALSE;
				break;
			case 2:
				if ((n - 2) - x <= w->inc) return FALSE;
				if ((m - 2) - y <= w->inc) return FALSE;
				break;
			case 3:
				if ((m - 2) - y <= w->inc - 1) return FALSE;
				if (x - 1 <= w->inc - 1) return FALSE;
				break;
			case 4:
				if (x - 1 <= w->inc) return FALSE;
				if (y - 1 <= w->inc) return FALSE;
				break;
			}
		}
	}

	// avoiding bombs
	while (i <= range) 
	{
		switch (screen[y][x + i])
		{
		case 2:
		case 3:
		case 6:
		case 7:
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
		case 7:
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
		case 7:
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
		case 7:
			i = range;
			break;
		case 4:
			return FALSE;
		}
		i++;
	}
	return TRUE;
}
int safe_dir(int y, int x)
{
	int i, choices[4], num_choices = 0, min = 5, ind = 0;
	for (i = 1; i <= min; i++){
		if (!can_pass(bot, screen[y][x + i]) || screen[y][x + i] == 5) break;
		else 
		{
			if (safe(y, x + i) || (can_pass(bot, screen[y + 1][x + i]) && safe(y + 1, x + i)) || (can_pass(bot, screen[y - 1][x + i]) && safe(y - 1, x + i))) 
			{
				min = i;
				num_choices++;
				choices[ind++] = 1;
				break;
			}
		}
	}
	for (i = 1; i <= min; i++){
		if (!can_pass(bot, screen[y + i][x]) || screen[y + i][x] == 5) break;
		else 
		{
			if (safe(y + i, x) || (can_pass(bot, screen[y + i][x + 1]) && safe(y + i, x + 1)) || (can_pass(bot, screen[y + i][x - 1]) && safe(y + i, x - 1))) 
			{
				if (i <= min)
				{
					choices[ind++] = 2;
					if (i == min) num_choices++;
					else
					{
						min = i;
						num_choices = 1;
					}
				}
				break;
			}
		}
	}
	for (i = 1; i <= min; i++){
		if (!can_pass(bot, screen[y][x - i]) || screen[y][x - i] == 5) break;
		else 
		{
			if (safe(y, x - i) || (can_pass(bot, screen[y + 1][x - i]) && safe(y + 1, x - i)) || (can_pass(bot, screen[y - 1][x - i]) && safe(y - 1, x - i))) 
			{
				if (i <= min)
				{
					choices[ind++] = 3;
					if (i == min) num_choices++;
					else
					{
						min = i;
						num_choices = 1;
					}
				}
				break;
			}
		}
	}
	for (i = 1; i <= min; i++){
		if (!can_pass(bot, screen[y - i][x]) || screen[y - i][x] == 5) break;
		else 
		{
			if (safe(y - i, x) || (can_pass(bot, screen[y - i][x + 1]) && safe(y - i, x + 1)) || (can_pass(bot, screen[y - i][x - 1]) && safe(y - i, x - 1))) 
			{
				if (i <= min)
				{
					choices[ind++] = 4;
					if (i == min) num_choices++;
					else
					{
						min = i;
						num_choices = 1;
					}
				}
				break;
			}
		}
	}

	min = 5;
	if (!num_choices && (bot->powers & 0x1))
	{
		for (i = 1; i <= min; i++){
			if ((!can_pass(bot, screen[y][x + i]) && screen[y][x + i] != 4) || screen[y][x + i] == 5) break;
			else 
			{
				if (screen[y][x + i] == 4 && screen[y][x + i + 1] == 0) 
				{
					min = i;
					num_choices++;
					choices[ind++] = 1;
					break;
				}
			}
		}
		for (i = 1; i <= min; i++){
			if ((!can_pass(bot, screen[y + i][x]) && screen[y + i][x] != 4) || screen[y + i][x] == 5) break;
			else 
			{
				if (screen[y + i][x] == 4 && screen[y + i + 1][x] == 0) 
				{
					if (i <= min)
					{
						choices[ind++] = 2;
						if (i == min) num_choices++;
						else
						{
							min = i;
							num_choices = 1;
						}
					}
					break;
				}
			}
		}
		for (i = 1; i <= min; i++){
			if ((!can_pass(bot, screen[y][x - i]) && screen[y][x - i] != 4) || screen[y][x - i] == 5) break;
			else 
			{
				if (screen[y][x - i] == 4 && screen[y][x - i - 1] == 0) 
				{
					if (i <= min)
					{
						choices[ind++] = 3;
						if (i == min) num_choices++;
						else
						{
							min = i;
							num_choices = 1;
						}
					}
					break;
				}
			}
		}
		for (i = 1; i <= min; i++){
			if ((!can_pass(bot, screen[y - i][x]) && screen[y - i][x] != 4) || screen[y - i][x] == 5) break;
			else 
			{
				if (screen[y - i][x] == 4 && screen[y - i - 1][x] == 0) 
				{
					if (i <= min)
					{
						choices[ind++] = 4;
						if (i == min) num_choices++;
						else
						{
							min = i;
							num_choices = 1;
						}
					}
					break;
				}
			}
		}
	}

	return num_choices ? choices[--ind - rand() % num_choices] : 0;
}
int detect_powers(void)
{
	int i, y = bot->y, x = bot->x, choices[4], num_choices = 0, min = 5, ind = 0;
	for (i = 1; i <= min; i++){
		if (!can_pass(bot, screen[y][x + i]) || !safe(y, x + i)) break;
		else 
		{
			if (screen[y][x + i] >= 11 || (can_pass(bot, screen[y + 1][x + i]) && safe(y + 1, x + i) && screen[y + 1][x + i] >= 11) || (can_pass(bot, screen[y - 1][x + i]) && safe(y - 1, x + i) && screen[y - 1][x + i] >= 11)) 
			{
				min = i;
				num_choices++;
				choices[ind++] = 1;
				break;
			}
		}
	}
	for (i = 1; i <= min; i++){
		if (!can_pass(bot, screen[y + i][x]) || !safe(y + i, x)) break;
		else 
		{
			if (screen[y + i][x] >= 11 || (can_pass(bot, screen[y + i][x + 1]) && safe(y + i, x + 1) && screen[y + i][x + 1] >= 11) || (can_pass(bot, screen[y + i][x - 1]) && safe(y + i, x - 1) && screen[y + i][x - 1] >= 11)) 
			{
				if (i <= min)
				{
					choices[ind++] = 2;
					if (i == min) num_choices++;
					else
					{
						min = i;
						num_choices = 1;
					}
				}
				break;
			}
		}
	}
	for (i = 1; i <= min; i++){
		if (!can_pass(bot, screen[y][x - i]) || !safe(y, x - i)) break;
		else 
		{
			if (screen[y][x - i] >= 11 || (can_pass(bot, screen[y + 1][x - i]) && safe(y + 1, x - i) && screen[y + 1][x - i] >= 11) || (can_pass(bot, screen[y - 1][x - i]) && safe(y - 1, x - i) && screen[y - 1][x - i] >= 11))
			{
				if (i <= min)
				{
					choices[ind++] = 3;
					if (i == min) num_choices++;
					else
					{
						min = i;
						num_choices = 1;
					}
				}
				break;
			}
		}
	}
	for (i = 1; i <= min; i++){
		if (!can_pass(bot, screen[y - i][x]) || !safe(y - i, x)) break;
		else 
		{
			if (screen[y - i][x] >= 11 || (can_pass(bot, screen[y - i][x + 1]) && safe(y - i, x + 1) && screen[y - i][x + 1] >= 11) || (can_pass(bot, screen[y - i][x - 1]) && safe(y - i, x - 1) && screen[y - i][x - 1] >= 11)) 
			{
				if (i <= min)
				{
					choices[ind++] = 4;
					if (i == min) num_choices++;
					else
					{
						min = i;
						num_choices = 1;
					}
				}
				break;
			}
		}
	}

	return num_choices ? choices[--ind - rand() % num_choices] : 0;
}
bool can_kill(void)
{
	int y = bot->y, x = bot->x, range = bot->bomb_range;
	struct PlayerList *p = plist_front;
	while (p)
	{
		if (p->player->id != bot->id && (((p->player->y == y && abs(p->player->x - x) <= range && !(screen[y][x + 1] == 2 && screen[y][x - 1] == 2))) || (p->player->x == x && abs(p->player->y - y) <= range && !(screen[y + 1][x] == 2 && screen[y - 1][x] == 2)))) return TRUE;
		p = p->next;
	}
	return FALSE;
}
bool players_in_range(int y, int x, int range)
{
	struct PlayerList *p = plist_front;
	while (p)
	{
		if (p->player->id != bot->id && abs(p->player->y - y) <= range && abs(p->player->x - x) <= range) return TRUE;
		p = p->next;
	}
	return FALSE;
}
int damage_made(int y, int x)
{
	int i, range = bot->bomb_range, blocks = 0;

	for (i = 1; i <= range; i++)
	{
		switch (screen[y][x + i])
		{
		case 3:
			blocks++;
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
		switch (screen[y + i][x])
		{
		case 3:
			blocks++;
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
		switch (screen[y][x - i])
		{
		case 3:
			blocks++;
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
		switch (screen[y - i][x])
		{
		case 3:
			blocks++;
			i = range;
			break;
		case 0:
		case 5:
			break;
		default:
			i = range;
		}
	}

	return blocks;
}
bool can_place_bomb(int y, int x)
{
	switch (screen[y][x])
	{
	case 2:
	case 3:
	case 4:
	case 6:
	case 7:
		return FALSE;
	default:
		return TRUE;
	}
}
bool should_place_bomb(int y, int x)
{
	bool should = FALSE;
	int range = bot->bomb_range;
	char bkp = screen[y][x];

	if (can_place_bomb(y, x))
	{
		screen[y][x] = 4;
		if (safe_dir(y, x)) should = TRUE;
		screen[y][x] = bkp;
	}

	return should;
}
int destroy_blocks(void)
{
	int action = 0, y = bot->y, x = bot->x, max_dmg = 0, dmg;

	if (bot->bombs && should_place_bomb(y, x) && (max_dmg = damage_made(y, x))) action = 5;
	
	if (can_pass(bot, screen[y][x + 1]) && safe(y, x + 1) && should_place_bomb(y, x + 1) && (dmg = damage_made(y, x + 1)) > max_dmg)
	{
		max_dmg = dmg;
		action = 1;
	}
	if (can_pass(bot, screen[y + 1][x]) && safe(y + 1, x) && should_place_bomb(y + 1, x) && (dmg = damage_made(y + 1, x)) > max_dmg)
	{
		max_dmg = dmg;
		action = 2;
	}
	if (can_pass(bot, screen[y][x - 1]) && safe(y, x - 1) && should_place_bomb(y, x - 1) && (dmg = damage_made(y, x - 1)) > max_dmg)
	{
		max_dmg = dmg;
		action = 3;
	}
	if (can_pass(bot, screen[y - 1][x]) && safe(y - 1, x) && should_place_bomb(y - 1, x) && (dmg = damage_made(y - 1, x)) > max_dmg)
	{
		max_dmg = dmg;
		action = 4;
	}

	return action;
}
int flee(void)
{
	int i, y = bot->y, x = bot->x, choices[4], num_choices = 0, min = 5, ind = 0;
	if (players_in_range(y, x, 2))
	{
		for (i = 1; i <= min; i++){
			if (!can_pass(bot, screen[y][x + i]) || !safe(y, x + i)) break;
			else
			{
				if (!players_in_range(y, x + i, 2) || (can_pass(bot, screen[y + 1][x + i]) && safe(y + 1, x + i) && !players_in_range(y + 1, x + i, 2)) || (can_pass(bot, screen[y - 1][x + i]) && safe(y - 1, x + i) && !players_in_range(y - 1, x + i, 2)))
				{
					min = i;
					num_choices++;
					choices[ind++] = 1;
					break;
				}
			}
		}
		for (i = 1; i <= min; i++){
			if (!can_pass(bot, screen[y + i][x]) || !safe(y + i, x)) break;
			else 
			{
				if (!players_in_range(y + i, x, 2) || (can_pass(bot, screen[y + i][x + 1]) && safe(y + i, x + 1) && !players_in_range(y + i, x + 1, 2)) || (can_pass(bot, screen[y + i][x - 1]) && safe(y + i, x - 1) && !players_in_range(y + i, x - 1, 2)))
				{
					if (i <= min)
					{
						choices[ind++] = 2;
						if (i == min) num_choices++;
						else
						{
							min = i;
							num_choices = 1;
						}
					}
					break;
				}
			}
		}
		for (i = 1; i <= min; i++){
			if (!can_pass(bot, screen[y][x - i]) || !safe(y, x - i)) break;
			else 
			{
				if (!players_in_range(y, x - i, 2) || (can_pass(bot, screen[y + 1][x - i]) && safe(y + 1, x - i) && !players_in_range(y + 1, x - i, 2)) || (can_pass(bot, screen[y - 1][x - i]) && safe(y - 1, x - i) && !players_in_range(y - 1, x - i, 2)))
				{
					if (i <= min)
					{
						choices[ind++] = 3;
						if (i == min) num_choices++;
						else
						{
							min = i;
							num_choices = 1;
						}
					}
					break;
				}
			}
		}
		for (i = 1; i <= min; i++){
			if (!can_pass(bot, screen[y - i][x]) || !safe(y - i, x)) break;
			else 
			{
				if (!players_in_range(y - i, x, 2) || (can_pass(bot, screen[y - i][x + 1]) && safe(y - i, x + 1) && !players_in_range(y - i, x + 1, 2)) || (can_pass(bot, screen[y - i][x - 1]) && safe(y - i, x - 1) && !players_in_range(y - i, x - 1, 2)))
				{
					if (i <= min)
					{
						choices[ind++] = 4;
						if (i == min) num_choices++;
						else
						{
							min = i;
							num_choices = 1;
						}
					}
					break;
				}
			}
		}
	}

	return num_choices ? choices[--ind - rand() % num_choices] : 0;
}
float dist_to(int y, int x, Player *player)
{
	return sqrt(pow(player->y - y, 2) + pow(player->x - x, 2));
}
int sgn(int x)
{
	return x < 0 ? -1 : x > 0 ? 1 : 0;
}
int hunt_players(void)
{
	int dir = 0, y = bot->y, x = bot->x, choices[2], ind = 0;
	float minf = m + n, dist;
	struct PlayerList *p = plist_front;
	Player *tar = NULL;
	
	//if (can_kill()) return 0;

	if (p && p->next)
	{

		if (players_in_range(y, x, 3))
			while (p)
			{
				if (p->player->id != bot->id)
				{
					dist = dist_to(y, x, p->player);
					if (dist <= minf || (dist == minf && (rand() & 1)))
					{
						minf = dist;
						tar = p->player;
					}
				}

				p = p->next;
			}
		else
			while (p)
			{
				if (p->player->id == bot->id)
				{
					if (p->next) tar = p->next->player;
					else tar = plist_front->player;
					minf = dist_to(y, x, tar);
					break;
				}

				p = p->next;
			}

		if (minf > 1)
		{
			if ((y == tar->y || x == tar->x) && screen[y + sgn(tar->y - y)][x + sgn(tar->x - x)] == 2)
			{
				if (y == tar->y)
				{
					if (can_pass(bot, screen[y + 1][x]) && safe(y + 1, x)) choices[ind++] = 2;
					if (can_pass(bot, screen[y - 1][x]) && safe(y - 1, x)) choices[ind++] = 4;
				}
				else
				{
					if (can_pass(bot, screen[y][x + 1]) && safe(y, x + 1)) choices[ind++] = 1;
					if (can_pass(bot, screen[y][x - 1]) && safe(y, x - 1)) choices[ind++] = 3;
				}
			}

			if (ind != 2 && can_pass(bot, screen[y][x + 1]) && safe(y, x + 1) && dist_to(y, x + 1, tar) < minf) choices[ind++] = 1;
			if (ind != 2 && can_pass(bot, screen[y + 1][x]) && safe(y + 1, x) && dist_to(y + 1, x, tar) < minf) choices[ind++] = 2;
			if (ind != 2 && can_pass(bot, screen[y][x - 1]) && safe(y, x - 1) && dist_to(y, x - 1, tar) < minf) choices[ind++] = 3;
			if (ind != 2 && can_pass(bot, screen[y - 1][x]) && safe(y - 1, x) && dist_to(y - 1, x, tar) < minf) choices[ind++] = 4;
		}
	}

	return ind ? choices[rand() % ind] : 0;
}
void bot_action(Player *current_bot, int diff)
{
	int y = current_bot->y, x = current_bot->x, i;
	char *commands = current_bot->code, com, bomb_flag = 0, nav_flag = -1;
	bot = current_bot;
	
	// miss?
	switch (diff)
	{
	case 1:
		if (rand() % 100 < 30) bot->action = -1;
		break;
	case 2:
		if (rand() % 100 < 15) bot->action = -1;
		break;
	}

	i = 0;
	while ((com = commands[i++]) != '\0' && !bot->action)
	{
		// miss?
		switch (diff)
		{
		case 1:
			if (rand() % 100 < 30) com = rand() % 8 + '0';
			break;
		case 2:
			if (rand() % 100 < 15) com = rand() % 8 + '0';
			break;
		}

		switch(com)
		{
		case '0':
			if (!safe(y, x)) bot->action = safe_dir(y, x);
			break;
		case '1':
			bot->action = detect_powers();
			break;
		case '2':
			bot->action = destroy_blocks();
			break;
		case '3':
			if (!bomb_flag && bot->bombs && can_kill() && should_place_bomb(y, x)) bot->action = 5;
			bomb_flag = 1;
			break;
		case '4':
			if (!bomb_flag && bot->bombs && players_in_range(y, x, 3) && should_place_bomb(y, x)) bot->action = 5;
			bomb_flag = 1;
			break;
		case '5':
			if (nav_flag == -1) nav_flag = 0;
			break;
		case '6':
			if (nav_flag == -1)
				switch (rand() & 31)
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
			break;
		case '7':
			if (nav_flag == -1) nav_flag = 1;
			break;
		}
	}
	
	if (!bot->action)
	{
		if (!nav_flag) bot->action = hunt_players();
		else bot->action = flee();
	}
}
void mob_action(Player *current_mob)
{
	int y = current_mob->y, x = current_mob->x;
	bool dirc = rand() % 100 < 20 ? TRUE : FALSE;
	bot = current_mob;
	switch (current_mob->last_action)
	{
	case 1:
		if (can_pass(bot, screen[y][x + 1]))
			bot->action = 1;
		else if (dirc && can_pass(bot, screen[y - 1][x]))
			bot->action = 4;
		else if (dirc && can_pass(bot, screen[y + 1][x]))
			bot->action = 2;
		else 
			bot->action = 3;
		break;
	case 2:
		if (can_pass(bot, screen[y + 1][x]))
			bot->action = 2;
		else if (dirc && can_pass(bot, screen[y][x - 1]))
			bot->action = 3;
		else if (dirc && can_pass(bot, screen[y][x + 1]))
			bot->action = 1;
		else 
			bot->action = 4;
		break;
	case 3:
		if (can_pass(bot, screen[y][x - 1]))
			bot->action = 3;
		else if (dirc && can_pass(bot, screen[y - 1][x]))
			bot->action = 4;
		else if (dirc && can_pass(bot, screen[y + 1][x]))
			bot->action = 2;
		else 
			bot->action = 1;
		break;
	case 4:
		if (can_pass(bot, screen[y - 1][x]))
			bot->action = 4;
		else if (dirc && can_pass(bot, screen[y][x - 1]))
			bot->action = 3;
		else if (dirc && can_pass(bot, screen[y][x + 1]))
			bot->action = 1;
		else 
			bot->action = 2;
		break;
	}
}