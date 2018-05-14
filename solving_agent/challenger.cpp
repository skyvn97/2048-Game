#include "gameplay.h"

class SolvingAgent {
private:
    struct StateLimitException{};

    static const int MAX_DEPTH = 35;
    static const int MAX_NUM_STATE = (int)2e5;

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
    SolvingAgent(){}

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

class ChallengeAgent {
private:
    struct StateLimitException{};

    static const int INF = (int)1e9 + 7;
    static const int MAX_NUM_STATE = (int)7e5;
    static const int MIN_LIMIT_DEPTH = 5;
    static const int MAX_LIMIT_DEPTH = 55;

    static const int RANDOM_RATE_NUMERATOR = 1;
    static const int RANDOM_RATE_DENOMINATOR = 16;

    int numState, limitDepth;

    long long heuristic(StateMask state) {
        int res = 0;
        REP(i, 4) REP(j, 4) {
            int a = VALUE(state, i, j);
            REP(k, 4) if (INSIDE(i + dx[k], j + dy[k])) {
                int b = VALUE(state, i + dx[k], j + dy[k]);
                res += (a - b) * (a - b);
            }
        }
        return -res;
    }

    int getBestScore(StateMask state, int depth, int minExpect = -INF, int maxExpect = INF) {
        if (depth >= limitDepth) return 0;
        if (minExpect >= maxExpect) return minExpect;

        if (++numState > MAX_NUM_STATE) throw StateLimitException();

        int best = minExpect;
        bool haveMove = false;
        REP(dir, 4) {
            pair<StateMask, int> outcome = getStateAfterMove(state, dir);
            if (outcome.fi == state) continue;
            haveMove = true;

            int value = outcome.se + getWorstScore(outcome.fi, depth + 1, best - outcome.se, maxExpect - outcome.se);
            if (value >= maxExpect) return value;
            maximize(best, value);
        }
        return haveMove ? best : 0;
    }

    int getWorstScore(StateMask state, int depth, int minExpect = -INF, int maxExpect = INF) {
        if (depth >= limitDepth) return 0;
        if (minExpect >= maxExpect) return minExpect;

        if (++numState > MAX_NUM_STATE) throw StateLimitException();

        int worst = maxExpect;
        REP(cell, 16) if (HEXA_BIT(state, cell) == 0) FOR(value, 1, 2) {
            StateMask nextState = insertTile(state, cell, value);
            int tmp = getBestScore(nextState, depth + 1, minExpect, worst);
            if (tmp <= minExpect) return value;
            minimize(worst, tmp);
        }
        return worst;
    }

    pair<int, int> findWorstInsertion(StateMask state) {
        int worst = INF;
        long long choices = 0;
        REP(cell, 16) if (HEXA_BIT(state, cell) == 0) FOR(value, 1, 2) {
            StateMask nextState = insertTile(state, cell, value);
            int tmp = getBestScore(nextState, 0);
            int choiceID = (cell << 1) ^ (value - 1);
            if (minimize(worst, tmp)) choices = MASK(choiceID);
            else if (worst == tmp) choices |= MASK(choiceID);
        }

        int bestMove[33];
        memset(bestMove, -0x3f, sizeof bestMove);
        REP(cell, 16) FOR(value, 1, 2) {
            int id = (cell << 1) ^ (value - 1);
            if (BIT(choices, id)) {
                StateMask nextState = insertTile(state, cell, value);
                REP(dir, 4) {
                    StateMask tmp = getStateAfterMove(nextState, dir).fi;
                    if (tmp != nextState) maximize(bestMove[id], heuristic(tmp));
                }
            }
        }

        worst = INF;
        REP(cell, 16) FOR(value, 1, 2) {
            int id = (cell << 1) ^ (value - 1);
            if (BIT(choices, id)) minimize(worst, bestMove[id]);
        }
        REP(cell, 16) FOR(value, 1, 2) {
            int id = (cell << 1) ^ (value - 1);
            if (BIT(choices, id) && worst == bestMove[id]) return make_pair(id >> 1, (id & 1) + 1);
        }

        assert(false);
    }

public:
    ChallengeAgent(){}

    pair<int, int> getNextInsertion(StateMask state) {
        if (rand() % RANDOM_RATE_DENOMINATOR < RANDOM_RATE_NUMERATOR) {
            while (true) {
                int cell = rand() % 16;
                if (HEXA_BIT(state, cell) == 0) return make_pair(cell, rand() % 2 + 1);
            }
        }

        limitDepth = MIN_LIMIT_DEPTH;
        pair<int, int> result(-1, -1);
        while (true) {
            numState = 0;
            try {
                result = findWorstInsertion(state);
            } catch (StateLimitException e) {
                return result;
            }
            if (limitDepth + 2 > MAX_LIMIT_DEPTH) return result;
            limitDepth += 2;
        }
    }
};

class GameController {
private:
    struct GameResult {
        int numTurn, score, maxTile;

        GameResult(int nt = 0, int s = 0, int mt = 0) {
            numTurn = nt; score = s; maxTile = mt;
        }
    };

    static const int NUM_GAME = 100;

    SolvingAgent solver;
    ChallengeAgent challenger;

    GameResult playGame(void) {
        StateMask state = randomInsert(0);
        int score = 0;
        int numTurn = 0;

        while (!endGame(state)) {
            numTurn++;
            int mv = solver.getNextMove(state);
            pair<StateMask, int> outcome = getStateAfterMove(state, mv);
            assert(state != outcome.fi);
            score += outcome.se;
            pair<int, int> ins = challenger.getNextInsertion(outcome.fi);
            state = insertTile(outcome.fi, ins.fi, ins.se);
        }

        int maxTile = 0;
        REP(i, 16) maximize(maxTile, (int)HEXA_BIT(state, i));

        printf("%d turns, score: %d, maxTile: %d\n", numTurn, score, int(MASK(maxTile)));
        system("sleep 3s");
        return GameResult(numTurn, score, MASK(maxTile));
    }

public:
    GameController(){}

    void run(void) {
        long long totScore = 0;
        map<int, int> cntMaxTile;
        REP(love, NUM_GAME) {
            GameResult result = playGame();
            totScore += result.score;
            cntMaxTile[result.maxTile]++;
        }

        printf("Statistics:\n");
        printf("Average score: %.3lf\n", 1.0 * totScore / NUM_GAME);
        FORE(it, cntMaxTile)
            printf("%d games (%.2lf%%) with maxTile = %d\n", it->se, 100.0 * it->se / NUM_GAME, it->fi);
    }
};

int main(int argc, char* argv[]) {
    if (argv != NULL && argv[1] != NULL && strcmp(argv[1], "--practice") == 0) {
        cerr << "Practice mode" << endl;
        srand(time(NULL));
        GameController().run();
        return 0;
    }

    ChallengeAgent agent;

    while (true) {
        int board[4][4];
        REP(i, 4) REP(j, 4) {
            assert(cin >> board[i][j]);
            if (board[i][j] < 0) return 0;
        }
        pair<int, int> tmp = agent.getNextInsertion(getMaskFromBoard(board));
        cout << COL(tmp.fi) << "\n" << ROW(tmp.fi) << "\n" << MASK(tmp.se) << endl;
    }

    return 1;
}
