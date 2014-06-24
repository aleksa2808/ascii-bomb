#include <curses.h>
#include <time.h>
#include <stdlib.h>
#include <Windows.h>
#include "structs.h"
#include "functions.h"
WINDOW *game_win, *hud_win, *story_win;


int fusecol=12;
int background=2;
int hudcol=0;
int m,n;

extern int mode;
extern int points, lives;

//gen_alg
int gen, hp;
char *fittest;

Sprite sprPlayer[10];
Sprite sprMap[MAP_SPRITE_NUM],sprBomb[3];

extern int time_end, iter_time;

void init_screen(int mm, int nn, int arena_id)
{	
	FILE *dat;
	char t1,t2;
	int i, j, l;
	int k;

    m = mm;
    n = nn;
    resize_term(7 + m * 4,n * 6);

	gen = 0;

    game_win = newwin(m * 4, n * 6, 7, 0);
    keypad(game_win, TRUE);
    nodelay(game_win, TRUE);
        
    hud_win = newwin(7, COLS, 0, 0);
	wrefresh(hud_win);

	/*Reading the sprites*/

	switch (arena_id){
		case 1:
			dat = fopen("data/mapsprites.txt","r");
			break;
		case 2:
			dat = fopen("data/mapsprites2.txt","r");
			break;
		case 3:
			dat = fopen("data/mapsprites3.txt","r");
			break;
	}

	i = fscanf(dat, "%d %d %d\n", &background, &fusecol, &hudcol);
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
				if (c1 == 14 && c1!=fusecol) c1 = fusecol;
				if (c2 == 14 && c2!=fusecol) c2 = fusecol;
				if (spr.ch[i][j] == 1) c= ch; else	c=spr.ch[i][j];
				wattron(game_win,COLOR_PAIR(c2*16+c1));
				mvwprintw(game_win,posx*4+i,posy*6+j,"%c",c);
				wattroff(game_win,COLOR_PAIR(c2*16+c1));
			}	
		}

}
void draw_player(int posx, int posy, Sprite spr, Sprite bckgr, int ID, int immo, int trnp){
	int i,j,col1,col2, t;
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
					else col1 = 15;
					if (spr.col2[ i ][ turn[ID]==0?j:5-j ]==16)
						col2 = bckgr.col2[i][j]==16?background:bckgr.col2[i][j];
					else col2 = 15;
					
					if (spr.ch[ i ][ turn[ID]==0?j:5-j ] == 0xDF) {t=col1; col1=col2; col2=t;}

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
		wattron(hud_win,COLOR_PAIR(128+hudcol));
		mvwprintw(hud_win, 0, i, "%c", 0xDC);
		mvwprintw(hud_win,5,i,"%c", 0xDF);
		wattroff(hud_win, COLOR_PAIR(128+hudcol));
	}
	for (i=1;i<5;i++){
		wattron(hud_win,COLOR_PAIR(128+hudcol));
		mvwprintw(hud_win, i,12*(ID)-7, "%c",0xDB);
		mvwprintw(hud_win,i,12*(ID),"%c",0xDB);
		wattroff(hud_win, COLOR_PAIR(128+hudcol));
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
					mvwprintw(hud_win, i+1, j+12*(ID)-6,"%c",(col1==0&&col2==0)?' ':spr.ch[i][ j ]);
					wattroff(hud_win,COLOR_PAIR(col1*16+col2));
				}
}

