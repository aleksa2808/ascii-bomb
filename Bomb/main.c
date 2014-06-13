#include <curses.h>
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#define WIDTH 30
#define HEIGHT 20 
#define MAP_SPRITE_NUM 20

int sdon;
int demoon;

typedef struct{
	unsigned char ch[4][6];
	short col1[4][6],col2[4][6];
}	Sprite;
Sprite sprPlayer[10];



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



struct m_list choices[10];
short menu_size;
void print_menu(WINDOW*, int, struct m_list);

extern int campaign(void);
extern int battle(int, int, int);
void updConfigs();




int main()
{	FILE *dat;
	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c,c2;
	int i,j,k,l,tint;
	int menu_start;
	char *splash = "SPLASH", *screen = "SCREEN!", dumpch, tempstr[50],t1,t2;
	int act_menu, ret_menu;
	int ptr;
	int len;
	int plr;
	int bts;
	int wns;
	int rep;
	int tmp;
	WINDOW *num_win;
	srand(time(0));
		
	//configs
	dat = fopen ("data/configs.txt","r");
	if (dat!=NULL) {
		fscanf(dat,"%d %d",&sdon,&demoon);
		fclose(dat);
	}
	else{
		sdon = 1;
		demoon = 1;
		updConfigs();
	}
	//menu forming
	dat = fopen ("data/menu.txt","r");
	fscanf(dat,"%d", &menu_size);
	i=0;
	while (i<menu_size){
		fscanf(dat,"%d%d", &choices[i].num,&choices[i].ret);
		choices[i].men = (smth*) malloc (sizeof(smth));
		for (j=0; j< choices[i].num; j++){
			fscanf(dat,"%d", &choices[i].men[j].id);
			dumpch = getc(dat);
			len=0;
			while (1){
				dumpch = getc(dat);
				if (dumpch!='\n' && dumpch!=EOF) {
					tempstr[len]=dumpch;
					len++;
				}
				else break;
			}
			tempstr[len]='\0';
			choices[i].men[j].nam = (char*) malloc (strlen(tempstr)*sizeof(char));
			strcpy(choices[i].men[j].nam,tempstr);
		}
		i++;
		
	}
	fclose(dat);

	//player sprite loading
	dat = fopen("data/playersprites.txt","r");
	fscanf(dat,"%d\n",&tint);
	for (tmp=0; tmp<tint; tmp++){
	for (i=0;i<4;i++)
		for (j=0;j<6;j++)
			sprPlayer[tmp].ch[i][j]=0xDF;
	
		for (i=0;i<4;i++){
			for (j=0;j<6;j++){
				fscanf(dat,"%c",&t1);
				if (t1=='x'){
					fscanf(dat,"%d%c%c",&l,&t1,&t2);
					sprPlayer[tmp].ch[i][j]=l;
				}
				else fscanf(dat,"%c",&t2);
				if (t1==t2) sprPlayer[tmp].ch[i][j]=' ';
				sprPlayer[tmp].col1[i][j]=t1-'a';
				sprPlayer[tmp].col2[i][j]=t2-'a';
			}
			fscanf(dat,"%c",&t1);
		}
	}
	fclose(dat);



	initscr();
	noecho();
	cbreak();
	curs_set(0);
	start_color();
	for (i=0;i<16;i++)
		for (j=0;j<16;j++)
			init_pair(16*i+j,i,j);
	resize_term (60,150);
	/* Splash screen */
	for (i = 0; i < COLS / 2 - 4; i++){
		clear();
		mvprintw(LINES / 2, i, "%s", splash);
		mvprintw(LINES / 2 + 1, COLS - 7 - i, "%s", screen);
		refresh();
		Sleep(15);
	}
	Sleep(200);
	
	flushinp();
	clear();
	refresh();


	menu_win = newwin(HEIGHT, WIDTH, (int)((float)(LINES-HEIGHT)/2),(int)((float)(COLS-WIDTH)/2));
	num_win = newwin(7,30,(LINES-7)/2,(COLS-30)/2);

	keypad(num_win,TRUE);
	nodelay(num_win, TRUE);
	keypad(menu_win, TRUE);
    nodelay(menu_win, TRUE);
	menu_start = clock();
	ptr=0;
	act_menu=0;
	ret_menu=-1;

	while (1){
		if ( demoon &&(menu_start + 15 * CLOCKS_PER_SEC <= clock())) {
			clear();
			refresh();
			battle(0, 8, 1);
			resize_term(60,150);
			menu_start = clock();
		}
		print_menu(menu_win, ptr, choices[act_menu]); 
		
		c= wgetch(menu_win);
		switch (c){
			case KEY_UP:
				if(sdon) PlaySound(TEXT("sounds/select.wav"), NULL, SND_ASYNC | SND_FILENAME);
				menu_start = clock();
				if (ptr==0) ptr = choices[act_menu].num-1;
				else ptr--;
				break;
			case KEY_DOWN:
				if(sdon) PlaySound(TEXT("sounds/select.wav"), NULL, SND_ASYNC | SND_FILENAME);
				menu_start = clock();
				if (ptr==choices[act_menu].num-1) ptr = 0;
				else ptr++;
				break;
			case ' ': case 10:
				if(sdon) PlaySound(TEXT("sounds/confirm.wav"), NULL, SND_ASYNC | SND_FILENAME);
				switch(choices[act_menu].men[ptr].id){
					case 10: 
						act_menu = 1;
						ptr = 0;
						wclear(menu_win);
						break;
					case 0: 
						delwin(menu_win);
						clrtoeol();
						refresh();
						endwin();
						return 0;
					case 20:
						clear();
						refresh();
						campaign();
						resize_term(60,150);
						menu_start = clock();
						break;
					case 21:
						
						box(num_win,0,0);
						wrefresh(num_win);
						rep=1;
						plr=2;
						bts=6;
						wns=2;

						while (rep==1){
							wclear(num_win);
							

							wattron(num_win,COLOR_PAIR(24));
							for (i=0;i<200;i++) wprintw(num_win," ");
							mvwprintw(num_win,2,2,"Izaberite broj igraca:");
							mvwprintw(num_win, 3,2,"%c %d %c", 0xAE, plr, 0xAF);
							wattroff(num_win,COLOR_PAIR(24));

							wattron(num_win,COLOR_PAIR(129));
							box(num_win,0,0);
							wattroff(num_win,COLOR_PAIR(129));

							refresh();
							wrefresh(num_win);
							c2 = wgetch(num_win);
							switch(c2){
								case 32: case 10:
									rep=0;
									break;
								case KEY_RIGHT:
									if (plr==1) plr++;
									break;
								case KEY_LEFT:
									if (plr==2) plr--;
									break;
								case 27:
									rep=-1;
									break;
								}
						Sleep(50);
						}
						if (rep==-1) break; 
						else rep=1;

						while (rep==1){
							wclear(num_win);
							wattron(num_win,COLOR_PAIR(24));
							for (i=0;i<200;i++) wprintw(num_win," ");
							mvwprintw(num_win,2,2,"Izaberite broj botova:");
							mvwprintw(num_win, 3,2,"%c %d %c", 0xAE, bts, 0xAF);
							wattroff(num_win,COLOR_PAIR(24));

							wattron(num_win,COLOR_PAIR(129));
							box(num_win,0,0);
							wattroff(num_win,COLOR_PAIR(129));

							refresh();
							wrefresh(num_win);
							c2 = wgetch(num_win);
							switch(c2){
								case 32: case 10:
									rep=0;
									break;
								case KEY_RIGHT:
									if (bts<8-plr) bts++;
									break;
								case KEY_LEFT:
									if (bts>2-plr) bts--;
									break;
								case 27:
									rep=-1;
									break;
								}
						Sleep(50);
						}
						if (rep==-1) break; 
						else rep=1;

						while (rep==1){
							wclear(num_win);

							wattron(num_win,COLOR_PAIR(24));
							for (i=0;i<200;i++) wprintw(num_win," ");
							mvwprintw(num_win,2,2,"Izaberite broj pobeda:");
							mvwprintw(num_win, 3,2,"%c %d %c", 0xAE, wns, 0xAF);
							wattroff(num_win,COLOR_PAIR(24));

							wattron(num_win,COLOR_PAIR(129));
							box(num_win,0,0);
							wattroff(num_win,COLOR_PAIR(129));

							refresh();
							wrefresh(num_win);
							c2 = wgetch(num_win);
							switch(c2){
								case 32: case 10:
									rep=0;
									break;
								case KEY_RIGHT:
									if (wns<5) wns++;
									break;
								case KEY_LEFT:
									if (wns>1) wns--;
									break;
								case 27:
									rep=-1;
									break;
								}
						Sleep(50);
						}
						if (rep==-1) break;

						battle(plr, bts, wns);
						resize_term(60,150);
						menu_start = clock();
						break;
					case 13:
						act_menu=2;
						ptr=0;
						wclear(menu_win);
						break;
					case 11:
						act_menu=3;
						ptr=0;
						wclear(menu_win);
						break;
					case 40:
						sdon= !sdon;
						wclear(menu_win);
						updConfigs();
						break;
					case 41:
						demoon = !demoon;
						wclear(menu_win);
						updConfigs();
						break;
					case 12:
						act_menu=4;
						ptr=0;
						wclear(menu_win);
						break;
				}
				break;
			case 27: 
				if (choices[act_menu].ret>=0) {
					act_menu = choices[act_menu].ret;
					ptr=0;
					wclear(menu_win);
				}
				break;
		}
		Sleep(20);
	}
	
	delwin(menu_win);
	clrtoeol();
	refresh();
	endwin();
	return 0;
}

