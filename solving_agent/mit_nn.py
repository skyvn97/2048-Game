import numpy as np

class GameUtils:
    '''
        Direction ID: 0 - LEFT, 1 - UP, 2 - RIGHT, 3 - DOWN
    '''

    @staticmethod
    def rotateClockwise(board):
        res = np.zeros((4, 4), dtype=int)
        for i in range(4):
            for j in range(4):
                res[j][3 - i] = board[i][j]
        return res

    @staticmethod
    def mergeLeft(board):
        newBoard = np.zeros((4, 4), dtype=int)
        score = 0
        for i in range(4):
            newRow = list()
            lastMerge = True
            for j in range(4):
                if board[i][j] > 0:
                    if not lastMerge and newRow[-1] == board[i][j]:
                        newRow[-1] += 1
                        score += 1 << newRow[-1]
                        lastMerge = True
                    else:
                        newRow.append(board[i][j])
                        lastMerge = False
            for j in range(4):
                newBoard[i][j] = newRow[j] if j < len(newRow) else 0
        return (newBoard, score)

    @staticmethod
    def getBoardAfterMove(board, dir):
        for i in range(4 - dir):
            board = GameUtils.rotateClockwise(board)
        board, score = GameUtils.mergeLeft(board)
        for i in range(dir):
            board = GameUtils.rotateClockwise(board)
        return (board, score)

    @staticmethod
    def insertTile(board, row, col, value):
        assert 0 <= row and row < 4
        assert 0 <= col and col < 4
        assert value == 1 or value == 2
        assert board[row][col] == 0
        board[row][col] = value

    @staticmethod
    def randomInsert(board):
        while (True):
            row = np.random.randint(0, 4)
            col = np.random.randint(0, 4)
            if board[row][col] == 0:
                board[row][col] = np.random.randint(1, 3)
                return
    @staticmethod
    def randomStartBoard():
        state = np.zeros((4, 4), dtype=int)
        for i in range(2):
            GameUtils.randomInsert(state)
        return state

    @staticmethod
    def canMove(board, dir):
        newBoard = GameUtils.getBoardAfterMove(board, dir)[0]
        for i in range(4):
            for j in range(4):
                if board[i][j] != newBoard[i][j]:
                    return True
        return False

    @staticmethod
    def endGame(board):
        for i in range(4):
            if GameUtils.canMove(board, i):
                return False
        return True

class GameEvent:
    def __init__(self, state=None, action=None, prob=None):
        self.state = state
        self.action = action
        self.prob = prob

class NeuralNetwork:
    LAYER_SIZE = [256, 200, 100, 4]
    NUM_LAYER = len(LAYER_SIZE) - 1

    LEARNING_RATE = 0.01
    REWARD_DIVISOR = 1000

    def __init__(self):
        self.weight = [None] * (NeuralNetwork.NUM_LAYER + 1)
        self.bias = [None] * (NeuralNetwork.NUM_LAYER + 1)
        self.tmp = [None] * (NeuralNetwork.NUM_LAYER + 1)
        self.output = [None] * (NeuralNetwork.NUM_LAYER + 1)

        for i in range(1, NeuralNetwork.NUM_LAYER + 1):
            self.weight[i] = np.random.randn(NeuralNetwork.LAYER_SIZE[i],
                                              NeuralNetwork.LAYER_SIZE[i - 1]) / 10
            self.bias[i] = np.random.randn(NeuralNetwork.LAYER_SIZE[i], 1) / 10
            self.tmp[i] = np.zeros_like(self.bias[i], dtype=int)
            self.output[i] = np.zeros_like(self.bias[i], dtype=int)

    def getOutput(self, data):
        self.output[0] = data
        for i in range(1, NeuralNetwork.NUM_LAYER + 1):
            self.tmp[i] = np.matmul(self.weight[i], self.output[i - 1]) + self.bias[i]
            if i < NeuralNetwork.NUM_LAYER:
                self.output[i] = np.maximum(self.tmp[i], 0)
            else:
                # if data.shape[1] == 1:
                #     print(self.tmp[i])
                exp = np.exp(self.tmp[i])
                sumExp = np.sum(exp)
                # if data.shape[1] == 1:
                #     print(sumExp)
                self.output[i] = exp / sumExp
        return self.output[NeuralNetwork.NUM_LAYER]

    def updateNetwork(self, data):
        numTest = data.shape[1]
        for i in range(NeuralNetwork.NUM_LAYER, 0, -1):
            dTmp = None
            if i == NeuralNetwork.NUM_LAYER:
                dTmp = data
            else:
                dTmp = data
                for r in range(dTmp.shape[0]):
                    for c in range(dTmp.shape[1]):
                        if self.tmp[i][r][c] <= 0:
                            dTmp[r][c] = 0
            dBias = 1.0 / numTest * np.sum(dTmp, axis=1, keepdims=True)
            dWeight = 1.0 / numTest * np.matmul(dTmp, self.output[i - 1].T)
            data = np.matmul(self.weight[i].T, dTmp)

            self.weight[i] -= NeuralNetwork.LEARNING_RATE * dWeight
            self.bias[i] -= NeuralNetwork.LEARNING_RATE * dBias

    def trainNetwork(self, game, reward):
        input = np.column_stack([NeuralNetwork.stateRepresent(event.state) for event in game])
        func = lambda event : (event.action - event.prob) * reward / NeuralNetwork.REWARD_DIVISOR
        output = np.column_stack([func(event) for event in game])
        self.getOutput(input)
        self.updateNetwork(output)

    def getAction(self, board):
        output = self.getOutput(np.column_stack([NeuralNetwork.stateRepresent(board)]))
        for dir in range(4):
            if not GameUtils.canMove(board, dir):
                output[dir][0] = 0

        sumProb = np.sum(output)
        threshold = np.random.rand() * sumProb

        for dir in range(4):
            if output[dir][0] > threshold:
                assert GameUtils.canMove(board, dir)
                return (dir, output)
            else:
                threshold -= output[dir][0]
        assert False

    @staticmethod
    def stateRepresent(board):
        res = np.array([0] * 256)
        id = 0
        for i in range(4):
            for j in range(4):
                res[id + board[i][j]] = 1
                id += 16
        return res

class GameController:
    NUM_GAME = 50

    def __init__(self):
        self.brain = NeuralNetwork()

    def playGame(self):
        state = GameUtils.randomStartBoard()
        history = list()
        totScore = 0
        while not GameUtils.endGame(state):
            action, prob = self.brain.getAction(state)
            va = np.zeros((4, 1))
            va[action][0] = 1
            history.append(GameEvent(state, va, prob))
            state, score = GameUtils.getBoardAfterMove(state, action)
            GameUtils.randomInsert(state)
            totScore += score
        reward = 0
        for i in range(4):
            for j in range(4):
                if state[i][j] > 0:
                    reward += 1 << int(state[i][j])
        return (history, reward, state, totScore)

    def run(self):
        for love in range(200):
            total = 0
            for love in range(GameController.NUM_GAME):
                history, reward, lastState, score = self.playGame()
                self.brain.trainNetwork(history, reward)
                total += score
                if np.random.randint(0, 10) == 0:
                    maxTile = 1 << np.max(lastState)
                    print("score: %d, maxTile: %d" % (score, maxTile))


controller = GameController()
controller.run()