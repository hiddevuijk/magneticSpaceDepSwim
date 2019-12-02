import numpy as np
import matplotlib.pyplot as plt

w = 1.5
L = 10
x = np.linspace(0,L,1000)

def f(x):
    return 0.5*(np.tanh(w*(x-L/4) )*np.tanh( w*(3*L/4 - x)) - np.tanh(-w*L/4)*np.tanh(3*L/4))
def g(x):
    return np.tanh(w*(L/4- abs(x-L/2) )) + np.tanh(w*3*L/4)

plt.plot(x,f(x))
plt.plot(x,g(x))
plt.show()

