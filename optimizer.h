#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "game2048.h"
#include "solver.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <algorithm>


const int CREATURE_NUM = 50;
const int ROUND_PER_TEST = 3;
const double MUTATE_RATE = 0.25;
const double MUTATE_MAX_VAL = 0.5;
const double NEW_CREATURE_RATE = 0.72;
    // rate of killing last 10% creatures and generating a new one


int randint(int a, int b)
{
    // [a, b]
    return (rand() % (b - a + 1)) + a;
}

double randf(void)
{
    return double(rand())/RAND_MAX;
}

double randf(double a, double b)
{
    return randf()*(b - a) + a;
}


class creature
{
public:
    double param[5];
    int test_round;
    double tot_score;
    double tot_max_val;

    void reset_test_data(void);
    creature();
    void operator=(const creature &c);

    void set_rand_k(void);
    void eval(int round=ROUND_PER_TEST);

    void mutate(void);
};

class generation
{
public:
    creature creatures[CREATURE_NUM];

    void rand_init(void);
    int kill(void);
    void next(void);

    void load_from_file(const char *filename, int file_creature_num=CREATURE_NUM);
    void log_to_cmd(void) const;
    void log_to_file(const char *filename) const;

    void run(const char *filename="\0", int file_creature_num=CREATURE_NUM);
};


void creature::reset_test_data(void)
{
    test_round = 0;
    tot_score = 0;
    tot_max_val = 0;
}


creature::creature()
{
    reset_test_data();
}


void creature::operator=(const creature &c)
{
    test_round = c.test_round;
    tot_score = c.tot_score;
    tot_max_val = c.tot_max_val;
    for (int i = 0; i < 5; ++i)
    {
        param[i] = c.param[i];
    }
}


bool cmp(const creature &c1, const creature &c2)
{
    return (c1.tot_score + c1.tot_max_val)/c1.test_round + c1.test_round*5 > 
        (c2.tot_score + c2.tot_max_val)/c2.test_round + c2.test_round*5;
}


void creature::set_rand_k(void)
{
    reset_test_data();

    param[0] = randf(-10, 50);
    param[1] = randf(-10, 35);
    param[2] = randf(-10, 35);

    param[3] = randf(-3, 3);
    param[4] = randf(-3, 3);
}

void creature::eval(int round /*=5*/)
{
    srand(time(0));
    evaluater::set_weight(param[0], param[1], param[2]);
    evaluater::set_svk(param[3], param[4]);
    game2048 game;
    for (int round_i = 1; round_i <= round; ++round_i)
    {
        game.clear_board();
        game.generate_new();
        int opt_i;
        do
        {
            game.generate_new();
            do
            {
                opt_i = solver::solve(game);
            }while (game.opt(opt_i) == -1 && !game.is_dead());
        }while (game.get_empty_num());

        tot_score += game.get_score();
        tot_max_val += 1 << game.get_max_val();
    }
    test_round += ROUND_PER_TEST;
}


void creature::mutate(void)
{
    param[randint(0, 4)] += randf(-MUTATE_MAX_VAL, MUTATE_MAX_VAL);
}


creature crossover(const creature &c1, const creature &c2)
{
    creature c;
    int crossover_point = randint(0, 4);
    // [0..cp)[cp..4]
    for (int i = 0; i < crossover_point; ++i)
    {
        c.param[i] = c1.param[i];
    }
    for (int i = crossover_point; i < 5; ++i)
    {
        c.param[i] = c2.param[i];
    }
    return c;
}


void generation::rand_init(void)
{
    for (int i = 0; i < CREATURE_NUM; ++i)
    {
        creatures[i].set_rand_k();
        creatures[i].eval();
    }
}


int generation::kill(void)
{
    int remain_num = 0;
    for (int i = 0; i < CREATURE_NUM; ++i)
    {
        if (randint(0, CREATURE_NUM - 1) >= i)
        {
            // alive
            std::swap(creatures[remain_num], creatures[i]);
            ++remain_num;
        }
    }
    return remain_num;
}


void generation::next(void)
{
    int remain_num = kill();
    int x, y;
    for (int i = remain_num; i < CREATURE_NUM; ++i)
    {
        x = randint(0, remain_num - 1);
        y = randint(0, CREATURE_NUM - 1);
        creatures[i] = crossover(creatures[x], creatures[y]);
        if (randf() <= MUTATE_RATE)
        {
            creatures[i].mutate();
        }
        creatures[i].reset_test_data();
    }
    if (NEW_CREATURE_RATE)
    {
        for (int i = CREATURE_NUM*0.9 - 1; i < CREATURE_NUM; ++i)
        {
            creatures[i].set_rand_k();
        }
    }
    for (int i = 0; i < CREATURE_NUM; ++i)
    {
        if (creatures[i].test_round < 27)
        {
            creatures[i].eval();
        }
    }
}


void generation::load_from_file(const char *filename, int file_creature_num /*=CREATURE_NUM*/)
{
    std::ifstream file;
    file.open(filename, std::ios::in);
    for (int i = 0; i < file_creature_num; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            file >> creatures[i].param[j];
        }
        file >> creatures[i].test_round >> creatures[i].tot_score >> creatures[i].tot_max_val;
        if (!creatures[i].test_round)
        {
            creatures[i].eval();
        }
    }
    for (int i = file_creature_num; i < CREATURE_NUM; ++i)
    {
        creatures[i].set_rand_k();
        creatures[i].eval();
    }
    file.close();
}


void generation::log_to_cmd(void) const
{
    for (int i = 0; i < CREATURE_NUM; ++i)
    {
        printf("%10.3lf %10.3lf (%3d) ", creatures[i].tot_score/creatures[i].test_round, 
            creatures[i].tot_max_val/creatures[i].test_round, 
            creatures[i].test_round);
        printf("[");
        for (int j = 0; j < 5; ++j)
        {
            printf("%7.2lf ", creatures[i].param[j]);
        }
        printf("]\n");
    }
}


void generation::log_to_file(const char *filename) const
{
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::trunc);
    for (int i = 0; i < CREATURE_NUM; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            file << creatures[i].param[j] << " ";
        }
        file << creatures[i].test_round << " ";
        file << creatures[i].tot_score << " ";
        file << creatures[i].tot_max_val << " ";
        file << "\n";
    }
    file.close();
}


void generation::run(const char *load_filename /*="\0"*/, int file_creature_num /*=CREATURE_NUM*/)
{
    srand(time(0));
    clock_t last_t = clock();
    if (load_filename[0] == '\0')
    {
        rand_init();
    }
    else
    {
        load_from_file(load_filename, file_creature_num);
    }
    char filename[40];
    for (int i = 0; ; ++i)
    {
        std::sort(creatures, creatures + CREATURE_NUM, cmp);
        printf("G%3d(%5.2lfs):\n", i, double(clock() - last_t)/CLOCKS_PER_SEC);
        log_to_cmd();
        sprintf(filename, "log/G%d", i);
        log_to_file(filename);
        last_t = clock();
        next();
    }
}



#endif
