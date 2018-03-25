# import the gameplay function, solver and challenger classes
from gameplay import playGame
from cpp_solver import CppSolver as Solver
from random_challenger import RandomChallenger as Challenger

# initialize the solver and challenger
solver = Solver()
challenger = Challenger()

# play the game
playGame(solver, challenger)
