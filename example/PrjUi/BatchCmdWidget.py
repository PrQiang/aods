import traceback, ParaMgr
from BatchCmdDlg import BatchCmdDlg
from BatchUploadDlg import BatchUploadDlg
from PySide2 import QtCore, QtWidgets, QtGui
from sshShell import sshShell

class BatchCmdWidget(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        self.initUi()  


    def initUi(self):
        try:
            self.searchLe = QtWidgets.QLineEdit()
            self.searchLe.textChanged.connect(self.search)
            self.searchBtn = QtWidgets.QPushButton("search")
            self.searchBtn.setFixedWidth(44)
            self.firstLineLay = QtWidgets.QHBoxLayout()
            self.firstLineLay.addSpacerItem(QtWidgets.QSpacerItem(600, 30))
            self.firstLineLay.addWidget(QtWidgets.QLabel("search"))
            self.firstLineLay.addWidget(self.searchLe)
            self.btnBatchCmd = QtWidgets.QPushButton("batch cmd")
            self.btnBatchCmd.clicked.connect(self.onBatchCmdClicked)
            self.firstLineLay.addWidget(self.btnBatchCmd)
            self.btnBatchUpload = QtWidgets.QPushButton("batch upload")
            self.btnBatchUpload.clicked.connect(self.onBatchUploadClicked)            
            self.firstLineLay.addWidget(self.btnBatchUpload)

            w = QtWidgets.QWidget()
            w.setLayout(self.firstLineLay)
            w.setFixedHeight(44)
            names = ["tag","ip", "location", "system"]
            self.tableWidget = QtWidgets.QTableWidget(0, len(names))
            for i in range(self.tableWidget.columnCount()):
                item = QtWidgets.QTableWidgetItem(names[i])
                self.tableWidget.setHorizontalHeaderItem(i, item)
            self.tableWidget.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)
            self.tableWidget.horizontalHeader().setSectionResizeMode(0, QtWidgets.QHeaderView.Fixed)
            self.tableWidget.setSelectionBehavior (QtWidgets.QAbstractItemView.SelectRows)
            self.vLay = QtWidgets.QVBoxLayout(self)
            self.vLay.addWidget(w)
            self.vLay.addWidget(self.tableWidget)
            self.setLayout(self.vLay)
            self.Update()
        except Exception as e:
            traceback.print_exc()


    def walkBack(self, para, parent):
        if para.get("isFolder") == True:return None
        self.appendItem(para.get("tag"), para)
        return None


    def Update(self):
        ParaMgr.Instance.walk(self.walkBack)


    def appendItem(self, tag, para):
        self.tableWidget.insertRow(self.tableWidget.rowCount())
        row = self.tableWidget.rowCount() - 1

        item = QtWidgets.QTableWidgetItem(tag)
        item.setData(1, para)
        item.setFlags(~QtCore.Qt.ItemIsEditable)
        self.tableWidget.setItem(row, 0, item)
        
        item = QtWidgets.QTableWidgetItem(para.get("ip", ''))
        item.setFlags(~QtCore.Qt.ItemIsEditable)
        self.tableWidget.setItem(row, 1, item)

        item = QtWidgets.QTableWidgetItem(para.get("location", ''))
        item.setFlags(~QtCore.Qt.ItemIsEditable)
        self.tableWidget.setItem(row, 2, item)

        item = QtWidgets.QTableWidgetItem(para.get("system", ''))
        item.setFlags(~QtCore.Qt.ItemIsEditable)
        self.tableWidget.setItem(row, 3, item)


    def search(self, searchText):
        try:
            showRows = []
            for item in self.tableWidget.findItems(searchText, QtCore.Qt.MatchContains&(~QtCore.Qt.MatchCaseSensitive)):
                if item.row() not in showRows: showRows.append(item.row())
            for i in range(self.tableWidget.rowCount()):
                if i not in showRows: self.tableWidget.hideRow(i)
                else:self.tableWidget.showRow(i)
        except Exception as e:
            traceback.print_exc()


    def getBatchSelectedShell(self):
        batchList = []
        for item in self.tableWidget.selectedItems():
            if item.column() != 0:continue  
            if self.tableWidget.isRowHidden(item.row()):continue
            para = item.data(1)
            if para:
                batchList.append(sshShell("%s(%s%s)"%(item.text(),para.get("location"), para.get("ip")), para.get("addr"), para.get("user"), para.get("pwd"), para.get("pkey"), para.get("key_filename"), 'utf8'))
        return batchList


    def onBatchCmdClicked(self):
        try:
            BatchCmdDlg(self.getBatchSelectedShell(), self).exec()
        except Exception as e:
            traceback.print_exc()


    def onBatchUploadClicked(self):
        BatchUploadDlg(self.getBatchSelectedShell(), self).exec()