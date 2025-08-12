import pyqtgraph as pg
import numpy as np
from pyqtgraph.Qt import QtCore, QtGui

def main():
    win = pg.plot()
    win.setWindowTitle("Order Book")
    x = np.arange(1000)
    y = np.random.normal(size=1000).cumsum()
    win.plot(x, y, pen=pg.mkPen(width=2))
    win.show()
    return win

if __name__ == "__main__":
    app = pg.mkQApp("Order Book")
    win = main()
    pg.exec()   
    


