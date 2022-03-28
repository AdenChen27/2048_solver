#ifndef SLOVER_H
#define SLOVER_H

#include "game2048.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>


template <typename T>
inline void renew_min(T &a, const T &b)
{
    if (b < a)
    {
        a = b;
    }
}

template <typename T>
inline void renew_max(T &a, const T &b)
{
    if (b > a)
    {
        a = b;
    }
}

inline double get_array_max(double *start_p, double *end_p)
{
    // [start_p, end_p)
    double max_val = *start_p;
    for (double *p = start_p + 1; p < end_p; ++p)
    {
        renew_max(max_val, *p);
    }
    return max_val;
}


inline void clear_array(double *start_p, double *end_p)
{
    for (double *p = start_p + 1; p < end_p; ++p)
    {
        *p = 0;
    }
}


namespace evaluater
{
    // evaluation config
    // double EMPTY_WEIGHT = 20.9081;// 6.89489;
    // double MAXVAL_WEIGHT = 16.6578;// -5.64058;
    // double SMOOTH_WEIGHT = 15.1376;// 0.424055;

    // double SVK1 = 0.945188;// 0.896329;
    // double SVK2 = -0.488449;// 1.0619;

    // 2: 
    double EMPTY_WEIGHT = 20.3934;
    double MAXVAL_WEIGHT = 16.4807;
    double SMOOTH_WEIGHT = 15.2707;

    double SVK1 = 0.928479;
    double SVK2 = -0.0285195;
    double smooth_val[5];

    const int x_step[4] = {0, 0, 1, -1};
    const int y_step[4] = {1, -1, 0, 0};

    void set_weight(double w1, double w2, double w3)
    {
        EMPTY_WEIGHT = w1;
        MAXVAL_WEIGHT = w2;
        SMOOTH_WEIGHT = w3;
    }

    void set_svk(double svk1, double svk2)
    {
        SVK1 = svk1;
        SVK2 = svk2;
    }


    double get_smooth_val(const game2048 &node)
    {
        clear_array(smooth_val, smooth_val + 4);
        register int x, y;
        for (register int i = 0; i < node.get_size(); ++i)
        {
            for (register int j = 0; j < node.get_size(); ++j)
            {
                for (register int k = 0; k < 4; ++k)
                {
                    x = i + x_step[k];
                    y = j + y_step[k];
                    if (node.in_board(x, y) && node.get(i, j) > node.get(x, y))
                    {   
                        smooth_val[k] -= node.get(x, y)
                        ? (node.get(i, j) - node.get(x, y))
                        : SVK1;
                    }
                    else if (node.in_board(x, y) && node.get(i, j) + 1 == node.get(x, y))
                    {
                        smooth_val[k] += SVK2;
                    }
                }
            }
        }
        return get_array_max(smooth_val, smooth_val + 4);
    }

    double eval(const game2048 &node)
    {
        int empty_count = node.get_empty_num();
        int max_val = node.get_max_val();
        double smooth_val = get_smooth_val(node);
        return empty_count*EMPTY_WEIGHT + 
            max_val*MAXVAL_WEIGHT + 
            smooth_val*SMOOTH_WEIGHT;
    }
}


namespace solver
{
    // search algorithm
    const bool PLAYER_SIDE = true;
    const bool PC_SIDE = false;
    const int SEARCH_DEPTH = 4;


    const double DOUBLE_INF = 1e10;

    double minimax(const game2048 &node, int depth, bool side, double alpha, double beta)
    {
        // player -> max
        // pc -> min
        if (node.is_dead() || !depth)
        {
            return evaluater::eval(node);
        }
        if (side == PLAYER_SIDE)
        {
            for (int opt_i = 0; opt_i < 4; ++opt_i)
            {
                game2048 child(node);
                if (child.opt(opt_i) != -1)
                {
                    renew_max(alpha, minimax(child, depth - 1, PC_SIDE, alpha, beta));
                    if (beta <= alpha)
                    {
                        return alpha;
                    }
                }
            }
            return alpha;
        }
        else
        {
            for (int i = 0; i < node.get_size(); ++i)
                for (int j = 0; j < node.get_size(); ++j)
                    if (!node.get(i, j))
                    {
                        game2048 child(node);
                        child.set(i, j, 2);
                        double _beta = 0.9*minimax(child, depth - 1, PLAYER_SIDE, alpha, beta);
                        child.set(i, j, 4);
                        _beta += 0.1*minimax(child, depth - 1, PLAYER_SIDE, alpha, beta);
                        renew_min(beta, _beta);
                        if (beta <= alpha)
                        {
                            return beta;
                        }
                    }
            return beta;
        }
    }

    int solve(const game2048 &game)
    {
        double eval[4];
        for (int opt_i = 0; opt_i < 4; ++opt_i)
        {
            game2048 node(game);
            eval[opt_i] = (node.opt(opt_i) == -1)
            ? -DOUBLE_INF*2
            : minimax(node, SEARCH_DEPTH, PC_SIDE, -DOUBLE_INF, DOUBLE_INF);
        }
        int max_eval_i = 0;
        for (int i = 1; i < 4; ++i)
        {
            if (eval[i] > eval[max_eval_i])
            {
                max_eval_i = i;
            }
        }
        return max_eval_i;
    }
}



#endif

