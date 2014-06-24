#pragma once
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include "structs.h"


void init_screen(int, int, int);
void draw(char**, struct BombList*, struct PlayerList*);
void scoreboard(int *, int, int);
void update_hud(int, char*, int);
void del_stuff(void);
void story_time(int);


void boom(int y, int x, int range);
struct BombList* get_bomb(int, int);
int time_end, iter_time;
int campaign(void);
int battle(int, int, int, int);
int training_area(void);
void updConfigs();
int fun(void);
