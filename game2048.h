#ifndef GAME2048_H
#define GAME2048_H

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>


const int DEFAULT_SIZE = 4;

// cmd output
const int OUTPUT_INT_WIDE = 5;

// cmd game
const bool GAME_CLS = true;


const bool LOG_DECODE = true;
const bool LOG_RAW = false;


class game2048
{
public:
    void init(int _size);
    game2048();
    game2048(const game2048 &game);
    ~game2048();

    inline int get_size(void) const;
    inline int get_score(void) const;
    inline int get(int i, int j) const;
    inline int** get_board(void) const;

    int get_max_val(void) const;
    int get_empty_num(void) const;

    int opt_u(void);
    int opt_d(void);
    int opt_l(void);
    int opt_r(void);
    inline int opt(int opt_i);

    inline void set(int i, int j, int v);

    void clear_board(void);
    void generate_new(void);

    bool is_dead(void) const;
    inline bool in_board(int i, int j) const;

    void load_from_file(const char *filename);

    void log_to_file(const char *filename) const;
    void log_to_cmd(bool log_type=LOG_DECODE) const;

    void cmd_game(void);

private:
    int size;
    int score;
    int** board;

    int merge(int a, int b, int x, int y);
};


void game2048::init(int _size)
{
    size = _size;
    score = 0;
    board = new int* [size];
    for (int i = 0; i < size; ++i)
    {
        board[i] = new int [size];
        for (int j = 0; j < size; ++j)
        {
            board[i][j] = 0;
        }
    }
}

game2048::game2048()
{
    init(DEFAULT_SIZE);
}

game2048::game2048(const game2048 &game)
{
    size = game.size;
    score = game.score;
    init(size);
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            board[i][j] = game.get(i, j);
        }
    }
}

game2048::~game2048()
{
    for (int i = 0; i < size; ++i)
    {
        delete []board[i];
    }
    delete []board;
}


inline int game2048::get_size(void) const
{
    return size;
}

inline int game2048::get_score(void) const
{
    return score;
}

inline int game2048::get(int i, int j) const
{
    return board[i][j];
}

inline int** game2048::get_board(void) const 
{
    return board;
}


int game2048::get_max_val(void) const
{
    int max_val = 0;
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            if (board[i][j] > max_val)
            {
                max_val = board[i][j];
            }
        }
    }
    return max_val;
}

int game2048::get_empty_num(void) const 
{
    int empty_count = 0;
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            if (!board[i][j])
            {
                ++empty_count;
            }
        }
    }
    return empty_count;
}


int game2048::merge(int a, int b, int x, int y)
{
    // (a, b) -> (x, y)
    // return merge_score / -1 for fail / 0 for moved to empty
    if (!board[x][y])
    {
        board[x][y] = board[a][b];
        board[a][b] = 0;
        return 0;
    }
    else if (board[x][y] == board[a][b])
    {
        ++board[x][y];
        board[a][b] = 0;
        return 1 << board[x][y];
    }
    return -1;
}


// [...][end][...][j]
// [0...end) -> done moving
// [end] to be merged with [j]
int game2048::opt_u(void)
{
    int score_add = 0;
    int merge_ret;
    bool opt_valid = false; // valid when a block is moved
    for (register int i = 0; i < size; ++i)
    {
        register int end = 0;
        for (register int j = 1; j < size; ++j)
        {
            if (!board[j][i])
            {
                continue;
            }
            merge_ret = merge(j, i, end, i);
            if (merge_ret)
            {
                ++end;
            }
            if (merge_ret == -1)
            {
                board[end][i] = board[j][i];
                if (end != j)
                {
                    board[j][i] = 0;
                    opt_valid = true;
                }
            }
            else
            {
                opt_valid = true;
                score_add += merge_ret;
            }
        }
    }
    if (opt_valid)
    {
        score += score_add;
        return score_add;
    }
    return -1;
}

int game2048::opt_d(void)
{
    int score_add = 0;
    int merge_ret;
    bool opt_valid = false; // valid when a block is moved
    for (register int i = 0; i < size; ++i)
    {
        register int end = size - 1;
        for (register int j = size - 2; j >= 0; --j)
        {
            if (!board[j][i])
            {
                continue;
            }
            merge_ret = merge(j, i, end, i);
            if (merge_ret)
            {
                --end;
            }
            if (merge_ret == -1)
            {
                board[end][i] = board[j][i];
                if (end != j)
                {
                    board[j][i] = 0;
                    opt_valid = true;
                }
            }
            else
            {
                opt_valid = true;
                score_add += merge_ret;
            }
        }
    }
    if (opt_valid)
    {
        score += score_add;
        return score_add;
    }
    return -1;
}

