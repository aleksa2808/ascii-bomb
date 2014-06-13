#include <curses.h>
#include <time.h>
#include <stdlib.h>
int m, n, k, background=2;
WINDOW *game_win, *hud_win;
#define MAP_SPRITE_NUM 16
#define BOMB_TIME 2

typedef struct{
	unsigned char ch[4][6];
	short col1[4][6],col2[4][6];
}	Sprite;

typedef struct {
	int id, type, x, y, health, bombs, bomb_range, action, last_action, immortal_end, last_move, speed;
	bool immortal;
	unsigned char powers;
} Player;
typedef struct { 
	Player *owner;
	int x, y, range, end_time;
} Bomb;
typedef struct BombList {
	Bomb *bomb;
	struct BombList *prev, *next;
};
typedef struct PlayerList {
	Player *player;
	struct PlayerList *prev, *next;
};
extern Sprite sprPlayer[10];
Sprite sprMap[MAP_SPRITE_NUM],sprBomb[3];

extern int time_end, iter_time;

void init_screen(int mm, int nn)
{	FILE *dat;
	char t1,t2;
	int i, j, l;

	clear();



    m = mm;
    n = nn;
	k=0;
    resize_term(12 + m * 4,n * 6);
 
    

	/*
    init_pair(1, COLOR_GREEN, COLOR_GREEN);
    init_pair(2, COLOR_BLUE, COLOR_GREEN);
    init_pair(3, COLOR_WHITE, COLOR_WHITE);
    init_pair(4, COLOR_YELLOW, COLOR_CYAN);
    init_pair(5, COLOR_MAGENTA,COLOR_GREEN);
	init_pair(6, COLOR_YELLOW, COLOR_RED);
	init_pair(7, COLOR_RED, COLOR_YELLOW);
    init_pair(12, 0, 14);
    init_pair(13, 0, 13);
    init_pair(14, 0, 12);
    init_pair(15, 0, 11);
    init_pair(16, 0, 10);
    */


    game_win = newwin(m * 4, n * 6, 12, 0);
    keypad(game_win, TRUE);
    nodelay(game_win, TRUE);
        
    hud_win = newwin(12, COLS, 0, 0);
    box(hud_win, 0, 0);
	wrefresh(hud_win);

	/*Reading the sprites*/


	dat = fopen("data/mapsprites.txt","r");
	for (k=0;k<MAP_SPRITE_NUM;k++){
		for (i=0;i<4;i++)
			for (j=0;j<6;j++)
				sprMap[k].ch[i][j]=0xDF;
		for (i=0;i<4;i++){
			for (j=0;j<6;j++){
				fscanf(dat,"%c",&t1);
				if (t1=='x'){
					fscanf(dat,"%d%c%c",&l,&t1,&t2);
					sprMap[k].ch[i][j]=l;
				}
				else fscanf(dat,"%c",&t2);
				if (t1==t2) sprMap[k].ch[i][j]=' ';
				sprMap[k].col1[i][j]=t1-'a';
				sprMap[k].col2[i][j]=t2-'a';
			}
			fscanf(dat,"%c",&t1);
		}
	}

	fclose(dat);

	dat = fopen("data/bombsprites.txt","r");
	for (k=0;k<3;k++){
		for (i=0;i<4;i++){
			for (j=0;j<6;j++){
				fscanf(dat,"%c",&t1);
				if (t1=='x'){
					fscanf(dat,"%d%c%c",&l,&t1,&t2);
					sprBomb[k].ch[i][j]=l;
				}
				else {fscanf(dat,"%c",&t2); sprBomb[k].ch[i][j]=' ';}
				sprBomb[k].col1[i][j]=t1-'a';
				sprBomb[k].col2[i][j]=t2-'a';
			}
			fscanf(dat,"%c",&t1);
		}
	}
	fclose(dat);
	/**/




    /*mvwprintw(hud_win, 4, 5, "Press ");
	wattron(hud_win, A_BOLD); 
    wprintw(hud_win, "Esc");
	wattroff(hud_win, A_BOLD); 
    wprintw(hud_win, " to get back");
    mvwprintw(hud_win, 5, 4, "  to the main menu.");

	wattron(hud_win, COLOR_PAIR(12));
    mvwprintw(hud_win, 7, 4, "BOMBS+");
	wattroff(hud_win, COLOR_PAIR(12));

	wattron(hud_win, COLOR_PAIR(13));
    mvwprintw(hud_win, 8, 4, "RANGE+");
	wattroff(hud_win, COLOR_PAIR(13));

	wattron(hud_win, COLOR_PAIR(14));
    mvwprintw(hud_win, 9, 4, "LIFE+");
	wattroff(hud_win, COLOR_PAIR(14));
	
	wattron(hud_win, COLOR_PAIR(15));
    mvwprintw(hud_win, 10, 4, "PASS THROUGH WALLS");
	wattroff(hud_win, COLOR_PAIR(15));
	
	wattron(hud_win, COLOR_PAIR(16));
    mvwprintw(hud_win, 11, 4, "IMMORTAL!");
	wattroff(hud_win, COLOR_PAIR(16));*/

    //wrefresh(hud_win);
}

