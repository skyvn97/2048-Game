import pygame
from random import randint
from classes import SolvingAgent

BOARD_SIZE = 4
VALID_ACTIONS = [pygame.K_UP, pygame.K_DOWN, pygame.K_LEFT, pygame.K_RIGHT]

class RandomSolver(SolvingAgent):
    # this function receives the tileMatrix as a BOARD_SIZE x BOARD_SIZE array
    # returns one of the four values in VALID_ACTIONS corresponding to the moving direction 
    def getAction(self, tileMatrix):
        return VALID_ACTIONS[randint(0, len(VALID_ACTIONS) - 1)]
