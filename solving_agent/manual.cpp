#include "gameplay.h"

char GameController::findMove(void) {
    string moves;
    REP(i, 4) if (getMove(state, i).fi != state) moves.push_back(DIR[i]);
    assert(!moves.empty());
    return moves[rand() % moves.size()];
}

int main(void) {
    srand(time(NULL));
    GameController controller;
    controller.playGame();
    return 0;
}
