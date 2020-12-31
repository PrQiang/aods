import traceback, threadpool
from PySide2 import QtCore, QtWidgets, QtGui

class ShellTableWidget(QtWidgets.QTableWidget):
    def __init__(self, backColor, mode = QtWidgets.QHeaderView.ResizeToContents):
        super().__init__()
        self.backColor = backColor
        self.horizontalHeader().setSectionResizeMode(mode)
        self.verticalHeader().setSectionResizeMode(mode)
        self.setColumnCount(4)
        self.items = []


    def appendItem(self, text):
        row, col = int(len(self.items)/self.columnCount()), len(self.items)%self.columnCount()
        if row + 1 > self.rowCount():
            self.insertRow(row)
        item = QtWidgets.QTableWidgetItem(text)
        #item.setBackground(self.backColor)
        item.setToolTip(text)
        self.setItem(row, col, item)
        self.items.append(text)


    def clear(self):
        self.items.clear()
        super().clear()



class BatchCmdDlg(QtWidgets.QDialog):
    signalResult = QtCore.Signal(bool, str)

    def __init__(self, batchItems, parent):
        super().__init__(parent)
        self.initUi(batchItems)


    def initUi(self, batchItems):
        self.signalResult.connect(self.doResult)
        self.editCmd = QtWidgets.QTextEdit()
        self.editCmd.setPlaceholderText("Please input commands here")
        btn = QtWidgets.QPushButton("run")
        hLay = QtWidgets.QHBoxLayout()
        hLay.addSpacerItem(QtWidgets.QSpacerItem(100, 20))
        hLay.addWidget(self.editCmd)
        self.editSc = QtWidgets.QTextEdit()
        self.editSc.setPlaceholderText("Please input success condition after doing commands")
        hLay.addWidget(self.editSc)
        hLay.addWidget(btn)
        btn.clicked.connect(self.onRunClicked)
        lineWidget = QtWidgets.QWidget()
        lineWidget.setLayout(hLay)
        lineWidget.setMaximumHeight(60)
        self.tabWidget = QtWidgets.QTabWidget()
        self.widgetDoing = ShellTableWidget(QtGui.QColor(220,220,220),QtWidgets.QHeaderView.Stretch)
        self.tabWidget.addTab(self.widgetDoing, "todo")
        self.widgetSuc = ShellTableWidget(QtGui.QColor(144,238,144))
        self.tabWidget.addTab(self.widgetSuc, "matched")
        self.widgetFai = ShellTableWidget(QtGui.QColor(225,120,120))
        self.tabWidget.addTab(self.widgetFai, "not match")
        vLay = QtWidgets.QVBoxLayout()
        vLay.addWidget(lineWidget)
        vLay.addWidget(self.tabWidget)
        self.setLayout(vLay)
        self.labelShells = batchItems
        for shell in self.labelShells:self.widgetDoing.appendItem(shell.Name())
        self.setModal(True)
        self.showMaximized()
        with open("./qss/TabWidget.qss", "rb") as f:
            self.setStyleSheet(f.read().decode())


    def doResult(self, isSuc, result):
        try:
            if isSuc: self.widgetSuc.appendItem(result)
            else: self.widgetFai.appendItem(result)
        except Exception as e:
            traceback.print_exc()


    def request(self, cmd, matchCondition, labelShell):
        try:
            result = labelShell.Excute(cmd)            
            self.signalResult.emit(matchCondition in result, "%s\r\n%s"%(labelShell.Name(), result))
        except Exception as e:
            traceback.print_exc()


    def onRunClicked(self):
        self.widgetSuc.clear()
        self.widgetFai.clear()
        tp, cmdText, matchCondition = threadpool.ThreadPool(min(len(self.labelShells), 20)), self.editCmd.toPlainText(), self.editSc.toPlainText()
        for labelShell in self.labelShells:
            tp.putRequest(threadpool.WorkRequest(self.request, (cmdText, matchCondition, labelShell)))
        tp.wait()