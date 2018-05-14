# 2048-Game
This repository contains several AI agents playing the popular game 2048. A python implementation written by lewisjdeane is used to support graphics viewing. Playable on your desktop with some neat features. To run this you will need python and pygame installed.

## Download

Clone/Fork this repository ot your computer to start developing. And then create pull requests to submit your code.

## What is included?
Inside the solving_agent folder, we have

- Two agents solving the game using AI based algorithms: Alphabeta prunning and Expectimax

- Two agents solving the game using Reinforcement learning models.

- One smart challenger generating new tiles for the purpose of hinder the solver.

- One challenger generating normally

Also, we have an implemetation written in python originally by lewisjdeane, has been modified to adapt our aim of auto playing.

## Usage

Only AI based agents are compatible with the python implementation. To watch the game, run 'python3 2048_auto.py' with 2 parameters: The commandline to execute the solving agent and the command line to execute the challenging agent.

For example, to see expectimax plays against random insertion, run: 'python3 2048_auto.py ./solving_agent/expectimax ./solving_agent/random_challenger'

Run './alphabeta_interactive --practice' to let the agent plays multiple game and produce the results (for analyzing purpose). The same feature applies for expectimax.