void draw_tile(int posx,int posy, Sprite spr){
	int i,j, col1, col2;


	for (i=0;i<4;i++)
		for (j=0;j<6;j++){
			
			if (spr.col1[i][j] == 16) 
				col1 = background;
			else col1 = spr.col1[i][j];
			if (spr.col2[i][j] == 16) 
				col2 = background;
			else col2 = spr.col2[i][j];


			wattron(game_win,COLOR_PAIR(col1*16+col2));
			mvwprintw(game_win, posx*4+i, posy*6+j,"%c",spr.ch[i][j]);
			wattroff(game_win,COLOR_PAIR(col1*16+col2));
		}
		
}
void draw_bomb(int posx, int posy, Sprite spr, int k){
	int i,j,c1,c2;
	char ch,c;
	switch (k%3) {
	case 0: ch='x'; break;
	case 1: ch='+'; break;
	case 2: ch='*'; break;
	}
	for (i=0;i<4;i++)
		for (j=0;j<6;j++){
			if (spr.ch[i][j] == ' ') continue;
			else{
				if (spr.col1[i][j] == 16) c1=background; else c1=spr.col1[i][j];	
				if (spr.col2[i][j] == 16) c2=background; else c2=spr.col2[i][j];	

				if (spr.ch[i][j] == 1) c= ch; else	c=spr.ch[i][j];
				wattron(game_win,COLOR_PAIR(c2*16+c1));
				mvwprintw(game_win,posx*4+i,posy*6+j,"%c",c);
				wattroff(game_win,COLOR_PAIR(c2*16+c1));
			}	
		}

}
void draw_player(int posx, int posy, Sprite spr, Sprite bckgr, int ID, int immo, int trnp){
	int i,j,col1,col2;
	static int turn[10] = {0};
	static int playercolor[10]= {15, 14, 13, 12, 11, 10, 9, 2};

	if (trnp == 1) turn[ID]=0;
	if (trnp == 3) turn[ID]=1;

	if (immo == 0){
		for (i=0;i<4;i++)
			for (j=0;j<6;j++) 
				if (!(spr.col1[ i ][ turn[ID]==0?j:5-j ]==16 && spr.col2[ i ][ turn[ID]==0?j:5-j ]==16)) {
					if (spr.col1[ i ][ turn[ID]==0?j:5-j ]==16)
						col1 = bckgr.col1[i][j]==16?background:bckgr.col1[i][j];
					else if (spr.col1[ i ][ turn[ID]==0?j:5-j ]==17)
						col1 = playercolor[ID];
						else col1 = spr.col1[ i ][ turn[ID]==0?j:5-j ];

					if (spr.col2[ i ][ turn[ID]==0?j:5-j ]==16)
						col2 = bckgr.col2[i][j]==16?background:bckgr.col2[i][j];
					else if (spr.col2[ i ][ turn[ID]==0?j:5-j ]==17)
						col2 = playercolor[ID];
						else col2 = spr.col2[ i ][ turn[ID]==0?j:5-j ];
					

					wattron(game_win,COLOR_PAIR(col1*16+col2));
					mvwprintw(game_win, posx*4+i, posy*6+j,"%c",(col1==0&&col2==0)?' ':spr.ch[i][ turn[ID]==0?j:5-j ]);
					wattroff(game_win,COLOR_PAIR(col1*16+col2));
				}
	}
	else {for (i=0;i<4;i++)
			for (j=0;j<6;j++) 
				if (!(spr.col1[ i ][ turn[ID]==0?j:5-j ]==16 && spr.col2[ i ][ turn[ID]==0?j:5-j ]==16)) {
					if (spr.col1[ i ][ turn[ID]==0?j:5-j ]==16)
						col1 = bckgr.col1[i][j]==16?background:bckgr.col1[i][j];
					else col1=15;
					if (spr.col2[ i ][ turn[ID]==0?j:5-j ]==16)
						col2 = bckgr.col2[i][j]==16?background:bckgr.col2[i][j];
					else col2 = 15;
				
					wattron(game_win,COLOR_PAIR(col2*16+col1));
					mvwprintw(game_win, posx*4+i, posy*6+j,"%c",spr.ch[ i ][ turn[ID]==0?j:5-j ]);
					wattroff(game_win,COLOR_PAIR(col2*16+col1));

				}
	}
}

