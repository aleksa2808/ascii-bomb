#include <curses.h>
#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#define M 50
#define N 79

void draw(char screen[M][N])
{
	int i, j;

	clear();
	for (i = 0; i < M; i++)
	{
		for (j = 0; j < N; j++)
		{
			attron(COLOR_PAIR(screen[i][j] + 1));
			printw("%d", screen[i][j]);
			attroff(COLOR_PAIR(screen[i][j] + 1));
		}
		printw("\n");
	}

}

int main(void)
{
	int i, j, x = 0, y = 0, ch, mx, my;
	char screen[M][N];

	initscr();			/* Start curses mode 		*/
	raw();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */
	curs_set(0);
	start_color();			/* Start color 			*/
	init_pair(2, COLOR_BLACK, COLOR_BLACK);
	
	init_pair(1, COLOR_GREEN, COLOR_GREEN);
	init_pair(3, COLOR_RED, COLOR_RED);
	for (i = 0; i < M; i++)
	{
		for (j = 0; j < N; j++)
		{
			screen[i][j] = 0;
		}
	}

	screen[x][y] = 1;

	srand(time(0));
	for (i = 0; i < 500; i++)
	{
		my = rand()%50;
		mx = rand()%79;
		screen[my][mx]=2;
	}
	
	draw(screen);

	//time_t last = time(0);
	while (1)
	{
		ch = getch();
		switch (ch)
		{
		case KEY_LEFT:
		case 'a':
			if (x && screen[y][x - 1] != 2) {
				screen[y][x] = 0;
				x--;
				screen[y][x] = 1;
			}
			break;
		case KEY_RIGHT:
		case 'd':
			if (x < N - 1 && screen[y][x + 1] != 2) {
				screen[y][x] = 0;
				x++;
				screen[y][x] = 1;
			}
			break;
		case KEY_UP:
		case 'w':
			if (y && screen[y - 1][x] != 2) {
				screen[y][x] = 0;
				y--;
				screen[y][x] = 1;
			}
			break;
		case KEY_DOWN:
		case 's':
			if (y < M - 1 && screen[y + 1][x] != 2) {
				screen[y][x] = 0;
				y++;
				screen[y][x] = 1;
			}
		}
		

		/*if (time(0) - last)
		{
			last = time(0);
		}*/
		//Sleep(20);
		draw(screen);
	}
	
	endwin();			/* End curses mode		  */

	return 0;
}