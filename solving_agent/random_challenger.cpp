#include "gameplay.h"

int main(int argc, char* argv[]) {
    srand(time(NULL));

    while (true) {
        int board[4][4];
        REP(i, 4) REP(j, 4) {
            assert(cin >> board[i][j]);
            if (board[i][j] < 0) return 0;
        }
        StateMask state = getMaskFromBoard(board);
        pair<int, int> tmp(-1, rand() % 2 + 1);
        while (true) {
            int cell = rand() % 16;
            if (HEXA_BIT(state, cell) == 0) {
                tmp.fi = cell;
                break;
            }
        }
        cout << COL(tmp.fi) << "\n" << ROW(tmp.fi) << "\n" << MASK(tmp.se) << endl;
    }

    return 1;
}
