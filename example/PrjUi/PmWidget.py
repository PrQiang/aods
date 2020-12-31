from PySide2 import QtCore, QtWidgets, QtGui
import traceback
from MiWidget import MiWidget
from RestfulApiClient import RestfulApiClient

class PmWidget(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.__initUi()


    def __initUi(self):
        try:
            # initialize the nav tree
            self.windowTitle = "UI"
            self.navTree = QtWidgets.QTreeWidget()
            self.navTree.setMaximumWidth(240)
            self.navTree.setHeaderHidden(True)
            self.miWidget = MiWidget()
            layout = QtWidgets.QHBoxLayout(self)
            layout.setSpacing(2)
            layout.addWidget(self.navTree)
            layout.addWidget(self.miWidget)
            with open("./qss/PmWidget.qss", "rb") as f:
                self.setStyleSheet(f.read().decode())
            self.__updateData()
            self.navTree.expandAll()
            self.navTree.itemClicked.connect(self.onClickItem)
        except Exception as e:
            print(e)


    def __updateData(self):
        try:
            self.navTree.clear()
            self.navTree.setHeaderLabel("")
            result = RestfulApiClient().QueryPms()
            if not result:return
            if result.get("query_pms_result", {}).get("result") != "success":return
    
            for p, ms in result["query_pms_result"]["detail"].items():
                parItem = QtWidgets.QTreeWidgetItem(self.navTree, [p, ])
                for m in ms:
                    childItem = QtWidgets.QTreeWidgetItem(parItem, [m, ])
        except Exception as e:
            print(e)


    def onClickItem(self, item, col):
        try:
            parItem = item.parent()
            if not parItem:return
            result = RestfulApiClient().QueryMi(parItem.text(0), item.text(0))
            if not result:return
            if result.get("query_mi_result", {}).get("result") != 'success':
                print(result)
                return
            self.miWidget.Update(parItem.text(0), item.text(0), result["query_mi_result"]["detail"])
        except Exception as e:
            print(e)