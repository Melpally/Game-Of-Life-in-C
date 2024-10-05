#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define HEIGHT 25
#define WIDTH 80

void dynamic_array_print(char* array, int height, int width);
int count_neighbors(char* array, int height, int width, int x, int y);
char* update_array(char* previous_array, int height, int width);
void set_raw_mode();
void reset_terminal_mode();
void clear_screen();

struct termios orig_termios;

int main(int argc, char* argv[]) {
    set_raw_mode();

    char* array = (char*)calloc(HEIGHT * WIDTH, sizeof(char));
    if (array == NULL) {
        printf("Memory allocation error\n");
        return 1;
    }

    if (argc > 1) {
        FILE* file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Failed to open file\n");
            free(array);
            return 1;
        }

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                int c = fgetc(file);
                if (c == EOF) {
                    break;
                }
                array[y * WIDTH + x] = (c == '#' ? '#' : '.');
            }
            fgetc(file);
        }
        fclose(file);
    } else {
        printf("Please provide an initial state file as an argument.\n");
        free(array);
        return 1;
    }

    int speed = 100000;
    char command = ' ';

    while (command != 'q') {
        clear_screen();
        dynamic_array_print(array, HEIGHT, WIDTH);
        char* next_array = update_array(array, HEIGHT, WIDTH);
        if (next_array == NULL) {
            printf("Memory allocation error\n");
            free(array);
            return 1;
        }
        free(array);
        array = next_array;
        usleep(speed);

        if (read(STDIN_FILENO, &command, 1) == 1) {
            if (command == '+') {
                speed = (speed > 50000) ? speed - 50000 : speed;
            } else if (command == '-') {
                speed += 50000;
            }
        }
    }

    free(array);
    reset_terminal_mode();
    return 0;
}

void dynamic_array_print(char* array, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%c", *(array + i * width + j));
        }
        printf("\n");
    }
}

int count_neighbors(char* array, int height, int width, int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) {
                continue;
            }
            int nx = (x + j + width) % width;
            int ny = (y + i + height) % height;
            if (*(array + ny * width + nx) == '#') {
                count++;
            }
        }
    }
    return count;
}

char* update_array(char* previous_array, int height, int width) {
    char* next_array = (char*)calloc(height * width, sizeof(char));
    if (next_array == NULL) {
        return NULL;
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int alive = count_neighbors(previous_array, height, width, x, y);
            char cell = *(previous_array + y * width + x);
            if (cell == '#') {
                if (alive < 2 || alive > 3) {
                    *(next_array + y * width + x) = '.';
                } else {
                    *(next_array + y * width + x) = '#';
                }
            } else {
                if (alive == 3) {
                    *(next_array + y * width + x) = '#';
                } else {
                    *(next_array + y * width + x) = '.';
                }
            }
        }
    }
    return next_array;
}

void set_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void reset_terminal_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void clear_screen() { printf("\e[1;1H\e[2J"); }
