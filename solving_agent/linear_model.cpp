#include "gameplay.h"

/* Author: Van Hanh Pham */

/** END OF TEMPLATE - ACTUAL SOLUTION COMES HERE **/

long long randLong(void) {
    long long res = 0;
    REP(love, 3) res = (res << 20) ^ (rand() & (MASK(20) - 1));
    return res;
}

class Vector {
private:
    vector<double> values;

public:
    Vector(int sz = 0, bool random = false) {
        values.assign(sz, 0);
        if (random) {
            FORE(it, values) *it = sin(rand());
        }
    }

    int size(void) const {
        return values.size();
    }

    double& operator [] (int id) {
        return values[id];
    }

    double operator [] (int id) const {
        return values[id];
    }

    Vector operator + (const Vector &v) const {
        assert(size() == v.size());
        Vector res(size());
        REP(i, size()) res[i] = values[i] + v[i];
        return res;
    }

    Vector operator - (const Vector &v) const {
        assert(size() == v.size());
        Vector res(size());
        REP(i, size()) res[i] = values[i] + v[i];
        return res;
    }

    Vector operator * (double k) const {
        Vector res(size());
        REP(i, size()) res[i] = values[i] * k;
        return res;
    }

    double operator * (const Vector &v) const {
//        debug();
        assert(size() == v.size());
        double res = 0;
        REP(i, size()) {
            res += values[i] * v[i];
//            if (IS_INF(res) || IS_NAN(res)) {
//                debug();
//                printf("FAILURE %.3lf and %.3lf\n", values[i], v[i]);
//                exit(EXIT_FAILURE);
//            }
        }

        return res;
    }

    void debug(void) const {
        printf("[");
        REP(i, size()) printf("%.3lf, ", values[i]);
        printf("]\n");
        REP(i, size()) if (Abs(values[i]) > 10000)
            printf("Element %d is %.3lf\n", i, values[i]);
    }
};

Vector operator * (double k, const Vector &v) {
    return v * k;
}

/**
    Feature lists:
        - Cell values from 0 to 15: 16 elements
        - Cell values 0, from 2^1 to 2^15: 16 elements
        - Number, squared and cubed number of cells with value from 0 to 15: 48 elements
        - Biggest cell from 1 to 15
        - Biggest cell from 2^1 to 2^15
        - Total borders of cells with highest value, 2nd highest value,...: 16 elements
        - Total Manhattan distance between cells of each value: 16 elements
        - Total pair of adjacent same number of each value: 16 elements
        - Difference between values, squared and cubed values in pairs of adjacent cells: 96 elements
        - Number of greater adjacent cells of every cells: 16 elements
*/

const int NUM_FEATURE = 242;

int NUM_BORDER[4][4];
int MANHATTAN[16][16];

const int NUM_SAMPLE = 40;
const int NUM_TRAINING = 100;

constexpr double LEARNING_RATE = 0.01;
const int RANDOM_DENOMINATOR = 1024;
const int RANDOM_RATE = 50;

void prepare(void) {
    REP(i, 16) REP(j, 16) MANHATTAN[i][j] = Abs(ROW(i) - ROW(j)) + Abs(COL(i) - COL(j));
    REP(i, 4) REP(j, 4) REP(d, 4) if (!INSIDE(i + dx[d], j + dy[d])) NUM_BORDER[i][j]++;
}

#define CUBE(x) (SQR(x) * (x))