void print_menu(WINDOW *menu_win, int ptr, struct m_list m_choices){
	int x,y,i;
	x=2; y=2;

	attron(COLOR_PAIR(51));
	for (i=0;i<COLS * LINES;i++) printw(" ");
	attroff(COLOR_PAIR(51));

	wattron(menu_win,COLOR_PAIR(24));
	for (i=0;i<900;i++) wprintw(menu_win," ");
	wattroff(menu_win,COLOR_PAIR(24));

	wattron(menu_win,COLOR_PAIR(129));
	box(menu_win,0,0);
	wattroff(menu_win,COLOR_PAIR(129));
		
	wattron(menu_win,COLOR_PAIR(24));
	for (i=0; i<m_choices.num; i++){
		switch (m_choices.men[i].id){
			case 40:
				mvwprintw(menu_win, y+2*i, x, "%c %s\t-\t%s",(i==ptr?0xAF:' '), m_choices.men[i].nam, sdon==1?"ON":"OFF");
				break;
			case 41:
				mvwprintw(menu_win, y+2*i, x, "%c %s\t-\t%s",(i==ptr?0xAF:' '), m_choices.men[i].nam, demoon==1?"ON":"OFF");
				break;
			default:
				mvwprintw(menu_win, y+2*i, x, "%c %s",(i==ptr?0xAF:' '), m_choices.men[i].nam);
				break;
			}
		
	}
	wattroff(menu_win,COLOR_PAIR(24));
	i=i;
	wrefresh(menu_win);
	refresh();
}

void updConfigs(){
	FILE *dat;
	dat = fopen ("data/configs.txt","w");
	fprintf(dat,"%d %d", sdon, demoon);
	fclose(dat);
}