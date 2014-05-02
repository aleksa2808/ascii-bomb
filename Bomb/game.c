#include <curses.h>
#include <stdlib.h>

extern void init_screen(int, int);
extern void draw(char**);

/* Random number generator :D */
// period = 2^96-1
static unsigned long x=123456789, y=362436069, z=521288629;
unsigned long xorshf96(void) 
{
	unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

	return z;
}

int game(void)
{
	/* ~Initialization~ */
	extern WINDOW *game_win;
	int i, j, x = 1, y = 1, ch, mx, my, fill, m = 23, n = 35, per = 50;
	
	char **screen;
	screen = (char**) malloc(m * sizeof(char*));
	for (i = 0; i < m; i++)
	{
		screen[i] = (char*) malloc(n * sizeof(char));
	}

	init_screen(m, n);



	/* ~Map creation~ */
	/* Grass */
	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++)
			screen[i][j] = 0;

	/* Borders */
	for (i = 0; i < m; i++)
	{
		screen[i][0] = 2;
		screen[i][n - 1] = 2;
	}
	for (i = 0; i < n; i++)
	{
		screen[0][i] = 2;
		screen[m - 1][i] = 2;
	}
	
	/* Blocks */
	for (i = 2; i < m - 1; i += 2)
		for (j = 2; j < n - 1; j += 2)
			screen[i][j]=2;

	/* Destructibles */
	fill = (((m - 2) / 2 + 1) * (n - 2) + ((m - 2) / 2) * ((n - 2) / 2 + 1)); // Number of empty blocks
	fill = fill * (per/100.0);
	for (i = 0; i < fill; i++)
	{
		do
		{
			my = xorshf96()%(m - 2) + 1;
			mx = xorshf96()%(n - 2) + 1;
		}
		while (screen[my][mx] != 0);
		screen[my][mx] = 3;
	}

	/* -Clearing corners- */
	/* Top left */
	screen[1][2] = 0;
	screen[1][1] = 0;
	screen[2][1] = 0;
	screen[3][1] = 0;
	/* Top right */
	screen[1][n - 4] = 0;
	screen[1][n - 3] = 0;
	screen[1][n - 2] = 0;
	screen[2][n - 2] = 0;
	/* Bottom left */
	screen[m - 3][1] = 0;
	screen[m - 2][1] = 0;
	screen[m - 2][2] = 0;
	screen[m - 2][3] = 0;
	/* Bottom right */
	screen[m - 2][n - 3] = 0;
	screen[m - 2][n - 2] = 0;
	screen[m - 3][n - 2] = 0;
	screen[m - 4][n - 2] = 0;

	/* -Spawning players- */
	/* Player 1 */
	screen[y][x] = 1;

	draw(screen);



	/* ~The Game Loop!~ */
	while (1)
	{
		ch = wgetch(game_win);
		switch (ch)
		{
		case KEY_LEFT:
		case 'a':
			if (screen[y][x - 1] == 0) {
				screen[y][x] = 0;
				x--;
				screen[y][x] = 1;
			}
			break;
		case KEY_RIGHT:
		case 'd':
			if (screen[y][x + 1] == 0) {
				screen[y][x] = 0;
				x++;
				screen[y][x] = 1;
			}
			break;
		case KEY_UP:
		case 'w':
			if (screen[y - 1][x] == 0) {
				screen[y][x] = 0;
				y--;
				screen[y][x] = 1;
			}
			break;
		case KEY_DOWN:
		case 's':
			if (screen[y + 1][x] == 0) {
				screen[y][x] = 0;
				y++;
				screen[y][x] = 1;
			}
			break;
		case 't':
			clear();
			refresh();
			return 0;
		}
		
		draw(screen);
	}



	/* Game over */
	for (i = 0; i < m; i++)
	{
		free(screen[i]);
	}
	free(screen);
	return 0;
}