vector<double> rawStateMeasurements(StateMask state) {
    assert(MANHATTAN[1][2] == 1);

    int value[4][4];
    int countValue[16], countBorder[16];
    memset(countValue, 0, sizeof countValue);
    memset(countBorder, 0, sizeof countBorder);
    REP(i, 4) REP(j, 4) {
        value[i][j] = VALUE(state, i, j);
        countValue[value[i][j]]++;
        countBorder[value[i][j]] += NUM_BORDER[i][j];
    }

    int id = 0;
    vector<double> res(NUM_FEATURE, 0);

    #define SCALE(x, l, r) (1.0 * ((x) - (l)) / ((r) - (l)))

    // cell values
    REP(i, 4) REP(j, 4) res[id++] = SCALE(value[i][j], 0, 15);
    REP(i, 4) REP(j, 4) res[id++] = SCALE(value[i][j] == 0 ? 0 : MASK(value[i][j]), 0, MASK(15));

    // number, squared and cubed numbers of cells with each value
    REP(i, 16) {
        int x = countValue[i];
        res[id++] = SCALE(x, 0, 16);
        res[id++] = SCALE(SQR(x), 0, 256);
        res[id++] = SCALE(CUBE(x), 0, 4096);
    }

    // biggest cell
    FORD(i, 15, 0) if (countValue[i] > 0) {
        res[id++] = SCALE(i, 0, 15);
        res[id++] = SCALE(MASK(i), 0, MASK(15));
        break;
    }

    // total borders of cells with each value
    int numValue = 0;
    FORD(i, 15, 0) if (countValue[i] > 0) {
        numValue++;
        res[id++] = SCALE(countBorder[i], 0, 10);
    }
    REP(aye, 16 - numValue) res[id++] = 0;

    // total distance between cells of each value
    REP(i, 16) if (value[ROW(i)][COL(i)] > 0) FOR(j, i + 1, 15) {
        int x = value[ROW(i)][COL(i)];
        int y = value[ROW(j)][COL(j)];
        if (x == y) res[id + x] += MANHATTAN[i][j];
    }
    REP(i, 16) res[id + i] = SCALE(res[id + i], 0, 100);
    id += 16;

    // total number of adjacent pair with same number
    REP(i, 4) REP(j, 4) {
        if (i + 1 < 4 && value[i][j] == value[i + 1][j])
            res[id + value[i][j]]++;
        if (j + 1 < 4 && value[i][j] == value[i][j + 1])
            res[id + value[i][j]]++;
    }
    REP(i, 16) res[id + i] = SCALE(res[id + i], 0, 32);
    id += 16;

    // difference between value of adjacent cells
    REP(i, 4) REP(j, 4) REP(k, 2) {
        int x = value[i][j];
        int y = k ? (i + 1 < 4 ? value[i + 1][j] : 0) : (j + 1 < 4 ? value[i][j + 1] : 0);
        if (x == 0 || y == 0) {
            REP(love, 3) res[id++] = 0;
        } else {
            res[id++] = SCALE(Abs(x - y), 0, 15);
            res[id++] = SCALE(Abs(SQR(x) - SQR(y)), 0, 255);
            res[id++] = SCALE(Abs(CUBE(x) - CUBE(y)), 0, 3375);
        }
    }

    // number of greater adjacent cells of every cells
    REP(i, 4) REP(j, 4) {
        int cnt = 0;
        REP(k, 4) {
            int x = i + dx[k], y = j + dy[k];
            if (INSIDE(x, y) && value[x][y] > value[i][j]) cnt++;
        }
        res[id++] = SCALE(cnt, 0, 4);
    }

    assert(id == NUM_FEATURE);
    return res;
}

Vector stateMeasurements(StateMask state) {
    vector<double> values = rawStateMeasurements(state);
    Vector res(values.size() + 1);
    REP(i, values.size()) res[i] = values[i];
    res[res.size() - 1] = 1;
    return res;
}

Vector coefficients;

double estimateExpectedGain(StateMask state) {
    double res = coefficients * stateMeasurements(state);
    if (IS_INF(res) || IS_NAN(res)) {
//        coefficients.debug();
//        stateMeasurements(state).debug();
//
//        printf("%.3lf\n", res);
        exit(EXIT_FAILURE);
    }
    return coefficients * stateMeasurements(state), 0.0;
}

