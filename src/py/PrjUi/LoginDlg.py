from PySide2 import QtWidgets, QtCore, QtGui
from RestfulApiClient import RestfulApiClient
from DataModel import DataModel


class LoginDlg(QtWidgets.QDialog):
    """description of class"""
    def __init__(self):
        super().__init__()
        self.failedTimes = 0
        self.__initUi()
        self.onClieckLogin()


    def __initUi(self):
        try:
            self.setWindowFlags(QtCore.Qt.FramelessWindowHint|QtCore.Qt.Dialog)
            self.pmp = QtGui.QPixmap("./image/login_back-1.jpg")
            self.setFixedWidth(480)
            self.setFixedHeight(320)
            hLayout = QtWidgets.QHBoxLayout()
            hLayout.setSpacing(0)
            label = QtWidgets.QLabel("aods login")
            label.setAlignment(QtCore.Qt.AlignCenter)
  
            hLayout.addWidget(label)
            self.closeBtn = QtWidgets.QPushButton(QtGui.QIcon("./image/close.png"), "")
            self.closeBtn.setFixedHeight(24)
            self.closeBtn.setFixedWidth(24)
            self.closeBtn.clicked.connect(self.reject)
            hLayout.addWidget(self.closeBtn)
           
            gLayout = QtWidgets.QGridLayout()
            gLayout.setSpacing(10)
            self.editUsr = QtWidgets.QLineEdit()
            self.editUsr.setPlaceholderText("user")
            gLayout.addWidget(self.editUsr, 0,1, QtCore.Qt.AlignCenter)

            self.editPassword = QtWidgets.QLineEdit()
            self.editPassword.setEchoMode(QtWidgets.QLineEdit.Password)
            self.editPassword.setPlaceholderText("password")
            self.editPassword.returnPressed.connect(self.onClieckLogin)
            gLayout.addWidget(self.editPassword, 1,1, QtCore.Qt.AlignCenter)

            self.labelToolTip = QtWidgets.QLabel()
            self.labelToolTip.hide()
            gLayout.addWidget(self.labelToolTip, 2,1, QtCore.Qt.AlignCenter)
  
            btnLogin = QtWidgets.QPushButton("login")
            btnLogin.clicked.connect(self.onClieckLogin)
            gLayout.addWidget(btnLogin, 3,1, QtCore.Qt.AlignCenter)
            vLayout = QtWidgets.QVBoxLayout()
            vLayout.addLayout(hLayout)
            vLayout.addSpacerItem(QtWidgets.QSpacerItem(self.width(), self.height() / 3))
            vLayout.addLayout(gLayout)
            vLayout.addSpacerItem(QtWidgets.QSpacerItem(self.width(), 38))
            self.setLayout(vLayout)

            with open("./qss/login.qss", 'rb') as f:
                self.setStyleSheet(f.read().decode())

        except Exception as e:
            import traceback
            traceback.print_exc()
            print(e)


    def paintEvent(self, event):
        painter = QtGui.QPainter(self)
        painter.drawPixmap(0, 0, self.pmp)
       

    def onClieckLogin(self):
        rlt = RestfulApiClient().Login(self.editUsr.text(),self.editPassword.text())
        if not rlt or rlt["login_result"]["result"] != "success":
            self.failedTimes += 1
            self.labelToolTip.show()
            self.labelToolTip.setStyleSheet("color:red;font-size:18px")
            self.labelToolTip.setText("login failed, please input again")
            if self.failedTimes > 300: return self.reject()
        else:
            DataModel.Instance().UpdateUser(rlt["login_result"])
            return self.accept()
