#include <curses.h>
#include <Windows.h>
#include <time.h>
#define WIDTH 30
#define HEIGHT 10 

int startx = 0;
int starty = 0;

char *choices[] = { 
			"Arena 1",
			"Arena 2",
			"Exit game"
		  };
int n_choices = sizeof(choices) / sizeof(char *);
void print_menu(WINDOW *menu_win, int highlight);

extern int game(void);

int main()
{	
	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c;
	int i;
	char *splash = "SPLASH", *screen = "SCREEN!";

	initscr();
	noecho();
	cbreak();
	curs_set(0);
	startx = 5;
	starty = 3;
	

	/* Splash screen */
	for (i = 0; i < COLS / 2 - 4; i++)
	{
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


	menu_win = newwin(HEIGHT, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	while(1)
	{	
		print_menu(menu_win, highlight);
		c = wgetch(menu_win);
		switch(c)
		{	
		case KEY_UP:
			if(highlight == 1)
				highlight = n_choices;
			else
				--highlight;
			break;
		case KEY_DOWN:
			if(highlight == n_choices)
				highlight = 1;
			else 
				++highlight;
			break;
		case 10:
			choice = highlight;
			break;
		}

		switch(choice) 
		{
		case 1:
			clear();
			refresh();
			game();
			resize_term(25, 80);
			break;
		case 2:
			clear();
			refresh();
			game();
			resize_term(25, 80);
			break;
		case 3:
			clrtoeol();
			refresh();
			endwin();
			return 0;
		}
		choice = 0;
	}	

	return 0;
}


void print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;	

	x = 2;
	y = 2;
	box(menu_win, 0, 0);
	for(i = 0; i < n_choices; ++i)
	{	
		if(highlight == i + 1)
		{	
			wattron(menu_win, A_REVERSE); 
			mvwprintw(menu_win, y, x, "%s", choices[i]);
			wattroff(menu_win, A_REVERSE);
		}
		else
			mvwprintw(menu_win, y, x, "%s", choices[i]);
		++y;
	}
	wrefresh(menu_win);
}