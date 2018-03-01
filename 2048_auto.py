# import the gameplay function, solver and challenger classes
from gameplay import playGame
from random_solver import RandomSolver as Solver
from random_challenger import RandomChallenger as Challenger

# initialize the solver and challenger
solver = Solver()
challenger = Challenger()

# play the game
playGame(solver, challenger)
