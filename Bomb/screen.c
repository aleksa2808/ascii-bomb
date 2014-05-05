#include <curses.h>
 
int m, n;
WINDOW *game_win, *hud_win;
 

#define upleft (((screen[i-1][j]==3)&&(screen[i][j-1]==3)&&(screen[i-1][j-1]==3)?' ':(((screen[i-1][j]==3)&&(screen[i][j-1]==3))?0xD9:(((screen[i][j-1]==3)?0xC4:((screen[i-1][j]==3)?0xB3:0xDA))))))
#define pureup		(screen[i-1][j]==3)?(' '):(0xC4)
#define upright (((screen[i-1][j]==3)&&(screen[i][j+1]==3)&&(screen[i-1][j+1]==3)?' ':(((screen[i-1][j]==3)&&(screen[i][j+1]==3))?0xC0:(((screen[i][j+1]==3)?0xC4:((screen[i-1][j]==3)?0xB3:0xBF))))))
//((screen[i-1][j]==3)&&(screen[i][j+1]==3)&&(screen[i-1][j+1]==3))?(' '):((screen[i-1][j]==3)&&(screen[i][j+1]==3))?(0xC0):(screen[i][j+1]==3)?(0xDD):(screen[i-1][j]==2?)?(0xB3):(0xBF)
#define downleft (((screen[i+1][j]==3)&&(screen[i][j-1]==3)&&(screen[i+1][j-1]==3)?' ':(((screen[i+1][j]==3)&&(screen[i][j-1]==3))?0xBF:(((screen[i][j-1]==3)?0xC4:((screen[i+1][j]==3)?0xB3:0xC0))))))
//downleft ((screen[i+1][j]==3)&&(screen[i][j-1]==3)&&(screen[i+1][j-1]==3))?(' '):((screen[i+1][j]==3)&&(screen[i][j-1]==3))?(0xBF):(screen[i][j-1]==3)?(0xDD):(screen[i+1][j]==2?)?(0xB3):(0xC0)
#define puredown (screen[i+1][j]==3)?(' '):(0xC4)
#define downright (((screen[i+1][j]==3)&&(screen[i][j+1]==3)&&(screen[i+1][j+1]==3)?' ':(((screen[i+1][j]==3)&&(screen[i][j+1]==3))?0xDA:(((screen[i][j+1]==3)?0xC4:((screen[i+1][j]==3)?0xB3:0xD9))))))
//((screen[i+1][j]==3)&&(screen[i][j+1]==3)&&(screen[i+1][j+1]==3))?(' '):((screen[i+1][j]==3)&&(screen[i][j+1]==3))?(0xDA):(screen[i][j+1]==3)?(0xDD):(screen[i+1][j]==2?)?(0xB3):(0xD9)

void init_screen(int mm, int nn)
{
    m = mm;
    n = nn;
 
    resize_term(50, 170);
 
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_GREEN);
    init_pair(2, COLOR_BLUE, COLOR_GREEN);
    init_pair(3, COLOR_WHITE, COLOR_WHITE);
    init_pair(4, COLOR_YELLOW, COLOR_CYAN);
    init_pair(5, COLOR_MAGENTA,COLOR_GREEN);
	init_pair(6, COLOR_RED, COLOR_RED);

    init_pair(12, 0, 14);
    init_pair(13, 0, 13);
    init_pair(14, 0, 12);
       
    game_win = newwin(m * 2 + 1, n * 3 + 1, 2, 3);
    keypad(game_win, TRUE);
    nodelay(game_win, TRUE);
        
    hud_win = newwin(20, 30, 5, COLS - 35);
    box(hud_win, 0, 0);
    mvwprintw(hud_win, 2, 6, "Bomberman! \\(^_^)/");
    mvwprintw(hud_win, 4, 5, "Press T to get back");
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

    wrefresh(hud_win);
}
 
void draw(char **screen)
{
    int i, j;
	char c1,c2,c3,c4,c5,c6;
    wclear(game_win);

    for (i = 0; i < m; i++)
    {
            for (j = 0; j < n; j++)
            {
                    wattron(game_win, COLOR_PAIR(screen[i][j] + 1));
					screen[i][j]==1?(c1=c3='_',c2='m',c4='(',c5='\"',c6=')'):screen[i][j]==2?(c1=0xDA,c2=0xC4,c3=0xBF,c4=0xC0,c5=0xC4,c6=0xD9):screen[i][j]==3?(c1=upleft,c2=pureup,c3=upright,c4=downleft,c5=puredown,c6=downright):screen[i][j]==4?(c1=0xCF,c2=0xBF,c3=' ',c4='(',c5='_',c6=')'):(c1=c2=c3=c4=c5=c6=' ');/*mesto za eksploziju*/
                        
						
					mvwprintw(game_win, i*2,j*3,"   ");
					mvwprintw(game_win, i*2+1,j*3,"   ");
					mvwprintw(game_win, i*2,j*3,"%c%c%c",c1,c2,c3);
					mvwprintw(game_win, i*2+1,j*3,"%c%c%c",c4,c5,c6);
                    //wprintw(game_win, "%d", screen[i][j]);
                    wattroff(game_win, COLOR_PAIR(screen[i][j] + 1));
            }
            wprintw(game_win, "\n");
    }
			wrefresh(game_win);
}