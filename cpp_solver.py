import pygame
from subprocess import call
from random import randint
from classes import SolvingAgent

BOARD_SIZE = 4
VALID_ACTIONS = [pygame.K_LEFT, pygame.K_UP, pygame.K_RIGHT, pygame.K_DOWN]

def getLog(value):
    if value == 0:
        return 0
    for i in range(20):
        if value == (1 << i):
            return i

def getState(tileMatrix):
    # print(tileMatrix)
    state = 0
    for i in range(BOARD_SIZE):
        for j in range(BOARD_SIZE):
            state |= getLog(tileMatrix[i][j]) << ((4 * j + i) << 2)
    return state

class CppSolver(SolvingAgent):
    # this function receives the tileMatrix as a BOARD_SIZE x BOARD_SIZE array
    # returns one of the four values in VALID_ACTIONS corresponding to the moving direction 
    def getAction(self, tileMatrix):
        state = getState(tileMatrix)
        # print("Send state %d" % (state))
        move = call(["./solving_agent/alphabeta", str(state)])
        # print("Receive move %d" % (move))
        return VALID_ACTIONS[move]

