
import numpy as np

class Butter(object):
    def __init__(self):
        self.A = np.array([[0.6742,  -0.2115],[0.2115,   0.9733]])
        self.B = np.array([[0.2991],[0.0378]])
        self.C = np.array([0.0748,   0.6977])
        self.D = 0.0134
        self.X = np.array([[0.0],[0.0]])

    def update(self, U):
        self.X = np.dot(self.A,self.X) + np.dot(self.B,U)
        y = np.dot(self.C,self.X) + np.dot(self.D,U)
        return y[0]

if __name__ == '__main__':
    butt = Butter()
    print butt.update(1)
    print butt.update(1)
    print butt.update(1)
    print butt.update(1)
