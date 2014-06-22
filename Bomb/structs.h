#pragma once
#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include "functions.h"


// 1000 = 1sec // never below 100!
// 600 is ok :D
#define TIME_SLICE 600

#define MAP_SPRITE_NUM 20

#define WIDTH 30
#define HEIGHT 20 

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

#define STORY_TIME 180
#define GAME_TIME 120
#define GEN_TIME 60
#define BOMB_TIME 2
#define FIRE_TIME 0.5
#define IMMORTAL_TIME 4

#define POWER_CHANCE 10

// GenAlg stuff
#define RANDOM_NUM		((float)rand()/(RAND_MAX+1)) // [0..1]

#define CROSSOVER_RATE            0.7
#define MUTATION_RATE             0.01
#define POP_SIZE                  8
#define CHROMO_LENGTH             60
#define GENE_LENGTH               3

//structs from ai.c, screen.c and game.c
typedef struct {
	int id, type, x, y, health, bombs, bomb_range, action, last_action, immortal_end, last_move, speed;
	bool immortal;
	unsigned char powers;
	char gene[CHROMO_LENGTH + 1], code[(CHROMO_LENGTH / GENE_LENGTH) + 1];
} Player;

typedef struct { 
	Player *owner;
	int x, y, range, end_time, xdir, ydir, last_move;
} Bomb;

typedef struct
{
	int wod_start, y, x, dir, inc, last_move;
	bool alive;
} WallOfDeath;

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

typedef struct ScoreList {
	char *name;
	int points;
	struct ScoreList *next;
};

//structs from screen.c and main.c
typedef struct Sprite{
	unsigned char ch[4][6];
	short col1[4][6],col2[4][6];
}	Sprite;

struct m_id {
	int id;
	char *nam;
};

struct m_list {
	int num;
	int ret;
	struct m_id *men;
	
};

typedef struct m_id smth;