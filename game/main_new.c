// fixed not recommended warnings
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// date time
#include <time.h>
// screen coordinates
#include <ncurses.h>

// Стандартный размер поля
#define SIZE_X 80
#define SIZE_Y 25

// Работа с полем
int currentField[SIZE_Y + 2][SIZE_X + 2] = {0};
int nextField[SIZE_Y + 2][SIZE_X + 2] = {0};

// Прототипы функций
int init_game();
void randomBoard();
void inputMapFromFile(char *string);
int set_speed(int *speed);
void start_game(int speed);
int newGeneration();
int count_neigh(int row, int column);
int renewCurrent();
void copyMargin();
void print_field();

int main() {
    int speed = 1, out;
    
    // Инициализация ncurses
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    halfdelay(10);
    
    out = init_game();
    while (out % 2 != 0) out = init_game();
    
    if (out == 0) {
        set_speed(&speed);
        start_game(speed);
    }
    
    endwin();
    return 0;
}

int init_game() {
    int gameVariable, out = 0;
    
    clear();
    printw("For start game, please, choose map type:\n");
    printw("1. gameRules1.txt\n");
    printw("2. gameRules2.txt\n");
    printw("3. gameRules3.txt\n");
    printw("4. gameRules4.txt\n");
    printw("5. gameRules5.txt\n");
    printw("6. RANDOM BOARD\n");
    printw("7. Exit\n\n");
    
    printw("During the game:\n");
    printw("Up arrow - speed up\n");
    printw("Down arrow - speed down\n");
    printw("Left arrow - quit.\n\n");
    
    printw("Your choice (1-7): ");
    refresh();
    
    // Временно используем echo для ввода
    echo();
    scanw("%d", &gameVariable);
    noecho();
    
    if (gameVariable > 7 || gameVariable < 1) {
        printw("Please, enter 1-6 variable, 7 for exit\n");
        getch();
        out = 1;
    } else {
        switch (gameVariable) {
            case 1:
                inputMapFromFile("gameRules1.txt");
                break;
            case 2:
                inputMapFromFile("gameRules2.txt");
                break;
            case 3:
                inputMapFromFile("gameRules3.txt");
                break;
            case 4:
                inputMapFromFile("gameRules4.txt");
                break;
            case 5:
                inputMapFromFile("gameRules5.txt");
                break;
            case 6:
                randomBoard();
                break;
            case 7:
                out = 2;
                break;
            default:
                out = 2;
        }
    }
    return out;
}

