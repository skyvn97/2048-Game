#include "gameplay.h"

int numState, numTurn;
set<StateMask> recentStates;

class Agent {
public:
    static const long long INF = (long long)1e18 + 7LL;

    static const int NUM_STATE = (int)1e5;
    static const int MIN_LIMIT_DEPTH = 9;
    static const int MAX_LIMIT_DEPTH = 30;

    int limitDepth;

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
        int bestMove = 0;

        long long choices[4];
        REP(dir, 4) choices[dir] = (getMove(state, dir).se << 2) ^ dir;
        sort(choices, choices + 4, greater<long long>());

        REP(i, 4) {
            int dir = choices[i] & 3;
            pair<StateMask, int> outcome = getMove(state, dir);

            if (outcome.fi == state) continue;

            long long worstCase = findWorstInsertion(outcome.fi, depth + 1, baseScore + outcome.se, best - (depth == 0), maxExpect);
//            if (depth == 0) printf("dir %c gives %lld\n", DIR[dir], worstCase);
            if (worstCase >= maxExpect && depth > 0) return maxExpect;
            if (best < worstCase) {
                best = worstCase;
                bestMove = MASK(dir);
            } else if (best == worstCase) bestMove |= MASK(dir);
        }

        if (bestMove == 0) return depth;
        if (depth > 0) return best;

//        printf("%d best choices\n", __builtin_popcount(bestMove));

        long long worstInsertion[4];
        memset(worstInsertion, 0x3f, sizeof worstInsertion);
        REP(dir, 4) if (BIT(bestMove, dir)) {
            StateMask newState = getMove(state, dir).fi;
            REP(cell, 16) if (HEXA_BIT(newState, cell) == 0) FOR(value, 1, 2)
                minimize(worstInsertion[dir], heuristic(insertTile(newState, cell, value)));
        }
        best = -INF;
        REP(dir, 4) if (BIT(bestMove, dir)) maximize(best, worstInsertion[dir]);
        REP(dir, 4) if (BIT(bestMove, dir) && best == worstInsertion[dir]) return dir;
        assert(false);
    }

    char getNextMove(StateMask state) {
        limitDepth = MIN_LIMIT_DEPTH;

        while (true) {
            numState = 0;
            int id = findBestMove(state, 0);
            if (limitDepth + 2 > MAX_LIMIT_DEPTH || numState >= NUM_STATE) {
                return DIR[id];
            }
            limitDepth += 2;
        }
    }
};

StateMask getMask(int board[4][4]) {
    StateMask res = 0;
    REP(i, 4) REP(j, 4) REP(k, 20) if (MASK(k) == board[i][j])
        res |= (1ULL * k) << (4 * POS(i, j));
    return res;
}

Agent agent;
int main(int argc, char* argv[]) {
    while (true) {
        numTurn++;
        int board[4][4];
        REP(i, 4) REP(j, 4) {
            assert(cin >> board[i][j]);
            if (board[i][j] < 0) {
                fprintf(stderr, "%d turns, %d states visited\n", numTurn, (int)recentStates.size());
                exit(EXIT_SUCCESS);
            }
        }
        cout << getDir(agent.getNextMove(getMask(board))) << endl;
    }

    return 1;
}
