#include <curses.h>
 
int m, n;
WINDOW *game_win, *hud_win;
 
void init_screen(int mm, int nn)
{
        m = mm;
        n = nn;
 
        resize_term(50, 170);
 
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_GREEN);
        init_pair(2, COLOR_BLUE, COLOR_BLUE);
        init_pair(3, COLOR_WHITE, COLOR_WHITE);
        init_pair(4, COLOR_RED, COLOR_RED);
		init_pair(5, COLOR_MAGENTA,COLOR_MAGENTA);
       
        game_win = newwin(m * 2 + 1, n * 3 + 1, 2, 3);
        keypad(game_win, TRUE);
        nodelay(game_win, TRUE);
       
        hud_win = newwin(8, 30, 5, COLS - 35);
        box(hud_win, 0, 0);
        mvwprintw(hud_win, 2, 6, "Bomberman! \\(^_^)/");
        mvwprintw(hud_win, 4, 5, "Press T to get back");
        mvwprintw(hud_win, 5, 4, "  to the main menu.");
        wrefresh(hud_win);
}
 
void draw(char **screen)
{
        int i, j;
 
        wclear(game_win);
        for (i = 0; i < m; i++)
        {
                for (j = 0; j < n; j++)
                {
                        wattron(game_win, COLOR_PAIR(screen[i][j] + 1));
                        mvwprintw(game_win, i*2,j*3,"%d%d%d",screen[i][j],screen[i][j],screen[i][j]);
                        mvwprintw(game_win, i*2+1,j*3,"%d%d%d",screen[i][j],screen[i][j],screen[i][j]);
                        //wprintw(game_win, "%d", screen[i][j]);
                        wattroff(game_win, COLOR_PAIR(screen[i][j] + 1));
                }
                wprintw(game_win, "\n");
        }
}