void draw_hudp(Sprite spr, int ID){
	int i,j,col1,col2;
	
	static int playercolor[10]= {15, 14, 13, 12, 11, 10, 9, 2};

	for (i=12*(ID)-7;i < 12*(ID) +1 ;i++){
		wattron(hud_win,COLOR_PAIR(130));
		mvwprintw(hud_win, 3, i, "%c", 0xDC);
		mvwprintw(hud_win,8,i,"%c", 0xDF);
		wattroff(hud_win, COLOR_PAIR(130));
	}
	for (i=4;i<8;i++){
		wattron(hud_win,COLOR_PAIR(130));
		mvwprintw(hud_win, i,12*(ID)-7, "%c",0xDB);
		mvwprintw(hud_win,i,12*(ID),"%c",0xDB);
		wattroff(hud_win, COLOR_PAIR(130));
	}
		for (i=0;i<4;i++)
			for (j=0;j<6;j++) {
					if (spr.col1[ i ][ j ]==16)
						col1 = 3;
					else if (spr.col1[ i ][ j ]==17)
						col1 = playercolor[ID-1];
						else col1 = spr.col1[ i ][ j ];

					if (spr.col2[ i ][ j ]==16)
						col2 = 3;
					else if (spr.col2[ i ][ j ]==17)
						col2 = playercolor[ID-1];
						else col2 = spr.col2[ i ][ j ];
					

					wattron(hud_win,COLOR_PAIR(col1*16+col2));
					mvwprintw(hud_win, i+4, j+12*(ID)-6,"%c",(col1==0&&col2==0)?' ':spr.ch[i][ j ]);
					wattroff(hud_win,COLOR_PAIR(col1*16+col2));
				}
}

void draw(char **screen, struct BombList *b, struct PlayerList *p)
{	
    int i, j, pnum, clk;
	double temptime;
    wclear(game_win);
    for (i=0; i<m; i++){
            for (j=0; j<n; j++){
                    draw_tile(i,j,sprMap[screen[i][j]]);
            }
    }
	clk = clock();
	
	while (b){
		temptime = 3*(b->bomb->end_time - clk)/((double) BOMB_TIME*CLOCKS_PER_SEC);
		if (temptime < 1) {draw_bomb (b->bomb->y,b->bomb->x,sprBomb[2],k);}
		else if (temptime < 2){draw_bomb (b->bomb->y,b->bomb->x,sprBomb[1],k);}
		else {draw_bomb (b->bomb->y,b->bomb->x,sprBomb[0],k);}
		b=b->next;
	}

	// HUD initialization
	wclear(hud_win);

	wattron(hud_win,COLOR_PAIR(50));
    for (i=0; i<COLS*12; i++) wprintw(hud_win, " ");
	mvwprintw(hud_win, 2, COLS / 2 - 2, "%d:%02d", ((time_end - iter_time) / 1000) / 60, ((time_end - iter_time) / 1000) % 60);
	
	wattroff(hud_win, COLOR_PAIR(50));
	i=0;
	while (p){
		if (p->player->immortal == TRUE && k % 20 > 10)
			draw_player(p->player->y,p->player->x,sprPlayer[0],sprMap[screen[p->player->y][p->player->x]<=10?screen[p->player->y][p->player->x]:0],p->player->id-1,1, p->player->action);
		else
			draw_player(p->player->y,p->player->x,sprPlayer[0],sprMap[screen[p->player->y][p->player->x]<=10?screen[p->player->y][p->player->x]:0],p->player->id-1,0, p->player->action);
		
		// HUD player drawing
		draw_hudp(sprPlayer[0],p->player->id);
		p = p->next;
	}
	
	
	k++;
	if (k > 3000) k = 0;
	wrefresh(game_win);
	wrefresh(hud_win);
	
}
void del_stuff()
{
	delwin(game_win);
	delwin(hud_win);
}