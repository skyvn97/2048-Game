#include "gameplay.h"

typedef short Score;

const Score INF = (Score)15151;
const long long INF_LL = (long long)1e18 + 7LL;

const int INIT_DEPTH = 9;
const int MAX_DEPTH = 50;
const int CALL_PER_TURN = (int)7e4;
const int MAX_NODE = (int)3e7;
const int MAX_TOTAL_VALUE = (int)1e5;

constexpr double HEURISTIC_RATE = 0.2;
constexpr double INSERT_RATE = 1.0;

int limitDepth, limitNode, numTurn, numNode, numCall;

const int WEIGHT[4][4] = {{15, 14, 13, 12}, {8, 9, 10, 11}, {7, 6, 5, 4}, {0, 1, 2, 3}};

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

class Node {
public:
    static const int MAX_CACHED_DEPTH = 15;

    #define MOVE   true
    #define GEN   false

    StateMask state;
    unsigned char numChild;
    bool type;
    Score* bonus;
    Node** children;
    Score minCache[(MAX_CACHED_DEPTH >> 1) + 1], maxCache[(MAX_CACHED_DEPTH >> 1) + 1];

    Node(StateMask state = 0, bool type = false) {
        this->state = state; this->type = type;
        numChild = 0;
        bonus = NULL; children = NULL;
        memset(minCache, -0x3f, sizeof minCache);
        memset(maxCache, 0x3f, sizeof maxCache);
    }

    void assign(StateMask state, bool type) {
        delete[] bonus;
        delete[] children;
        *this = Node(state, type);
        assert(bonus == NULL && children == NULL);
    }

    void calcChildren();

    Score getValue(int depth = 0, Score minExp = -INF, Score maxExp = INF) {
//        printf("CALL at %llu, depth %d, with range (%d, %d)\n", state, depth, minExp, maxExp);
        if (depth >= limitDepth) return 0;
        if (minExp >= maxExp) return minExp;

        int cachedSlot = limitDepth - depth <= MAX_CACHED_DEPTH ? (limitDepth - depth) >> 1 : -1;
        if (cachedSlot >= 0) assert(minCache[cachedSlot] <= maxCache[cachedSlot]);
        if (depth > 0 && cachedSlot >= 0) {
            if (maxExp <= minCache[cachedSlot]) return minCache[cachedSlot];
            if (minExp >= maxCache[cachedSlot]) return maxCache[cachedSlot];
            if (minCache[cachedSlot] == maxCache[cachedSlot]) return minCache[cachedSlot];
        }

        numCall++;

        calcChildren();
        if (numChild == 0) return 0;

        if (type == MOVE) {
            Score best = minExp;
            REP(i, numChild) {
                Score gain = bonus[i];
                Score tmp = gain + children[i]->getValue(depth + 1, best - gain, maxExp - gain);
                if (tmp >= maxExp) {
                    if (cachedSlot >= 0) maximize(minCache[cachedSlot], maxExp);
                    return maxExp;
                }
                maximize(best, tmp);
            }
            if (cachedSlot >= 0) {
                if (minExp < best && best < maxExp) minCache[cachedSlot] = maxCache[cachedSlot] = best;
                else minimize(maxCache[cachedSlot], minExp);
            }

            return best;
        } else {
            assert(numChild > 0);

            Score worst = maxExp;
            REP(i, numChild) {
                Score gain = 0;
                Score tmp = gain + children[i]->getValue(depth + 1, minExp - gain, worst - gain);
                if (tmp <= minExp) {
                    if (cachedSlot >= 0) minimize(maxCache[cachedSlot], minExp);
                    return minExp;
                }
                minimize(worst, tmp);
            }
            if (cachedSlot >= 0) {
                if (minExp < worst && worst < maxExp) minCache[cachedSlot] = maxCache[cachedSlot] = worst;
                else maximize(minCache[cachedSlot], maxExp);
            }

            return worst;
        }
    }

