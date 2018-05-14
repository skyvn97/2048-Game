#include "gameplay.h"

class Agent {
private:
    struct StateLimitException{};

    static const int MAX_DEPTH = 35;
    static const int MAX_NUM_STATE = (int)3e5;

    int limitDepth, numState;

    double getBestScore(StateMask state, int depth) {
        if (depth >= limitDepth) return 0;
        if (++numState > MAX_NUM_STATE) throw StateLimitException();

        double best = 0;
        REP(dir, 4) {
            pair<StateMask, int> outcome = getStateAfterMove(state, dir);
            if (outcome.fi == state) continue;

            maximize(best, outcome.se + getAverageScore(outcome.fi, depth + 1));
        }
        return best;
    }

    double getAverageScore(StateMask state, int depth) {
        if (depth >= limitDepth) return 0;
        numState++;

        double sum = 0;
        int numMove = 0;

        REP(cell, 16) if (HEXA_BIT(state, cell) == 0) FOR(value, 1, 2) {
            StateMask nextState = insertTile(state, cell, value);
            sum += getBestScore(nextState, depth + 1);
            numMove++;
        }
        return sum / numMove;
    }

public:
    Agent(){}

    int getNextMove(StateMask state) {
        limitDepth = 1;
        int curBestDir = -1;
        while (true) {
            numState = 0;
            int bestDir = -1;

            try {
                double bestScore = -INFINITY;

                REP(dir, 4) {
                    pair<StateMask, int> outcome = getStateAfterMove(state, dir);
                    if (outcome.fi == state) continue;

                    if (maximize(bestScore, outcome.se + getAverageScore(outcome.fi, 1)))
                        bestDir = dir;
                }
            } catch (StateLimitException e) {
                return curBestDir;
            }

            curBestDir = bestDir;
            if (limitDepth + 2 > MAX_DEPTH) return curBestDir;
            limitDepth += 2;
        }
    }
};

class Tester {
private:
    struct GameResult {
        int numTurn, score, maxTile, secondMaxTile;

        GameResult(int nt = 0, int s = 0, int mt = 0, int smt = 0) {
            numTurn = nt; score = s; maxTile = mt; secondMaxTile = smt;
        }
    };

    static const int NUM_GAME = 200;

    GameResult playGame(void) {
        Agent agent;

        StateMask state = randomInsert(0);
        int score = 0;
        int numTurn = 0;
        while (!endGame(state)) {
            numTurn++;
            int mv = agent.getNextMove(state);
            pair<StateMask, int> outcome = getStateAfterMove(state, mv);
            assert(outcome.fi != state);
            score += outcome.se;
            state = randomInsert(outcome.fi);
        }

        int maxTile = 0, secondMaxTile = 0;
        REP(i, 16) {
            int value = MASK(HEXA_BIT(state, i));
            if (value > maxTile) {
                secondMaxTile = maxTile;
                maxTile = value;
            } else maximize(secondMaxTile, value);
        }
        printf("%d turns, score: %d, maxTile: %d, secondMaxTile: %d\n", numTurn, score, maxTile, secondMaxTile);
        system("sleep 45s");
        return GameResult(numTurn, score, maxTile, secondMaxTile);
    }

public:
    Tester(){}

    void test(void) {
        long long totScore = 0;
        map<int, int> cntMaxTile;
        REP(love, NUM_GAME) {
            GameResult result = playGame();
            totScore += result.score;
            cntMaxTile[result.maxTile]++;
            if ((love + 1) % 10 == 0)
                cerr << love + 1 << " games completed." << endl;
        }

        printf("Statistics:\n");
        printf("Average score: %.3lf\n", 1.0 * totScore / NUM_GAME);
        FORE(it, cntMaxTile)
            printf("%d games (%.2lf%%) with maxTile = %d\n", it->se, 100.0 * it->se / NUM_GAME, it->fi);
    }
};

StateMask getMask(int board[4][4]) {
    StateMask res = 0;
    REP(i, 4) REP(j, 4) REP(k, 20) if ((int)MASK(k) == board[i][j])
        res |= (1ULL * k) << (4 * POS(i, j));
    return res;
}

int main(int argc, char* argv[]) {
    if (argv != NULL && argv[1] != NULL && strcmp(argv[1], "--practice") == 0) {
        cerr << "Practice mode:" << endl;
        srand(time(NULL));
        Tester().test();
        return 0;
    }

    Agent agent;
    while (true) {
        int board[4][4];
        REP(i, 4) REP(j, 4) {
            assert(cin >> board[i][j]);
            if (board[i][j] < 0) {
                exit(EXIT_SUCCESS);
            }
        }
        cout << agent.getNextMove(getMask(board)) << endl;
    }

    return 1;
}
