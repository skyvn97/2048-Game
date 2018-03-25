#include "gameplay.h"

int numState;

class Agent {
public:
    static const long long INF = (long long)1e18 + 7LL;

    static const int NUM_STATE = (int)1e5;
    static const int MIN_LIMIT_DEPTH = 9;
    static const int MAX_LIMIT_DEPTH = 30;

    int limitDepth;

    long long heuristic(StateMask state) {
        int res = 0;
        REP(i, 4) REP(j, 4) if (VALUE(state, i, j) != 0) {
            if (i + 1 < 4 && VALUE(state, i + 1, j) != 0)
                res += VALUE(state, i, j) != VALUE(state, i + 1, j);
            if (j + 1 < 4 && VALUE(state, i, j + 1) != 0)
                res += VALUE(state, i, j) != VALUE(state, i, j + 1);
        }
        return res;
    }

    long long findWorstInsertion(StateMask state, int depth, int baseScore,
                                 long long minExpect = -INF, long long maxExpect = INF) {
        if (depth >= limitDepth) return baseScore;
        if (minExpect >= maxExpect) return minExpect;
        numState++;

        long long worst = maxExpect;

        REP(cell, 16) if (HEXA_BIT(state, cell) == 0) FOR(value, 1, 2) {
            StateMask newState = insertTile(state, cell, value);
            long long bestCase = findBestMove(newState, depth + 1, baseScore, minExpect, worst);
            if (bestCase <= minExpect) return minExpect;
            minimize(worst, bestCase);
        }

        return worst < INF ? worst : 0;
    }

    long long findBestMove(StateMask state, int depth, int baseScore = 0,
                           long long minExpect = -INF, long long maxExpect = INF) {
        if (depth >= limitDepth) return baseScore;
        if (minExpect >= maxExpect) return maxExpect;
        numState++;

        long long best = minExpect;
        int bestMove = -1;

        long long choices[4];
        REP(dir, 4) choices[dir] = (getMove(state, dir).se << 2) ^ dir;
        sort(choices, choices + 4, greater<long long>());

        REP(i, 4) {
            int dir = choices[i] & 3;
            pair<StateMask, int> outcome = getMove(state, dir);

            if (outcome.fi == state) continue;

            long long worstCase = findWorstInsertion(outcome.fi, depth + 1, baseScore + outcome.se, best, maxExpect);
            if (worstCase >= maxExpect && depth > 0) return maxExpect;
            if (best < worstCase) {
                best = worstCase;
                bestMove = dir;
            }
        }

        if (bestMove < 0) return depth;
        else return depth == 0 ? bestMove : best;
    }

    char getNextMove(StateMask state) {
        limitDepth = MIN_LIMIT_DEPTH;

        while (true) {
            numState = 0;
            int id = findBestMove(state, 0);
            if (limitDepth + 2 > MAX_LIMIT_DEPTH || numState >= NUM_STATE) {
                printf("depth %d ", limitDepth);
                return DIR[id];
            }
            limitDepth += 2;
        }
    }
};

const int TRICKY_STATE[4][4] = {{4, 4, 256, 2}, {32, 4, 8, 32}, {16, 512, 128, 2}, {4, 2048, 4, 16}};
StateMask getMask(const int board[4][4]) {
    StateMask res = 0;
    REP(i, 4) REP(j, 4) REP(k, 20) if (MASK(k) == board[i][j])
        res |= (1ULL * k) << (4 * POS(i, j));
    return res;
}

Agent agent;
int main(int argc, char* argv[]) {
    srand(220797);

    if (argv != NULL && argv[1] != NULL) {
        string number(argv[1]);
        StateMask state = 0;
        FORE(it, number) state = state * 10 + *it - '0';
        char res = agent.getNextMove(state);
        fprintf(stderr, "Num states: %d\n", numState);
        return getDir(res);
    }

//    GameController<Agent> controller(&agent);
//    controller.playGame();
    return 0;
}
