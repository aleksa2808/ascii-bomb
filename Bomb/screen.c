#include <curses.h>
#include <time.h>
int m, n, k;
WINDOW *game_win, *hud_win;
 
#define upleft (((screen[i-1][j]==3)&&(screen[i][j-1]==3)&&(screen[i-1][j-1]==3)?' ':(((screen[i-1][j]==3)&&(screen[i][j-1]==3))?0xD9:(((screen[i][j-1]==3)?0xC4:((screen[i-1][j]==3)?0xB3:0xDA))))))
#define pureup	(screen[i-1][j]==3)?(' '):(0xC4)
#define upright (((screen[i-1][j]==3)&&(screen[i][j+1]==3)&&(screen[i-1][j+1]==3)?' ':(((screen[i-1][j]==3)&&(screen[i][j+1]==3))?0xC0:(((screen[i][j+1]==3)?0xC4:((screen[i-1][j]==3)?0xB3:0xBF))))))
#define downleft (((screen[i+1][j]==3)&&(screen[i][j-1]==3)&&(screen[i+1][j-1]==3)?' ':(((screen[i+1][j]==3)&&(screen[i][j-1]==3))?0xBF:(((screen[i][j-1]==3)?0xC4:((screen[i+1][j]==3)?0xB3:0xC0))))))
#define puredown (screen[i+1][j]==3)?(' '):(0xC4)
#define downright (((screen[i+1][j]==3)&&(screen[i][j+1]==3)&&(screen[i+1][j+1]==3)?' ':(((screen[i+1][j]==3)&&(screen[i][j+1]==3))?0xDA:(((screen[i][j+1]==3)?0xC4:((screen[i+1][j]==3)?0xB3:0xD9))))))

typedef struct {
	int id, x, y, health, bombs, bomb_range, action, immortal_end, last_move;
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

void init_screen(int mm, int nn)
{
    m = mm;
    n = nn;
	k=0;
 
    resize_term(5 + m * 2 + 1, 3 + n * 3 + 3);
 
    start_color();
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
       
    game_win = newwin(m * 2 + 1 , n * 3, 5, 3);
    keypad(game_win, TRUE);
    nodelay(game_win, TRUE);
        
    hud_win = newwin(5, COLS, 0, 0);
    box(hud_win, 0, 0);
    mvwprintw(hud_win, 2, COLS / 2 - 9, "Bomberman! \\(^_^)/");
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

    wrefresh(hud_win);
}
void draw(char **screen, struct BombList *b, struct PlayerList *p)
{	
    int i, j;
	char c1,c2,c3,c4,c5,c6;
    wclear(game_win);
    for (i = 0; i < m; i++)
    {
            for (j = 0; j < n; j++)
            {
                    screen[i][j]==5?k%20<10?wattron(game_win,COLOR_PAIR(6)):wattron(game_win,COLOR_PAIR(7)):wattron(game_win, COLOR_PAIR(screen[i][j] + 1));
					screen[i][j]==2?(c1=0xDA,c2=0xC4,c3=0xBF,c4=0xC0,c5=0xC4,c6=0xD9):screen[i][j]==3?(c1=upleft,c2=pureup,c3=upright,c4=downleft,c5=puredown,c6=downright):screen[i][j]==4?(c1=0xCF,c2=0xBF,c3=' ',c4='(',c5='_',c6=')'):(c1=c3=c5=c2=c4=c6=screen[i][j]==5?0xB1:' ');/*mesto za eksploziju*/
					mvwprintw(game_win, i*2,j*3,"%c%c%c",c1,c2,c3);
					mvwprintw(game_win, i*2+1,j*3,"%c%c%c",c4,c5,c6);
                    //wprintw(game_win, "%d", screen[i][j]);
                    screen[i][j]==5?k%20<10?wattroff(game_win,COLOR_PAIR(6)):wattroff(game_win,COLOR_PAIR(7)):wattroff(game_win, COLOR_PAIR(screen[i][j] + 1));
            }
            wprintw(game_win, "\n");
    }
	while (b != NULL){
		wattron(game_win,COLOR_PAIR(5));
		b->bomb->end_time - CLOCKS_PER_SEC >=clock()?((k%3==0?(c1=0xCF):k%3==1?(c1='*'):(c1='+')),c2=0xBF,c3=' ',c4='(',c5='_',c6=')'):((k%3==0?(c2=0xCF):k%3==1?(c2='*'):(c2='+')),c1=' ',c3=' ',c4='(',c5='_',c6=')');
		mvwprintw(game_win, b->bomb->y*2,b->bomb->x*3,"%c%c%c",c1,c2,c3);
		mvwprintw(game_win, b->bomb->y*2+1,b->bomb->x*3,"%c%c%c",c4,c5,c6);
		wattroff(game_win,COLOR_PAIR(5));

		b=b->next;
	}
	while (p != NULL){
		if (p->player->immortal == TRUE && k % 20 > 10)
			wattron(game_win, COLOR_PAIR(1));
		else
			wattron(game_win, COLOR_PAIR(2));
		c1=c3=0xDC;c2=c4=c5=c6=0xDF;
		//c1=c3='_', c2='m', c4='(', c5='\"', c6=')';
		mvwprintw(game_win, p->player->y*2,p->player->x*3,"%c%c%c",c1,c2,c3);
		mvwprintw(game_win, p->player->y*2+1,p->player->x*3,"%c%c%c",c4,c5,c6);
		if (p->player->immortal == TRUE && k % 20 > 10) wattroff(game_win, COLOR_PAIR(1));
		else wattroff(game_win, COLOR_PAIR(2));

		p = p->next;
	}
	k++;
	if (k > 3000) k = 0;
	wrefresh(game_win);
}
void del_stuff()
{
	delwin(game_win);
	delwin(hud_win);
}