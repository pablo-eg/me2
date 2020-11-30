# ------------------------------------------------------
# -------------------- mplwidget.py --------------------
# ------------------------------------------------------
from PyQt5.QtWidgets import*

from matplotlib.backends.backend_qt5agg import FigureCanvas

from matplotlib.figure import Figure


class MplWidget(QWidget):

    def __init__(self, parent = None):

        QWidget.__init__(self, parent)

        self.canvas = FigureCanvas(Figure(facecolor="#efebe7", edgecolor="#efebe7"))

        vertical_layout = QVBoxLayout()
        vertical_layout.addWidget(self.canvas)

        self.canvas.axes = self.canvas.figure.add_subplot(111, facecolor="#efebe7")
        self.setLayout(vertical_layout)
