import sys, os
from PySide2 import QtWidgets, QtCore
from LoginDlg import LoginDlg
from PmWidget import PmWidget
from SmWidget import SmWidget

if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    dlg = LoginDlg()# do login
    if dlg.exec_() != dlg.Accepted:
        sys.exit(0)
    try:
        widget = QtWidgets.QTabWidget()
        widget.setWindowTitle("UI")
        widget.showMaximized()
        widget.addTab(PmWidget(), "Deploy Management")
        widget.addTab(SmWidget(True), "Server Management")
        with open("./qss/TabWidget.qss", "rb") as f:
            widget.setStyleSheet(f.read().decode())
    except Exception as e:
        print(e)
    sys.exit(app.exec_())