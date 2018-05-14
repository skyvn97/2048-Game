#include "gameplay.h"

class Agent {
private:
    struct StateLimitException{};

public:
    static const long long INF = (long long)1e18 + 7LL;

    static const int NUM_STATE = (int)7e5;
    static const int MIN_LIMIT_DEPTH = 5;
    static const int MAX_LIMIT_DEPTH = 51;

    int limitDepth, numState;

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

    long long findWorstInsertion(StateMask state, int depth, int baseScore,
                                 long long minExpect = -INF, long long maxExpect = INF) {
        if (depth >= limitDepth) return baseScore;
        if (minExpect >= maxExpect) return minExpect;
        if (++numState > NUM_STATE) throw StateLimitException();

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
        if (++numState > NUM_STATE) throw StateLimitException();

        long long best = minExpect;
        int bestMove = 0;

        long long choices[4];
        REP(dir, 4) choices[dir] = (getStateAfterMove(state, dir).se << 2) ^ dir;
        sort(choices, choices + 4, greater<long long>());

        REP(i, 4) {
            int dir = choices[i] & 3;
            pair<StateMask, int> outcome = getStateAfterMove(state, dir);

            if (outcome.fi == state) continue;

            long long worstCase = findWorstInsertion(outcome.fi, depth + 1, baseScore + outcome.se, best - (depth == 0), maxExpect);
            if (worstCase >= maxExpect && depth > 0) return maxExpect;
            if (best < worstCase) {
                best = worstCase;
                bestMove = MASK(dir);
            } else if (best == worstCase) bestMove |= MASK(dir);
        }

        if (bestMove == 0) return depth;
        if (depth > 0) return best;

        long long worstInsertion[4];
        memset(worstInsertion, 0x3f, sizeof worstInsertion);
        REP(dir, 4) if (BIT(bestMove, dir)) {
            StateMask newState = getStateAfterMove(state, dir).fi;
            REP(cell, 16) if (HEXA_BIT(newState, cell) == 0) FOR(value, 1, 2)
                minimize(worstInsertion[dir], heuristic(insertTile(newState, cell, value)));
        }
        best = -INF;
        REP(dir, 4) if (BIT(bestMove, dir)) maximize(best, worstInsertion[dir]);
        REP(dir, 4) if (BIT(bestMove, dir) && best == worstInsertion[dir]) return dir;
        assert(false);
    }

    int getNextMove(StateMask state) {
        limitDepth = MIN_LIMIT_DEPTH;
        int curBestMove = -1;

        while (true) {
            numState = 0;
            try {
                curBestMove = findBestMove(state, 0);
            } catch (StateLimitException e) {
                return curBestMove;
            }

            if (limitDepth + 2 > MAX_LIMIT_DEPTH) return curBestMove;
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

    static const int NUM_GAME = 10;

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
        system("sleep 120s");
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
        srand(time(NULL));
        Tester().test();
        return 0;
    }

    Agent agent;
    while (true) {
        int board[4][4];
        REP(i, 4) REP(j, 4) {
            assert(cin >> board[i][j]);
            if (board[i][j] < 0) exit(EXIT_SUCCESS);
        }
        cout << agent.getNextMove(getMask(board)) << endl;
    }

    return 1;
}