    int getBestMove(void) {
        assert(type == MOVE);
        calcChildren();

        double moveValue[numChild];
        REP(i, numChild) {
            int score = bonus[i] + children[i]->getValue(1);
            long long worstInsertion = INF_LL;
            children[i]->calcChildren();
            REP(j, children[i]->numChild)
                minimize(worstInsertion, heuristic(children[i]->children[j]->state));
            moveValue[i] = (score + HEURISTIC_RATE) / (1 + HEURISTIC_RATE);
        }

        int bestMove = 0;
        REP(i, numChild) if (moveValue[i] > moveValue[bestMove]) bestMove = i;
        return getDirFromState(state, children[bestMove]->state);
    }
};

unordered_map<StateMask, Node*> pointers[2];
vector<Node*> statesWithValue[MAX_TOTAL_VALUE];
int valueToReplace, currentTotalValue;

Node* getNodeToReplace(void) {
    while (valueToReplace < currentTotalValue) {
        vector<Node*> &tmp = statesWithValue[valueToReplace];
        if (!tmp.empty()) {
            Node* res = tmp.back(); tmp.pop_back();
            return res;
        }
        valueToReplace++;
    }
    return NULL;
}

Node* getNodePointer(StateMask state, bool type) {
    unordered_map<StateMask, Node*> &cur = pointers[type];
    if (cur.find(state) != cur.end()) return cur[state];

    Node *tmp = getNodeToReplace();
    if (tmp == NULL) {
        if (++numNode > MAX_NODE) {
            fprintf(stderr, "%d turns, %d nodes, %d calls\n", numTurn, numNode, numCall);
            exit(EXIT_FAILURE);
        }
        tmp = new Node(state, type);
    }
    else {
        pointers[tmp->type].erase(tmp->state);
        tmp->assign(state, type);
    }

    statesWithValue[totalValue(state)].push_back(tmp);
    return cur[state] = tmp;
}

void Node::calcChildren(void) {
    if (children != NULL) return;

    if (type == MOVE) {
        children = new Node*[4];
        bonus = new Score[4];
        REP(i, 4) {
            children[i] = NULL; bonus[i] = 0;
        }

        REP(dir, 4) {
            pair<StateMask, int> outcome = getStateAfterMove(state, dir);
            if (outcome.fi == state) continue;
            children[numChild] = getNodePointer(outcome.fi, type ^ 1);
            bonus[numChild] = outcome.se;
            numChild++;
        }
    } else {
        int cnt = 0;
        REP(i, 16) if (HEXA_BIT(state, i) == 0) cnt++;
        StateMask gens[cnt * 2 + 1];
        cnt = 0;
        REP(cell, 16) if (HEXA_BIT(state, cell) == 0) FOR(value, 1, 2)
            gens[cnt++] = insertTile(state, cell, value);
        random_shuffle(gens, gens + cnt);
        numChild = cnt * INSERT_RATE + 0.5;
        children = new Node*[numChild];
        REP(i, numChild) children[i] = getNodePointer(gens[i], type ^ 1);
    }
}

int getNextMove(StateMask state) {
    Node* node = getNodePointer(state, MOVE);
    int oldNumCall = numCall;

    limitDepth = INIT_DEPTH;
    while (true) {
        int mv = node->getBestMove();
        if (limitDepth + 2 > MAX_DEPTH || numCall >= oldNumCall + CALL_PER_TURN) {
            cerr << "Depth: " << limitDepth << endl;
            cerr << "Total extra calls: " << numCall - oldNumCall << endl;
            return mv;
        }
        limitDepth += 2;
        oldNumCall = numCall;
    }
}

int main(void) {
    while (true) {
        numTurn++;

        int board[4][4];
        memset(board, 0, sizeof board);

        REP(i, 4) REP(j, 4) {
            assert(cin >> board[i][j]);
            if (board[i][j] < 0) {
                fprintf(stderr, "%d turns, %d nodes, %d calls\n", numTurn, numNode, numCall);
                exit(EXIT_SUCCESS);
            }
        }
        StateMask state = getMaskFromBoard(board);
        currentTotalValue = totalValue(state);
        cout << getNextMove(state) << endl;
    }
}
