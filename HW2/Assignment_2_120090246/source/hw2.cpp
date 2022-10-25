#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <termios.h>
#include <fcntl.h>

#define ROW 10
#define COLUMN 50
bool finish = 0;
char state = ' ';
char direction = ' ';
pthread_mutex_t mutex;
int logs_length = 15;
struct Node
{
	int x, y;
	Node(int _x, int _y) : x(_x), y(_y){};
	Node(){};
} frog;

char map[ROW + 10][COLUMN];

// Determine a keyboard is hit or not. If yes, return 1. If not, return 0.
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

void *logs_move(void *t)
{
	int random_initial[10];
	
	srand((int)time(0));
	for (int i = 1; i < 10; i++)
	{
		random_initial[i] = rand() % 49;
	}
	/*  Move the logs  */

	while (!finish)
	{	
		pthread_mutex_lock(&mutex);
		int logs[15];
		for (int row = 1; row < 10; row++)
		{
			if (row % 2 == 0)
			{
				random_initial[row] = (random_initial[row] + 1) % 49;
			}
			else if (random_initial[row] == 0)
			{
				random_initial[row] = 48;
			}
			else
			{
				random_initial[row] -= 1;
			}

			for (int i = 0; i < 49; i++)
			{
				map[row][i] = ' ';
			}
			for (int i = 0; i < logs_length; i++)
			{
				logs[i] = (random_initial[row] + i) % 49;
				map[row][logs[i]] = '=';
			}
		}
		
		if (direction == 'r')
		{
			frog.y += 1;
		}
		else if (direction == 'l')
		{
			frog.y -= 1;
		}

		if (frog.y < 0 || frog.y > 50)
		{
			state = 'l';
			finish = 1;
			pthread_exit(NULL);
		}
		map[frog.x][frog.y] = '0';
		pthread_mutex_unlock(&mutex);
		usleep(100000);
	}
	pthread_exit(NULL);
}

void *frogs_move(void *t)
{

	while (!finish)
	{
		if (kbhit())
		{
			/*  Check keyboard hits, to change frog's position or quit the game. */
			char in = getchar();

			pthread_mutex_lock(&mutex);
			if (in == 'q' || in == 'Q')
			{
				state = 'q';
				finish = 1;
				pthread_exit(NULL);
			}
			else if (in == 'd' || in == 'D')
			{
				if (frog.x == 10)
				{
					map[frog.x][frog.y] = '|';
				}
				frog.y += 1;
			}
			else if (in == 'a' || in == 'A')
			{
				if (frog.x == 10)
				{
					map[frog.x][frog.y] = '|';
				}
				frog.y -= 1;
			}
			else if (in == 'w' || in == 'W')
			{
				if (frog.x == 10)
				{
					map[frog.x][frog.y] = '|';
				}
				frog.x -= 1;
			}
			else if (in == 's' || in == 'S' && frog.x != 10)
			{
				frog.x += 1;
			}

			if (map[frog.x][frog.y] == ' ' || frog.y < 0 || frog.x >= 49)
			{
				state = 'l';
				finish = 1;
				pthread_exit(NULL);
			}
			else if (frog.x == 0)
			{
				state = 'w';
				finish = 1;
				pthread_exit(NULL);
			}
			else if (map[frog.x][frog.y] == '=')
			{
				if (frog.x % 2 == 0)
				{
					direction = 'r';
				}
				else
				{
					direction = 'l';
				}
			}
			else direction = ' ';
			pthread_mutex_unlock(&mutex);
		}
	}
}
void *print_map(void *t){
	while (!finish){
		pthread_mutex_lock(&mutex);
		printf("\033[H\033[2J");
		for (int i = 0; i <= ROW; ++i)
		{
			puts(map[i]);
		}
		pthread_mutex_unlock(&mutex);
		usleep(30000);
	}
	printf("\033[H\033[2J");
	if (state == 'w')
	{
		printf("You win the game!!\n");
	}
	else if (state == 'l')
	{
		printf("You lose the game!!\n");
	}
	else if(state == 'q'){
		printf("You exit the game.\n");
	}
	pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
	// Initialize the river map and frog's starting position
	memset(map, 0, sizeof(map));
	int i, j;
	for (i = 1; i < ROW; ++i)
	{
		for (j = 0; j < COLUMN - 1; ++j)
			map[i][j] = ' ';
	}

	for (j = 0; j < COLUMN - 1; ++j)
		map[ROW][j] = map[0][j] = '|';

	for (j = 0; j < COLUMN - 1; ++j)
		map[0][j] = map[0][j] = '|';

	frog = Node(ROW, (COLUMN - 1) / 2);
	map[frog.x][frog.y] = '0';

	// Print the map into screen
	for (i = 0; i <= ROW; ++i)
		puts(map[i]);

	/*  Create pthreads for wood move and frog control.  */
	pthread_t pthread_logs;
	pthread_t pthread_frogs;
	pthread_t pthread_print;
	pthread_attr_t attr;

	pthread_mutex_init(&mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pthread_create(&pthread_logs, &attr, logs_move, &map);
	pthread_create(&pthread_frogs, &attr, frogs_move, &map);
	pthread_create(&pthread_print, &attr, print_map, &map);

	pthread_join(pthread_logs, NULL);
	pthread_join(pthread_frogs, NULL);
	pthread_join(pthread_print, NULL);

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);

	/*  Display the output for user: win, lose or quit.  */

	return 0;
}