int game2048::opt_l(void)
{
    // printf("\nIN opt_L\n");
    int score_add = 0;
    int merge_ret;
    bool opt_valid = false; // valid when a block is moved
    for (register int i = 0; i < size; ++i)
    {
        register int end = 0;
        for (register int j = 1; j < size; ++j)
        {
            if (!board[i][j])
            {
                continue;
            }
            merge_ret = merge(i, j, i, end);
            // printf("[(%d,%d)->(%d,%d), %d]", i, j, i, end, merge_ret);
            if (merge_ret)
            {
                ++end;
            }
            if (merge_ret == -1)
            {
                board[i][end] = board[i][j];
                if (end != j)
                {
                    board[i][j] = 0;
                    opt_valid = true;
                }
            }
            else
            {
                opt_valid = true;
                score_add += merge_ret;
            }
        }
    }
        // printf("OUT%d\n", score_add);
    if (opt_valid)
    {
        score += score_add;
        return score_add;
    }
    // printf("OUT with -1(%d)\n", score_add);
    return -1;
}

int game2048::opt_r(void)
{
    int score_add = 0;
    int merge_ret;
    bool opt_valid = false; // valid when a block is moved
    for (register int i = 0; i < size; ++i)
    {
        register int end = size - 1;
        for (register int j = size - 2; j >= 0; --j)
        {
            if (!board[i][j])
            {
                continue;
            }
            merge_ret = merge(i, j, i, end);
            if (merge_ret)
            {
                --end;
            }
            if (merge_ret == -1)
            {
                board[i][end] = board[i][j];
                if (end != j)
                {
                    board[i][j] = 0;
                    opt_valid = true;
                }
            }
            else
            {
                opt_valid = true;
                score_add += merge_ret;
            }
        }
    }
    if (opt_valid)
    {
        score += score_add;
        return score_add;
    }
    return -1;
}

inline int game2048::opt(int opt_i)
{
    if (opt_i == 0 || opt_i == 72 || opt_i == 119 || opt_i == 49)
    {
        return opt_u();
    }
    else if (opt_i == 1 || opt_i == 77 || opt_i == 100 || opt_i == 50)
    {
        return opt_r();
    }
    else if (opt_i == 2 || opt_i == 80 || opt_i == 115 || opt_i == 51)
    {
        return opt_d();
    }
    else if (opt_i == 3 || opt_i == 75 || opt_i == 97 || opt_i == 52)
    {
        return opt_l();
    }
    return -1;
}


inline void game2048::set(int i, int j, int v)
{
    board[i][j] = v;
}


void game2048::clear_board(void)
{
    score = 0;
    for (register int i = 0; i < size; ++i)
    {
        for (register int j = 0; j < size; ++j)
        {
            board[i][j] = 0;
        }
    }
}

void game2048::generate_new(void)
{
    int new_x;
    int new_y;
    do
    {
        new_x = rand() % size;
        new_y = rand() % size;
    }while(board[new_x][new_y]);
    if (rand() % 10)
    {
        board[new_x][new_y] = 1;
    }
    else
    {
        board[new_x][new_y] = 2;
    }
}


bool game2048::is_dead(void) const
{
    for (register int i = 0; i < size; ++i)
    {
        for (register int j = 0; j < size; ++j)
        {
            if (!board[i][j] || 
                (
                    i != size - 1 && i != size - 1 && 
                    (board[i][j] == board[i][j + 1] || board[i][j] == board[i + 1][j])
                )
            )
            {
                return false;
            }
        }
    }
    return true;
}

inline bool game2048::in_board(int i, int j) const
{
    return i >= 0 && i < size && j >= 0 && j < size;
}


void game2048::load_from_file(const char *filename)
{
    std::ifstream file;
    file.open(filename, std::ios::in);
    file >> size;
    file >> score;
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            file >> board[i][j];
        }
    }
    file.close();
}

void game2048::log_to_file(const char *filename) const
{
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::trunc);
    file << size << " " << score << "\n";
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            file << board[i][j] << " ";
        }
        file << "\n";
    }
    file.close();
}

void game2048::log_to_cmd(bool log_type /* =LOG_DECODE */) const
{
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            if (log_type == LOG_DECODE)
            {
                if (board[i][j] <= 4)
                    printf("\x1b[37m%5d\x1b[37m", board[i][j]? 1 << board[i][j]: 0);
                else if (board[i][j] <= 8)
                    printf("\x1b[36m%5d\x1b[37m", 1 << board[i][j]);
                else if (board[i][j] == 9)
                    printf("\x1b[35m%5d\x1b[37m", 1 << board[i][j]);
                else if (board[i][j] == 9)
                    printf("\x1b[34m%5d\x1b[37m", 1 << board[i][j]);
                else
                    printf("\x1b[31m%5d\x1b[37m", 1 << board[i][j]);
            }
            else
            {
                printf("%5d", board[i][j]);
            }
        }
        putchar('\n');
    }
    putchar('\n');
}

void game2048::cmd_game(void)
{
    clear_board();
    generate_new();
    char key_down;
    do
    {
        if (GAME_CLS)
        {
            system("clear");
        }
        generate_new();
        log_to_cmd(LOG_DECODE);
        printf("Score: %5d\n", score);
        do
        {
            key_down = getchar();
            if (key_down == 27 || key_down == 26)
            {
                return;
            }
        }while (opt(key_down) == -1 && !is_dead());
    }while(get_empty_num());
    printf("Good Game.\nFinal Score:%d", score);
}


#endif