double expectedInsertion(StateMask state) {
    double res = 0;
    int numCell = 0;

    REP(cell, 16) if (HEXA_BIT(state, cell) == 0) {
        numCell++;
        FOR(value, 1, 2) res += estimateExpectedGain(insertTile(state, cell, value));
    }

//    printf("EXP %.3lf\n", res / numCell / 2);
    return res / numCell / 2;
}

int findBestMove(StateMask state) {
    pair<StateMask, int> outcome[4];
    REP(i, 4) outcome[i] = getStateAfterMove(state, i);

    if (rand() % RANDOM_DENOMINATOR < RANDOM_RATE) {
        while (true) {
            int dir = rand() & 3;
            if (outcome[dir].fi != state) return dir;
        }
    }

    double best = -INFINITY;
    int bestMove = -1;
    REP(i, 4) if (outcome[i].fi != state && maximize(best, outcome[i].se + expectedInsertion(outcome[i].fi)))
        bestMove = i;
    if (0 <= bestMove && bestMove < 4) return bestMove;
    printf("FAILED\n");
    printBoard(state);
    printf("%.3lf\n", best);
    exit(EXIT_FAILURE);
}

vector<pair<StateMask, int>> playGame(bool printResult = false) {
    StateMask cur = randomInsert(0);
    vector<pair<StateMask, int>> history(1, make_pair(cur, 0));

    int score = 0;
    while (!endGame(cur)) {
        int mv = findBestMove(cur);
        pair<StateMask, int> next = getStateAfterMove(cur, mv);
        assert(next.fi != cur);
        score += next.se;
        cur = randomInsert(next.fi);
        history.push_back(make_pair(cur, score));
    }

    if (printResult) {
        printf("%d points, %d turns\n", score, (int)history.size() - 1);
        printf("Final state:\n");
        printBoard(cur);
        printf("-----\n");
    }

    FORE(it, history) it->se = score - it->se;
    return history;
}

void training(const vector<pair<StateMask, int>> &results) {
    FORE(it, results) {
        Vector point = stateMeasurements(it->fi);
        double estimate = coefficients * point;
        coefficients = coefficients + LEARNING_RATE * (it->se - estimate) * point;
//        printf("board:\n");
//        printBoard(it->fi);
//        printf("give: %d, estimate %.3lf\n", it->se, estimate);
//        printf("Point:\n");
//        point.debug();
//        printf("AFTER TRAINING\n");
//        coefficients.debug();
    }
}

void process(const string &inputFile) {
    coefficients = Vector(NUM_FEATURE + 1, true);

    if (inputFile != "") {
        freopen(inputFile.c_str(), "r", stdin);
        vector<double> saved;
        double input;
        while (cin >> input) saved.push_back(input);
        REP(i, (int)saved.size() - 1) coefficients[i] = saved[i];
        coefficients[coefficients.size() - 1] = saved.back();
    }

    int bestScore = 0;
    REP(love, 50) {
        cerr << "Batch " << love + 1 << endl;

        long long totScore = 0;
        long long totSQR = 0;
        vector<pair<StateMask, int>> games;
        REP(love, NUM_TRAINING) {
//            coefficients.debug();
            vector<pair<StateMask, int>> game = playGame(love == 0);
            training(game);
            totScore += game.front().se;
            totSQR += SQR(game.front().se);
            maximize(bestScore, game.front().se);
            games.push_back(make_pair(game.back().fi, game.front().se));
        }

//        FORE(it, games) if (randLong() % totSQR < NUM_SAMPLE * SQR(it->se)) {
//            int maxTile = 0;
//            REP(cell, 16) maximize(maxTile, (int)MASK(HEXA_BIT(it->fi, cell)));
//            printf("%d points, maxTile = %d\n", it->se, maxTile);
//        }

        printf("average: %.3lf\n", 1.0 * totScore / NUM_TRAINING);
        printf("best score: %d\n", bestScore);
        system("sleep 1s");
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    prepare();
    process(argv == NULL || argv[1] == NULL ? "" : string(argv[1]));
    return 0;
}

/*** LOOK AT MY CODE. MY CODE IS AMAZING :D ***/
