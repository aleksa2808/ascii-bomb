#include <curses.h>
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "functions.h"


int sdon;
int demoon;
int transon;
Sprite sprPlayer[10];

struct m_list choices[10];
short menu_size;
void print_menu(WINDOW*, int, struct m_list, int tr);



int main()
{	FILE *dat;
	WINDOW *menu_win, *num_win;
	int highlight = 1;
	int choice = 0;
	int c,c2;
	int i,j,k,l,tint;
	int menu_start;
	char *splash = "Error 404:", *screen = "Name Not Found", dumpch, tempstr[50],t1,t2;
	int act_menu, ret_menu;
	int ptr;
	int len;
	int plr;
	int bts;
	int wns;
	int rep;
	int tmp;
	int dif;
	struct ScoreList *slist = NULL, *s = NULL, *t, *p = NULL;
	int i1, j1, chksum, sum, k1;
	char ch, *pts;
	srand(time(0));
		
	//configs
	dat = fopen ("data/configs.txt","r");
	if (dat!=NULL) {
		fscanf(dat,"%d %d %d",&sdon,&demoon, &transon);
		fclose(dat);
	}
	else{
		sdon = 1;
		demoon = 1;
		transon = 1;
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
	resize_term(50,100);
	/* Splash screen */
	for (i = 0; i < COLS / 2 - 4; i++){
		clear();
		mvprintw(LINES / 2, i, "%s", splash);
		mvprintw(LINES / 2 + 1, COLS - strlen(screen) - i, "%s", screen);
		refresh();
		Sleep(15);
	}
	Sleep(700);
	
	flushinp();
	clear();
	refresh();


	menu_win = newwin(HEIGHT, WIDTH, (int)((float)(LINES-HEIGHT)/2) + 3,(int)((float)(COLS-WIDTH)/2));
	num_win = newwin(7,26,(LINES-7)/2 + 4,(COLS-30)/2 + 2);

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
			battle(0, 8, 1, 2);
			resize_term(50,100);
			menu_start = clock();
		}
		print_menu(menu_win, ptr, choices[act_menu],1); 
		
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
						resize_term(50,100);
						menu_start = clock();
						break;
					case 21:
						
						box(num_win,0,0);
						wrefresh(num_win);
						rep=1;
						plr=1;
						bts=3;
						wns=3;
						dif=2;
						while (rep==1){
							wclear(num_win);
							

							wattron(num_win,COLOR_PAIR(24));
							for (i=0;i<200;i++) wprintw(num_win," ");
							mvwprintw(num_win,2,2,"AMOUNT OF PLAYERS:");
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
									menu_start = clock();
									break;
								}
						Sleep(20);
						}
						if (rep==-1) break; 
						else rep=1;

						while (rep==1){
							wclear(num_win);
							wattron(num_win,COLOR_PAIR(24));
							for (i=0;i<200;i++) wprintw(num_win," ");
							mvwprintw(num_win,2,2,"AMOUNT OF BOTS:");
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
						Sleep(20);
						}
						if (rep==-1) break; 
						else rep=1;

						while (rep==1){
							wclear(num_win);

							wattron(num_win,COLOR_PAIR(24));
							for (i=0;i<200;i++) wprintw(num_win," ");
							mvwprintw(num_win,2,2,"AMOUNT OF WINS:");
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
						Sleep(20);
						}
						if (rep==-1) break; 
						else rep=1;

						while (bts && rep==1){
							wclear(num_win);

							wattron(num_win,COLOR_PAIR(24));
							for (i=0;i<200;i++) wprintw(num_win," ");
							mvwprintw(num_win,2,2,"DIFFICULTY:");
							mvwprintw(num_win, 3,2,"%c %s %c", 0xAE, dif == 1 ? "EASY" : dif == 2 ? "MEDIUM" : "HARD", 0xAF);
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
									if (dif<3) dif++;
									break;
								case KEY_LEFT:
									if (dif>1) dif--;
									break;
								case 27:
									rep=-1;
									break;
								}
						Sleep(20);
						}

						if (rep==-1) break;

						battle(plr, bts, wns, dif);
						resize_term(50,100);
						menu_start = clock();
						break;
					case 22:
						clear();
						refresh();
						training_area();
						resize_term(50,100);
						menu_start = clock();
						break;
					case 11:
						act_menu=2;
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
					case 42:
						transon = !transon;
						wclear(menu_win);
						updConfigs();
						break;
					case 12:
						act_menu=3;
						ptr=0;
						wclear(menu_win);
						break;
					case 50:
						do {
							ch=wgetch(menu_win);
							print_menu(menu_win,ptr,choices[act_menu],0);
							wclear(menu_win);
							wattron(menu_win,COLOR_PAIR(24));
							for (i=0;i<900;i++) wprintw(menu_win," ");
							wattroff(menu_win,COLOR_PAIR(24));

							wattron(menu_win,COLOR_PAIR(129));
							box(menu_win,0,0);
							wattroff(menu_win,COLOR_PAIR(129));
							

							wattron(menu_win,COLOR_PAIR(24));
							mvwprintw(menu_win,2,2,"You are a penguin. With a top hat.");
							mvwprintw(menu_win,3,2,"You also have bombs.");
							mvwprintw(menu_win,4,2,"You also have enemies.");
							mvwprintw(menu_win,5,2,"Some of them also have bombs.");
							mvwprintw(menu_win,6,2,"Bombs can kill enemies.");
							mvwprintw(menu_win,7,2,"So, try to kill your enemies.");
							mvwprintw(menu_win,9,2,"Made by");
							mvwprintw(menu_win,10,2,"\t\tAleksa Pavlovic");
							mvwprintw(menu_win,11,2,"\t\tNikola Vaic");
							mvwprintw(menu_win,12,2,"\t\tDusan Mrvaljevic");
							wattroff(menu_win,COLOR_PAIR(24));

							wattron(menu_win,COLOR_PAIR(129));
							mvwprintw(menu_win,17,20-4,"CONTINUE");
							wattroff(menu_win,COLOR_PAIR(129));
							refresh();
							Sleep(20);
						} while (ch!=32 && ch!=27 && ch!=10);	
						wclear(menu_win);
						wattron(menu_win,COLOR_PAIR(24));
						for (i=0;i<900;i++) wprintw(menu_win," ");
						wattroff(menu_win,COLOR_PAIR(24));

						break;
					case 51:
						do {
							ch=wgetch(menu_win);
							if (ch == 'f')
							{
								clear();
								refresh();
								fun();
								resize_term(50,100);
							}
							print_menu(menu_win,ptr,choices[act_menu],0);
							wclear(menu_win);
							wattron(menu_win,COLOR_PAIR(24));
							for (i=0;i<900;i++) wprintw(menu_win," ");
							wattroff(menu_win,COLOR_PAIR(24));

							wattron(menu_win,COLOR_PAIR(129));
							box(menu_win,0,0);
							wattroff(menu_win,COLOR_PAIR(129));
							

							wattron(menu_win,COLOR_PAIR(24));
							mvwprintw(menu_win,2,2,"Arrow Keys\t- P1 movement");
							mvwprintw(menu_win,3,2,"Space Bar\t- P1 bomb set");
							mvwprintw(menu_win,4,2,"WASD Keys\t- P2 movement");
							mvwprintw(menu_win,5,2,"G Key\t\t- P2 bomb set");
							mvwprintw(menu_win,6,2,"Enter Key\t- Pause");
							mvwprintw(menu_win,7,2,"ESC Key\t- Back");
							mvwprintw(menu_win,12,2,"F Key\t- ???");
							wattroff(menu_win,COLOR_PAIR(24));

							wattron(menu_win,COLOR_PAIR(129));
							mvwprintw(menu_win,17,20-4,"CONTINUE");
							wattroff(menu_win,COLOR_PAIR(129));
							refresh();
							Sleep(20);
						} while (ch!=32 && ch!=27 && ch!=10);	
						wclear(menu_win);
						wattron(menu_win,COLOR_PAIR(24));
						for (i=0;i<900;i++) wprintw(menu_win," ");
						wattroff(menu_win,COLOR_PAIR(24));
						menu_start = clock();
						break;
					case 52:
						do {
							ch=wgetch(menu_win);
							print_menu(menu_win,ptr,choices[act_menu],0);
							wclear(menu_win);
							wattron(menu_win,COLOR_PAIR(24));
							for (i=0;i<900;i++) wprintw(menu_win," ");
							wattroff(menu_win,COLOR_PAIR(24));

							wattron(menu_win,COLOR_PAIR(129));
							box(menu_win,0,0);
							wattroff(menu_win,COLOR_PAIR(129));
							

							wattron(menu_win,COLOR_PAIR(24));
							mvwprintw(menu_win,2,2,"LIFE UP\t- H+");
							mvwprintw(menu_win,3,2,"RANGE UP\t- R+");
							mvwprintw(menu_win,4,2,"PUSHING\t- Boot");
							mvwprintw(menu_win,5,2,"WALL CLIMB\t- Ladders");
							mvwprintw(menu_win,6,2,"INVINCIBILITY\t- Top Hat");
							wattroff(menu_win,COLOR_PAIR(24));

							wattron(menu_win,COLOR_PAIR(129));
							mvwprintw(menu_win,17,20-4,"CONTINUE");
							wattroff(menu_win,COLOR_PAIR(129));
							refresh();
							Sleep(20);
						} while (ch!=32 && ch!=27 && ch!=10);	
						wclear(menu_win);
						wattron(menu_win,COLOR_PAIR(24));
						for (i=0;i<900;i++) wprintw(menu_win," ");
						wattroff(menu_win,COLOR_PAIR(24));
						menu_start = clock();
						break;
					
					case 30:
						dat = fopen("data/highscores.txt", "r");
						if (dat)
						{
							sum = 0;
							for (i1 = 0; i1 < 10; i1++)
							{
								while ((ch = fgetc(dat)) != '\n')
								{
									sum += ch;
								}
							}
							fscanf(dat, "%d", &chksum);

							if (sum == chksum)
							{
								fseek(dat, 0, SEEK_SET);
								for (i1 = 0; i1 < 10; i1++)
								{
									t = (struct ScoreList*) malloc(sizeof(struct ScoreList));

									while ((ch = fgetc(dat)) != ' ');

									t->name = NULL;

									j1 = 0;
									while ((ch = fgetc(dat)) != ' ')
									{
										if (j1 % 10 == 0) t->name = (char*) realloc(t->name, (j1 + 10) * sizeof(char));
										t->name[j1++] = ch;
									}
									if (j1 % 10 == 0) t->name = (char*) realloc(t->name, (j1 + 1) * sizeof(char));
									t->name[j1] = '\0';

									pts = NULL;

									j1 = 0;
									while ((ch = fgetc(dat)) != '\n')
									{
										if (j1 % 10 == 0) pts = (char*) realloc(pts, (j1 + 10) * sizeof(char));
										pts[j1++] = ch;
									}
									if (j1 % 10 == 0) pts = (char*) realloc(pts, (j1 + 1) * sizeof(char));
									pts[j1] = '\0';

									t->points = atoi(pts);
									t->next = NULL;

									if (!s) slist = t;
									else s->next = t;
									s = t;
								}
							}
							fclose(dat);
						}
						do{
							ch=wgetch(menu_win);
							t=slist;
							print_menu(menu_win, ptr, choices[act_menu],0); 
							wclear(menu_win);
							wattron(menu_win,COLOR_PAIR(24));
							for (i=0;i<900;i++) wprintw(menu_win," ");
							wattroff(menu_win,COLOR_PAIR(24));

						
							wattron(menu_win,COLOR_PAIR(129));
							box(menu_win,0,0);
							wattroff(menu_win,COLOR_PAIR(129));

							wattron(menu_win,COLOR_PAIR(24));
							mvwprintw(menu_win, 2, 14,"HIGH-SCORES");
							for (i=1; i<=10; i++){
								mvwprintw(menu_win, 3+i,1, " %2d. %-20s     %5d", i, t?t->name:" ", t?t->points:0);
								if (t) t=t->next;
							}
							wattron(menu_win,COLOR_PAIR(129));
							mvwprintw(menu_win,17,20-4,"CONTINUE");
							wattroff(menu_win,COLOR_PAIR(129));
							wattroff(menu_win,COLOR_PAIR(24));
							refresh();
							Sleep(20);
						} while (ch!=10	&&	ch!=32 && ch!=27);

						t=slist;
						while (t){
							p=t->next;
							free(t);
							t=p;
						}
						slist=p=s=t=NULL;
						wclear(menu_win);
						wattron(menu_win,COLOR_PAIR(24));
						for (i=0;i<900;i++) wprintw(menu_win," ");
						wattroff(menu_win,COLOR_PAIR(24));

						wattron(menu_win,COLOR_PAIR(129));
						box(menu_win,0,0);
						wattroff(menu_win,COLOR_PAIR(129));
						
						menu_start = clock();
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

	for (i=0; i<menu_size; i++){
		for (j=0; j<choices[i].num; j++){
			free (choices[i].men[j].nam);
		}
		free (choices[i].men);

	}
	
	delwin(menu_win);
	delwin(num_win);

	clrtoeol();
	refresh();
	endwin();
	return 0;
}

