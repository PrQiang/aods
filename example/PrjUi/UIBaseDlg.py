import traceback
from PySide2 import QtWidgets, QtCore, QtGui

class UIBaseDlg(QtWidgets.QDialog):
    def __init__(self, title = '', style = None, par = None):
        super().__init__()
        self.setModal(True)
        self.__initHeader(title)
        try:
            with open(style or "./qss/uidlg.qss", 'rb') as f:
                self.setStyleSheet(f.read().decode())
        except Exception as e:
            traceback.print_exc()


    def __initHeader(self, title):
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint|QtCore.Qt.Dialog)
        self.pmp = QtGui.QPixmap("./image/bg.jpg")
        self.hLayout = QtWidgets.QHBoxLayout()
        self.hLayout.setSpacing(0)
        label = QtWidgets.QLabel(title)
        label.setAlignment(QtCore.Qt.AlignCenter)
  
        self.hLayout.addWidget(label)
        self.closeBtn = QtWidgets.QPushButton(QtGui.QIcon("./image/close.png"), "")
        self.closeBtn.setFixedHeight(24)
        self.closeBtn.setFixedWidth(24)
        self.closeBtn.clicked.connect(self.reject)
        self.hLayout.addWidget(self.closeBtn)


    def paintEvent(self, event):
        painter = QtGui.QPainter(self)
        painter.drawPixmap(0, 0, self.pmp.scaled(self.size(), QtCore.Qt.IgnoreAspectRatio, QtCore.Qt.SmoothTransformation))