#include<bits/stdc++.h>
#define FOR(i, a, b) for (int i = (a), _b = (b); i <= _b; i++)
#define FORD(i, b, a) for (int i = (b), _a = (a); i >= _a; i--)
#define REP(i, n) for (int i = 0, _n = (n); i < _n; i++)
#define FORE(i, v) for (__typeof((v).begin()) i = (v).begin(); i != (v).end(); i++)
#define ALL(v) (v).begin(), (v).end()
#define fi   first
#define se   second
#define MASK(i) (1ULL << (i))
#define BIT(x, i) (((x) >> (i)) & 1)
#define div   ___div
#define next   ___next
#define prev   ___prev
#define left   ___left
#define right   ___right
#define distance   ___distance
#define __builtin_popcount __builtin_popcountll
using namespace std;
template<class X, class Y>
    void minimize(X &x, const Y &y) {
        if (x > y) x = y;
    }
template<class X, class Y>
    void maximize(X &x, const Y &y) {
        if (x < y) x = y;
    }
template<class T>
    T Abs(const T &x) {
        return (x < 0 ? -x : x);
    }

/* Author: Van Hanh Pham */

/** END OF TEMPLATE - ACTUAL SOLUTION COMES HERE **/

const int dx[] = {0, 1, 0, -1};
const int dy[] = {1, 0, -1, 0};

const char DIR[] = "<^>v";
int getDir(char c) {
    REP(i, 4) if (DIR[i] == c) return i;
    assert(false);
}

typedef unsigned long long StateMask;
#define HEXA_MASK(i) MASK((i) << 2)
#define HEXA_BIT(x, i) (((x) >> ((i) << 2)) & 15)

#define POS(x, y) (((x) << 2) ^ (y))
#define ROW(id) ((id) >> 2)
#define COL(id) ((id) & 3)

#define VALUE(state, x, y) HEXA_BIT(state, POS(x, y))

#define INSIDE(x, y) ((x) >= 0 && (x) < 4 && (y) >= 0 && (y) < 4)

StateMask getRow(StateMask mask, int id) {
    return (mask >> (id << 4)) & (MASK(16) - 1);
}

string printRow(StateMask mask) {
    char buf[100];
    memset(buf, 0, sizeof buf);
    string res;
    REP(i, 4) {
        int value = HEXA_BIT(mask, i);
        if (value > 0) sprintf(buf, "%6d", (int)MASK(value));
        else sprintf(buf, "   ---");
        res += string(buf);
    }
    return res;
}

void printBoard(StateMask mask) {
    printf("___BOARD___\n");
    REP(i, 4) cout << printRow(getRow(mask, i)) << endl;
    printf("___END___\n");
}

pair<StateMask, int> moveLeft(StateMask row) {
    int values[4] = {0, 0, 0, 0};
    int col = -1;
    bool merged = false;
    int score = 0;
    REP(i, 4) if (HEXA_BIT(row, i) > 0) {
        if (col >= 0 && values[col] == HEXA_BIT(row, i) && !merged) {
            values[col]++;
            merged = true;
            score += MASK(values[col]);
        } else {
            values[++col] = HEXA_BIT(row, i);
            merged = false;
        }
    }

    StateMask res = 0;
    REP(i, 4) res |= (1ULL * values[i]) << (i << 2);
//    printf("From '%s' to '%s' gains %d points\n", printRow(row).c_str(), printRow(res).c_str(), score);
    return make_pair(res, score);
}

StateMask rotateLeft(StateMask mask) {
    StateMask res = 0;
    REP(i, 4) REP(j, 4) res |= VALUE(mask, i, j) << (POS(3 ^ j, i) << 2);
    return res;
}

pair<StateMask, int> getMove(StateMask mask, int dir) {
    StateMask board = mask;
    REP(love, dir) board = rotateLeft(board);
    int score = 0;
    StateMask newBoard = 0;
    REP(i, 4) {
        pair<StateMask, int> tmp = moveLeft(getRow(board, i));
        newBoard |= tmp.fi << (i << 4);
        score += tmp.se;
    }

//    printf("Begin:\n");
//    printBoard(mask);
//    printf("End:\n");
//    printBoard(newBoard);

    REP(love, (4 - dir) % 4) newBoard = rotateLeft(newBoard);
    return make_pair(newBoard, score);
}

StateMask insertTile(StateMask mask, int pos, int val) {
    assert(HEXA_BIT(mask, pos) == 0);
    return mask | ((1ULL * val) << (4 * pos));
}

template<class Agent> class GameController {
public:
    Agent *agent;
    int totScore, numTurn;
    StateMask state;

    GameController(Agent *agent = NULL) {
        this->agent = agent;
        totScore = numTurn = 0;
        state = 0;
    }

    char findMove(void) {
        if (agent == NULL) {
            string input; cin >> input;
            return input[0];
        }
        return agent->getNextMove(state);
    }

    bool endGame(void) {
        REP(i, 4) if (getMove(state, i).fi != state) return false;
        return true;
    }
    void genMoreTile(void) {
        vector<int> cells;
        REP(i, 4) REP(j, 4) if (VALUE(state, i, j) == 0) cells.push_back(POS(i, j));
        assert(!cells.empty());
        state = insertTile(state, cells[rand() % cells.size()], rand() % 2 + 1);
    }
    pair<StateMask, int> playGame(void) {
        if (state == 0) genMoreTile();
        while (!endGame()) {
            int mv = getDir(findMove());
            pair<StateMask, int> tmp = getMove(state, mv);
            if (state == tmp.fi) {
                printf("INVALID MOVE");
                printf("Total score: %d\n", totScore);
                printf("Number of turns: %d\n", numTurn);
                exit(EXIT_FAILURE);
            }
            totScore += tmp.se;
            numTurn++;
            state = tmp.fi;

            genMoreTile();

            if (numTurn % 10 == 0) {
                printf("Num turn: %d\n", numTurn);
                printBoard(state);
            }
        }

        printf("GAME OVER!\n");
        printf("Total score: %d\n", totScore);
        printf("Number of turns: %d\n", numTurn);
        printBoard(state);
        return make_pair(state, totScore);
    }
};

/*** LOOK AT MY CODE. MY CODE IS AMAZING :D ***/
