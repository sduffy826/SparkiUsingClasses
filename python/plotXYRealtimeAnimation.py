import random
from itertools import count
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

plt.style.use('fivethirtyeight')

x_vals = []
y_vals = []

index = count()

# This is actually redrawing the entire plot each time.... it's ok, so fast it's not a problem
# for our data.  May want to read more on it down the road and have it only update graph... but
# that's more complicated as the configuration space may also change
def animate(i):
  data = pd.read_csv('data.csv')
  x = data['x_value']
  y = data['y_value']

  plt.cla()  # Clears axis

  plt.plot(x, y, label='My travels :)')

  plt.legend(loc='upper left')
  plt.tight_layout()

ani = FuncAnimation(plt.gcf(), animate, interval=1000)

plt.tight_layout()
plt.show()