void inputMapFromFile(char *string) {
    FILE *f = fopen(string, "r");
    if (f == NULL) {
        clear();
        printw("ERROR: Cannot open file '%s'\n", string);
        printw("Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Отладочное сообщение
    clear();
    printw("Opening file: %s\n", string);
    refresh();
    
    // Очищаем текущее поле
    for (int i = 0; i < SIZE_Y + 2; i++) {
        for (int j = 0; j < SIZE_X + 2; j++) {
            currentField[i][j] = 0;
        }
    }
    
    // Читаем файл построчно
    char line[SIZE_X + 10];  // Немного больше для безопасности
    int row = 1;
    
    while (fgets(line, sizeof(line), f) != NULL && row <= SIZE_Y) {
        // Убираем символ новой строки
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        // Обрабатываем каждый символ в строке
        for (int col = 1; col <= SIZE_X && col-1 < (int)len; col++) {
            if (line[col-1] == '*') {
                currentField[row][col] = 1;
            } else {
                currentField[row][col] = 0;
            }
        }
        row++;
    }
    
    fclose(f);
    
    // Показываем, сколько строк прочитано
    clear();
    printw("File '%s' loaded successfully.\n", string);
    printw("Rows read: %d\n", row-1);
    printw("Press any key to continue...");
    refresh();
    getch();
}

void start_game(int speed) {
    int end = 0;
    int delay = 1000 / speed;
    if (delay < 50) delay = 50;
    if (delay > 1000) delay = 1000;
    timeout(delay);
    
    while (!end) {
        int ch = getch();
        switch (ch) {
            case KEY_DOWN:
                if (speed > 1) {
                    speed--;
                    delay = 1000 / speed;
                    if (delay < 50) delay = 50;
                    timeout(delay);
                }
                break;
            case KEY_UP:
                if (speed < 20) {
                    speed++;
                    delay = 1000 / speed;
                    if (delay < 50) delay = 50;
                    timeout(delay);
                }
                break;
            case KEY_LEFT:
                end = 1;
                break;
            case ERR:
                // Нет ввода - продолжаем симуляцию
                break;
        }
        
        print_field();
        
        end += newGeneration();
        end += renewCurrent();
        copyMargin();
    }
}

void print_field() {
    clear();
    
    // Отображаем поле с границами
    for (int i = 1; i <= SIZE_Y; i++) {
        for (int j = 1; j <= SIZE_X; j++) {
            if (currentField[i][j] == 1)
                printw("*");
            else
                printw(" ");
        }
        printw("\n");
    }
    
    // Статусная строка
    printw("\nControls: UP - faster, DOWN - slower, LEFT - exit\n");
    refresh();
}

int set_speed(int *speed) {
    clear();
    printw("Enter game speed (1-20, where 1 is slowest): ");
    refresh();
    
    echo();
    scanw("%d", speed);
    noecho();
    
    if (*speed < 1) *speed = 1;
    if (*speed > 20) *speed = 20;
    
    return 0;
}

void randomBoard() {
    unsigned int seed = time(NULL);
    
    // Очищаем поле
    for (int i = 0; i < SIZE_Y + 2; i++) {
        for (int j = 0; j < SIZE_X + 2; j++) {
            currentField[i][j] = 0;
        }
    }
    
    // Заполняем случайными значениями
    for (int i = 1; i <= SIZE_Y; i++) {
        for (int j = 1; j <= SIZE_X; j++) {
            currentField[i][j] = (rand_r(&seed) % 4 == 0) ? 1 : 0;
        }
    }
    
    clear();
    printw("Random board generated.\n");
    printw("Press any key to continue...");
    refresh();
    getch();
}

int newGeneration() {
    int out = 1;
    
    // Инициализируем nextField
    for (int i = 0; i < SIZE_Y + 2; i++) {
        for (int j = 0; j < SIZE_X + 2; j++) {
            nextField[i][j] = 0;
        }
    }
    
    for (int i = 1; i <= SIZE_Y; i++) {
        for (int j = 1; j <= SIZE_X; j++) {
            int nneighb = count_neigh(i, j);
            if ((currentField[i][j] && (nneighb == 2 || nneighb == 3)) ||
                (!currentField[i][j] && nneighb == 3)) {
                out = 0;
                nextField[i][j] = 1;
            }
        }
    }
    return out;
}

int count_neigh(int row, int column) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (!(i == 0 && j == 0)) {
                count += currentField[row + i][column + j];
            }
        }
    }
    return count;
}

void copyMargin() {
    // Копируем границы для торического поля
    for (int i = 1; i <= SIZE_Y; i++) {
        currentField[i][0] = currentField[i][SIZE_X];
        currentField[i][SIZE_X + 1] = currentField[i][1];
    }
    
    for (int j = 1; j <= SIZE_X; j++) {
        currentField[0][j] = currentField[SIZE_Y][j];
        currentField[SIZE_Y + 1][j] = currentField[1][j];
    }
    
    // Углы
    currentField[0][0] = currentField[SIZE_Y][SIZE_X];
    currentField[0][SIZE_X + 1] = currentField[SIZE_Y][1];
    currentField[SIZE_Y + 1][0] = currentField[1][SIZE_X];
    currentField[SIZE_Y + 1][SIZE_X + 1] = currentField[1][1];
}

int renewCurrent() {
    int out = 1;
    for (int i = 0; i < SIZE_Y + 2; i++) {
        for (int j = 0; j < SIZE_X + 2; j++) {
            currentField[i][j] = nextField[i][j];
            if (currentField[i][j] != 0) out = 0;
        }
    }
    return out;
}
