#include <curses.h>
#define M 15
#define N 31

void draw(char screen[M][N], WINDOW *local_win)
{
	int i, j;

	wclear(local_win);
	for (i = 0; i < M; i++)
	{
		for (j = 0; j < N; j++)
		{
			wattron(local_win, COLOR_PAIR(screen[i][j] + 1));
			wprintw(local_win, "%d", screen[i][j]);
			wattroff(local_win, COLOR_PAIR(screen[i][j] + 1));
		}
		wprintw(local_win, "\n");
	}
}

int main(void)
{
	/* ~Initialization~ */
	WINDOW *game_win, *hud_win;
	int i, j, x = 1, y = 1, ch;
	char screen[M][N];

	initscr();
	cbreak();
	//keypad(stdscr, TRUE);
	noecho();
	curs_set(0);

	start_color();
	init_pair(1, COLOR_GREEN, COLOR_GREEN);
	init_pair(2, COLOR_BLUE, COLOR_BLUE);
	init_pair(3, COLOR_BLACK | COLOR_WHITE, COLOR_BLACK | COLOR_WHITE);
	
	game_win = newwin(M + 1, N + 1, 2, 3);
	keypad(game_win, TRUE);
	
	hud_win = newwin(8, 30, 5, COLS - 40);
	box(hud_win, 0, 0);
	mvwprintw(hud_win, 3, 5, "Bomberman! \\(^_^)/");
	wrefresh(hud_win);



	/* ~Map creation~ */
	/* Grass */
	for (i = 0; i < M; i++)
		for (j = 0; j < N; j++)
			screen[i][j] = 0;

	/* Borders */
	for (i = 0; i < M; i++)
	{
		screen[i][0] = 2;
		screen[i][N - 1] = 2;
	}
	for (i = 0; i < N; i++)
	{
		screen[0][i] = 2;
		screen[M - 1][i] = 2;
	}
	
	/* Blocks */
	for (i = 2; i < M - 1; i += 2)
		for (j = 2; j < N - 1; j += 2)
			screen[i][j]=2;

	/* Player 1 */
	screen[x][y] = 1;

	draw(screen, game_win);



	/* ~The Game Loop!~ */
	while (1)
	{
		ch = wgetch(game_win);
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

		draw(screen, game_win);
	}
	

	/* ~Le End~ */
	/* Reminder: Free your memory! */
	endwin();
	return 0;
}