void draw(char **screen, struct BombList *b, struct PlayerList *p)
{	
	static int k;
    int i, j, clk;
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

	wattron(hud_win,COLOR_PAIR(hudcol));
    for (i=0; i<COLS*12; i++) wprintw(hud_win, " ");
	wattroff(hud_win, COLOR_PAIR(hudcol));
	wattron (hud_win,COLOR_PAIR(15*16));
	if (mode != 3) mvwprintw(hud_win, 6, COLS / 2 - 3, "%02d:%02d", ((time_end - iter_time) / 1000) / 60, ((time_end - iter_time) / 1000) % 60);
	if (gen > 0) wprintw(hud_win, "  Gen: %d / Fattest: %s, ate %d of pie.", gen, fittest, hp);
	if (mode == 3)
	{
		mvwprintw(hud_win, 3, COLS / 2 - 20, "Hope you had fun with this little game! ^_^");
		mvwprintw(hud_win, 5, COLS / 2 + 10, "Now RUN!");
	}

	wattroff (hud_win, COLOR_PAIR(15*16));
	
	
	i=0;

	if (mode ==1){
		wattron(hud_win,COLOR_PAIR(hudcol));
		mvwprintw(hud_win, 6, 6, "Lives:%d\tPoints:%d",lives, points);
		wattroff(hud_win,COLOR_PAIR(hudcol));
	}
	while (p){
		if (p->player->immortal == TRUE && k % 20 > 10)
			draw_player(p->player->y,	p->player->x,	sprPlayer[p->player->type],	sprMap[screen[p->player->y][p->player->x]],p->player->id-1,1, p->player->action);
		else
			draw_player(p->player->y,	p->player->x,	sprPlayer[p->player->type],	sprMap[screen[p->player->y][p->player->x]],p->player->id-1,0, p->player->action);
		// HUD player drawing
		if (p->player->type == 0 && mode != 3) draw_hudp(sprPlayer[0],p->player->id);
		p = p->next;
	}
	
	
	k++;
	if (k > 3000) k = 0;
	wrefresh(game_win);
	wrefresh(hud_win);
}

void update_hud(int g, char f[(CHROMO_LENGTH / GENE_LENGTH) + 1], int h)
{
	gen = g;
	fittest = f;
	hp = h;
}

void del_stuff()
{
	delwin(game_win);
	delwin(hud_win);
}

void scoreboard(int *scores, int num, int winner){
	int i,j,k;
	short col1, col2;
	static int playercolor[10]= {15, 14, 13, 12, 11, 10, 9, 2};
	
	
	clear();
	refresh();

	for (i=0; i<COLS;i++){
		attron(COLOR_PAIR(j=rand()%256));
		mvprintw(0,i,"%c",0xDF);
		attroff(COLOR_PAIR(j));

		attron(COLOR_PAIR(j=rand()%256));
		mvprintw(LINES-1,i,"%c",0xDF);
		attroff(COLOR_PAIR(j));
	}
	
	for (i=1; i<LINES-1; i++){
		attron(COLOR_PAIR(j=rand()%256));
		mvprintw(i, 0, "%c", 0xDF);
		attroff(COLOR_PAIR(j));

		attron(COLOR_PAIR(j=rand()%256));
		mvprintw(i, 1, "%c", 0xDF);
		attroff(COLOR_PAIR(j));

		attron(COLOR_PAIR(j=rand()%256));
		mvprintw(i, COLS-1, "%c", 0xDF);
		attroff(COLOR_PAIR(j));

		attron(COLOR_PAIR(j=rand()%256));
		mvprintw(i, COLS-2, "%c", 0xDF);
		attroff(COLOR_PAIR(j));
	}


	for (i=0; i<num; i++){
		for (j=0; j<4; j++)
			for (k=0; k<6; k++){


				if (sprPlayer[0].col1[j][k]==16) col1=2;
				else 
					if (sprPlayer[0].col1[j][k]==17) col1= playercolor[i];
					else col1 = sprPlayer[0].col1[j][k];

				if (sprPlayer[0].col2[j][k]==16) col2=2;
				else
					if (sprPlayer[0].col2[j][k]==17) col2= playercolor[i];
					else col2 = sprPlayer[0].col2[j][k];

				attron (COLOR_PAIR(col1*16+col2));
				mvprintw (i*6+3+j, k+4, "%c",sprPlayer[0].ch[j][k]);
				attroff (COLOR_PAIR(col1*16+col2));
			}
		for (j=0; j<scores[i]; j++){
			attron (COLOR_PAIR(224));
			mvprintw(i*6+3, 15+j*9, "%c%c %c%c", 0xDC, 0xDC, 0xDC, 0xDC);
			mvprintw(i*6+4, 15+j*9, " %c%c%c ",0xDB, 0xDB,0xDB);
			mvprintw(i*6+5, 15+j*9, " %c%c%c ",0xDF, 0xDB,0xDF);
			mvprintw(i*6+6, 15+j*9, " %c%c%c ",0xDC,0xDB,0xDC);
			attroff (COLOR_PAIR(224));
			if (winner-1 == i && j==scores[i]-1) {
				attron(COLOR_PAIR(15*16));
				mvprintw(i*6+3, 15+j*9-2,"*");
				mvprintw(i*6+4, 15+j*9+6,"*");
				mvprintw(i*6+5, 15+j*9-1,"*");
				attroff(COLOR_PAIR(15*16));
			}
		}

		
	}
	refresh();
	Sleep(1500);
	flushinp();
}

