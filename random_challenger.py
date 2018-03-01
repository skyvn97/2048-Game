from random import randint
from classes import ChallengingAgent

BOARD_SIZE = 4

class RandomChallenger(ChallengingAgent):
    # this function receives the current tileMatrix as a BOARD_SIZE * BOARD_SIZE matrix
    # this returns a tuple (x, y, k) meaning that the cell at x-th row and y-th column should have a new tile of value k
    # rows and columns are numbered from 0 to BOARD_SIZE - 1.
    # the inserted cell (x, y) should be an empty cell in tileMatrix
    def getNewTile(self, tileMatrix):
        while True:
            x = randint(0, BOARD_SIZE - 1)
            y = randint(0, BOARD_SIZE - 1)
            if tileMatrix[x][y] != 0:
                continue
            return x, y, randint(1, 2) * 2
