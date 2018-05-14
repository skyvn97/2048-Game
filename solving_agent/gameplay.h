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
#define SQR(x) (1LL * (x) * (x))
#define IS_INF(x) (std::isinf(x))
#define IS_NAN(x) (std::isnan(x))
#define div   ___div
#define next   ___next
#define prev   ___prev
#define left   ___left
#define right   ___right
#define distance   ___distance
#define __builtin_popcount __builtin_popcountll
using namespace std;
template<class X, class Y>
    bool minimize(X &x, const Y &y) {
        X eps = 1e-9;
        if (x > y + eps) {
            x = y;
            return true;
        } else return false;
    }
template<class X, class Y>
    bool maximize(X &x, const Y &y) {
        X eps = 1e-9;
        if (x + eps < y) {
            x = y;
            return true;
        } else return false;
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

StateMask getMaskFromBoard(const int board[4][4]) {
    StateMask res = 0;
    REP(i, 4) REP(j, 4) REP(k, 20) if ((int)MASK(k) == board[i][j])
        res |= (1ULL * k) << (4 * POS(i, j));
    return res;
}

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
    cerr << "___BOARD___\n";
    REP(i, 4) cerr << printRow(getRow(mask, i)) << endl;
    cerr << "___END___" << endl;
}

int totalValue(StateMask mask) {
    int res = 0;
    REP(row, 4) REP(col, 4) if (VALUE(mask, row, col) > 0)
        res += MASK(VALUE(mask, row, col));
    return res;
}

pair<StateMask, int> moveLeft(StateMask row) {
    int values[4] = {0, 0, 0, 0};
    int col = -1;
    bool merged = false;
    int score = 0;
    REP(i, 4) if (HEXA_BIT(row, i) > 0) {
        if (col >= 0 && values[col] == (int)HEXA_BIT(row, i) && !merged) {
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

StateMask flipAllRows(StateMask mask) {
    StateMask res = 0;
    REP(i, 4) REP(j, 4) res |= VALUE(mask, i, j) << (POS(i, 3 ^ j) << 2);
    return res;
}

pair<StateMask, int> getStateAfterMoveSLOW(StateMask mask, int dir) {
    assert(false);

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

pair<StateMask, int> getStateAfterMove(StateMask mask, int dir) {
    StateMask newMask = 0;
    int score = 0;

    // LEFT
    if (dir == 0) {
        REP(row, 4) {
            StateMask last = 0;
            int pos = -1;
            REP(col, 4) if (VALUE(mask, row, col) > 0) {
                if (VALUE(mask, row, col) == last) {
                    newMask ^= (last ^ (last + 1)) << (POS(row, pos) << 2);
                    score += MASK(last + 1);
                    last = 0;
                } else {
                    last = VALUE(mask, row, col);
                    newMask ^= last << (POS(row, ++pos) << 2);
                }
            }
        }
    }

    // UP
    if (dir == 1) {
        REP(col, 4) {
            StateMask last = 0;
            int pos = -1;
            REP(row, 4) if (VALUE(mask, row, col) > 0) {
                if (VALUE(mask, row, col) == last) {
                    newMask ^= (last ^ (last + 1)) << (POS(pos, col) << 2);
                    score += MASK(last + 1);
                    last = 0;
                } else {
                    last = VALUE(mask, row, col);
                    newMask ^= last << (POS(++pos, col) << 2);
                }
            }
        }
    }

    // RIGHT
    if (dir == 2) {
        REP(row, 4) {
            StateMask last = 0;
            int pos = 4;
            FORD(col, 3, 0) if (VALUE(mask, row, col) > 0) {
                if (VALUE(mask, row, col) == last) {
                    newMask ^= (last ^ (last + 1)) << (POS(row, pos) << 2);
                    score += MASK(last + 1);
                    last = 0;
                } else {
                    last = VALUE(mask, row, col);
                    newMask ^= last << (POS(row, --pos) << 2);
                }
            }
        }
    }

    // DOWN
    if (dir == 3) {
        REP(col, 4) {
            StateMask last = 0;
            int pos = 4;
            FORD(row, 3, 0) if (VALUE(mask, row, col) > 0) {
                if (VALUE(mask, row, col) == last) {
                    newMask ^= (last ^ (last + 1)) << (POS(pos, col) << 2);
                    score += MASK(last + 1);
                    last = 0;
                } else {
                    last = VALUE(mask, row, col);
                    newMask ^= last << (POS(--pos, col) << 2);
                }
            }
        }
    }
    return make_pair(newMask, score);
}

int getDirFromState(StateMask from, StateMask to) {
    REP(dir, 4) if (getStateAfterMove(from, dir).fi == to) return dir;
    assert(false);
}

StateMask insertTile(StateMask mask, int pos, int val) {
    assert(HEXA_BIT(mask, pos) == 0);
    return mask | ((1ULL * val) << (pos << 2));
}

StateMask endGame(StateMask state) {
    REP(dir, 4) if (getStateAfterMove(state, dir).fi != state) return false;
    return true;
}

int cellID[17] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1};
StateMask randomInsert(StateMask state) {
    random_shuffle(cellID, cellID + 16);
    REP(i, 16) if (HEXA_BIT(state, cellID[i]) == 0)
        return insertTile(state, cellID[i], rand() % 2 + 1);
    assert(false);
}

/*** LOOK AT MY CODE. MY CODE IS AMAZING :D ***/