void portrait(int ID)
{
	int i,j,col1,col2;
	
	static int playercolor[10]= {15, 14, 13, 12, 11, 10, 9, 2};

	Sprite spr = sprPlayer[0];

	for (i=4;i < 12 ;i++){
		wattron(story_win,COLOR_PAIR(128));
		mvwprintw(story_win, 0, i, "%c", 0xDC);
		mvwprintw(story_win,5,i,"%c", 0xDF);
		wattroff(story_win, COLOR_PAIR(128));
	}
	for (i=1;i<5;i++){
		wattron(story_win,COLOR_PAIR(128));
		mvwprintw(story_win, i,4, "%c",0xDB);
		mvwprintw(story_win,i,11,"%c",0xDB);
		wattroff(story_win, COLOR_PAIR(128));
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
					

					wattron(story_win,COLOR_PAIR(col1*16+col2));
					mvwprintw(story_win, i+1, j + 5,"%c",(col1==0&&col2==0)?' ':spr.ch[i][ j ]);
					wattroff(story_win,COLOR_PAIR(col1*16+col2));
				}
}
void story_time(int boss)
{
	int i;
	char ch, *text;
	bool skip = FALSE;

	story_win = newwin(7, COLS, 0, 0);
	nodelay(story_win, TRUE);
	box(story_win, 0, 0);
	
	switch (boss)
	{
	case 1:
		// boss 1
		portrait(boss + 4);
		text = "So, we meet again! nenexexedadada! ";
		for (i = 0; text[i] != '\0'; i++)
		{
			mvwprintw(story_win, 3, 16 + i, "%c", text[i]);
			if ((ch = wgetch(story_win)) == ' ') skip = TRUE;
			if (!skip)
				if (i == 3 || i == 18) Sleep(500);
				else Sleep(50);
		}
		skip = FALSE;		

		while ((ch = wgetch(story_win)) != ' ');
		
		// player
		portrait(1);
		mvwprintw(story_win, 3, 16, "Kojicu? Zar i ovde?!                ");
		
		while ((ch = wgetch(story_win)) != ' ');
		
		// boss 1
		portrait(boss + 4);
		mvwprintw(story_win, 3, 16, "muexexexexexe!                       ");
		
		while ((ch = wgetch(story_win)) != ' ');
		break;
	case 2:
		// boss 2
		portrait(boss + 4);
		mvwprintw(story_win, 3, 16, "JO SOJ TU PADRE! ");
		
		while ((ch = wgetch(story_win)) != ' ');
		
		// player
		portrait(1);
		mvwprintw(story_win, 3, 16, "NOOOOOOO!... wait... hm.. you're right... :/ ");
		
		while ((ch = wgetch(story_win)) != ' ');
		break;
	case 3:
		// boss 3
		portrait(boss + 4);
		mvwprintw(story_win, 3, 16, "Sure hope Dusan will come up with something wittier... ");
		
		while ((ch = wgetch(story_win)) != ' ');
		
		// player
		portrait(1);
		mvwprintw(story_win, 3, 16, "Yeap... :/                                             ");
		
		while ((ch = wgetch(story_win)) != ' ');
		
		// boss 3
		portrait(boss + 4);
		mvwprintw(story_win, 3, 16, "Wanna fight?                                           ");
		
		while ((ch = wgetch(story_win)) != ' ');
		
		// player
		portrait(1);
		mvwprintw(story_win, 3, 16, "k                                                      ");
		
		while ((ch = wgetch(story_win)) != ' ');
		break;
	}

	delwin(story_win);
}