void print_menu(WINDOW *menu_win, int ptr, struct m_list m_choices, int tr){
	int x,y,i, xt = 16, yt = 5;
	static int k=0;
	x=2; y=2;
	
	//attron(COLOR_PAIR(51));
	//for (i=0;i<COLS * LINES;i++) printw(" ");
	//attroff(COLOR_PAIR(51));
	k++;
	if (k>5000) k = 0;
	attron(COLOR_PAIR(240));
	mvprintw(yt + 1, xt, "  ____   ____  __  __ ____  ______ _____  __  __          _   _ ");
	mvprintw(yt + 2, xt, " |  _ \\ / __ \\|  \\/  |  _ \\|  ____|  __ \\|  \\/  |   /\\   | \\ | |");
	mvprintw(yt + 3, xt, " | |_) | |  | | \\  / | |_) | |__  | |__) | \\  / |  /  \\  |  \\| |");
	mvprintw(yt + 4, xt, " |  _ <| |  | | |\\/| |  _ <|  __| |  _  /| |\\/| | / /\\ \\ | . ` |");
	mvprintw(yt + 5, xt, " | |_) | |__| | |  | | |_) | |____| | \\ \\| |  | |/ ____ \\| |\\  |");
	mvprintw(yt + 6, xt, " |____/ \\____/|_|  |_|____/|______|_|  \\_\\_|  |_/_/    \\_\\_| \\_|");
	attroff(COLOR_PAIR(240));

	attron (COLOR_PAIR(224));
	mvprintw(10,4,"*");
	mvprintw(15,2,"%c", k%200<100?'*':0xF9);
	
	mvprintw(40,21,"%c", k%100<50?0xF9:'*');
	mvprintw(30,9,"*");
	mvprintw(43,40,"%c", k%222<111?'*':'+');
	mvprintw(46, 70, "*");
	mvprintw(44,5,"*");
	mvprintw(30,86,"%c",k%700<350?'*':0xF9);
	mvprintw(5,91, "%c",k%312<156?'*':'+');
	mvprintw(13, 78,"%c", k%160<80?' ':'*');
	mvprintw(17,72,"*");
	mvprintw(19,92,"%c",k%123<62?0xF9:'*');
	attroff (COLOR_PAIR(224));

	attron (COLOR_PAIR(8*16));
	mvprintw(39,84,"\\__/");
	mvprintw(40, 85,"%c%c",0xDB,0xDB);
	mvprintw(41,83,"__%c%c__",0xDB,0xDB);
	mvprintw(43,84,"=%c%c=",0xDB,0xDB);
	mvprintw(44,85,"||");
	mvprintw(42,85,"||");
	attroff(COLOR_PAIR(8*16));
	attron(COLOR_PAIR(k%348/2<348/4?4*16:12*16));
	mvprintw(38,83,".    .");
	mvprintw(41,82,".");
	mvprintw(41,89,".");
	attroff(COLOR_PAIR(k%348/2<348/4?4*16:12*16));



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
			case 42:
				mvwprintw(menu_win, y+2*i, x, "%c %s\t-\t%s", (i==ptr?0xAF:' '), m_choices.men[i].nam, transon==1?"ON":"OFF");
				break;
			default:
				mvwprintw(menu_win, y+2*i, x, "%c %s",(i==ptr?0xAF:' '), m_choices.men[i].nam);
				break;
			}
		
	}
	wattroff(menu_win,COLOR_PAIR(24));
	i=i;
	if (tr){
	wrefresh(menu_win);
	refresh();
	}
}

void updConfigs(){
	FILE *dat;
	dat = fopen ("data/configs.txt","w");
	fprintf(dat,"%d %d %d", sdon, demoon, transon);
	fclose(dat);
}