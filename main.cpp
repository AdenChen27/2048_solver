#include "game2048.h"
#include "solver.h"
#include "optimizer.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>


namespace test
{
    const int TEST_ROUND = 25;
    const bool PRINT = true;
    const bool PRINT_EVERY_ROUND = true;
    const bool SPLIT_LINE = true;
    const int PRINT_ID_STEP = 5;

    double mean_s; // score
    double mean_oc; // opt_count
    double mean_mv; // max_val

    clock_t start_t, end_t;

    void cmd_print_split_line(int line_long);
    inline void test_init(void);
    inline void renew_data(const game2048 &game, int round, int round_i, int opt_count);
    inline void print_final_test_data(int round);
    double test(int round=10);
}


void test::cmd_print_split_line(int line_long)
{
    for (int i = 0; i < line_long; ++i)
    {
        putchar('_');
    }
    putchar('\n');
}


inline void test::test_init(void)
{
    srand(time(0));
    mean_s = 0;
    mean_oc = 0;
    mean_mv = 0;
    start_t = clock();
}


inline void test::renew_data(const game2048 &game, int round, int round_i, int opt_count)
{
    if (PRINT && PRINT_EVERY_ROUND)
    {
        (round_i % PRINT_ID_STEP == 0)
        ? printf("%3d ", round_i)
        : printf("    ");
        printf("%5d| %8d| %7d\n", opt_count, game.get_score(), 1 << game.get_max_val());
    }
    mean_s += double(game.get_score())/round;
    mean_oc += double(opt_count)/round;
    mean_mv += double(1 << game.get_max_val())/round;
}


// test_data:
// prefix: len 4
inline void test::print_final_test_data(int round)
{
    if (!PRINT)
    {
        return ;
    }
    // QueryPerformanceCounter(&end_t);
    if (PRINT_EVERY_ROUND && SPLIT_LINE)
    {
        cmd_print_split_line(30);
    }
    printf("[+] %5.1lf| %8.1lf| %7.1lf\n", mean_oc, mean_s, mean_mv);
    // double total_time = double(end_t.QuadPart - start_t.QuadPart)/time_f.QuadPart;
    // printf("MPS:%5.1lf\n", mean_oc*round/total_time);
    // printf("SPR:%5.1lf\n", total_time/round);
    // printf("T: %5.1lfs\n", total_time);
}


double test::test(int round /*=10*/)
{
    char opt_name[4][10] = {"UP", "RIGHT", "DOWN", "LEFT"};
    test_init();
    game2048 game;
    for (int round_i = 1; round_i <= round; ++round_i)
    {
        // printf("start test\n");
        game.clear_board();
        game.generate_new();
        int opt_i;
        int opt_count = 0;
        clock_t last_t;
        do
        {
            game.generate_new();
            last_t = clock();
            opt_i = solver::solve(game);
            game.opt(opt_i);

            system("clear");

            ++opt_count;
            game.log_to_cmd();
            end_t = clock();
            printf("opt:  %s\n\n", opt_name[opt_i]);
            printf("T:    %5.1lf s\n", (double)(end_t - start_t)/CLOCKS_PER_SEC);
            printf("t:    %5.1lfms\n", (double)(end_t - last_t)/CLOCKS_PER_SEC/opt_count*1000);
            printf("score:%5d\n", game.get_score());
            printf("max:  %5d\n", game.get_max_val()? 1 << game.get_max_val(): 0);

        }while (game.get_empty_num());
        // renew_data(game, round, round_i, opt_count);
        // game.log_to_cmd();
    }
    // print_final_test_data(round);
    return mean_s;
}


int main()
{
    srand(time(0));
    // while (true)
    // {
    //     double k1 = double(rand() % 100)/20;
    //     double k2 = double(rand() % 100)/20;
    //     evaluater::set_svk(k1, k2);
    //     printf("[%7.4lf %7.4lf]", k1, k2);
    //     test(TEST_ROUND);
    // }
    test::test(1);
    // printf("%7.2lf\n", test::test(test::TEST_ROUND));
    // creature c;
    // c.set_rand_k();
    // for (int i = 0; i < 5; ++i)
    // {
    //     printf("%lf ", c.k[i]);
    // }
    // putchar('\n');
    // c.eval();
    // printf("%lf %lf\n", c.score, c.max_val);
    // generation g;
    // g.run("starter/S_2", 30);
    return 0;
}

