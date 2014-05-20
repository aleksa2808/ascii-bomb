#include <curses.h>
#include <Windows.h>
#include <time.h>
#define WIDTH 30
#define HEIGHT 10 

int startx = 0;
int starty = 0;

char *choices[] = { 
			"NORMAL MODE",
			"MULTIPLAYER MODE",
			"BATTLE MODE",
			"EXIT GAME"
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
	int menu_start;
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
    nodelay(menu_win, TRUE);
	menu_start = clock();
	while(1)
	{			
		if (menu_start + 15 * CLOCKS_PER_SEC <= clock()) 
		{
			clear();
			refresh();
			game(0);
			resize_term(25, 80);
			menu_start = clock();
		}

		print_menu(menu_win, highlight);
		c = wgetch(menu_win);
		switch(c)
		{	
		case KEY_UP:
			PlaySound(TEXT("sounds/select.wav"), NULL, SND_ASYNC | SND_FILENAME);
			menu_start = clock();
			if(highlight == 1)
				highlight = n_choices;
			else
				--highlight;
			break;
		case KEY_DOWN:
			PlaySound(TEXT("sounds/select.wav"), NULL, SND_ASYNC | SND_FILENAME);
			menu_start = clock();
			if(highlight == n_choices)
				highlight = 1;
			else 
				++highlight;
			break;
		case 10:
		case ' ':
			PlaySound(TEXT("sounds/confirm.wav"), NULL, SND_ASYNC | SND_FILENAME);
			choice = highlight;
			break;
		}

		switch(choice) 
		{
		case 1:
			clear();
			refresh();
			//game(1);
			resize_term(25, 80);
			break;
		case 2:
			clear();
			refresh();
			game(2);
			resize_term(25, 80);
			menu_start = clock();
			break;
		case 3:
			clear();
			refresh();
			game(1);
			resize_term(25, 80);
			menu_start = clock();
			break;
		case 4:
			delwin(menu_win);
			clrtoeol();
			refresh();
			endwin();
			return 0;
		}
		choice = 0;
	}	
	
	delwin(menu_win);
	clrtoeol();
	refresh();
	endwin();
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