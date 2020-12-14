import traceback, ParaMgr
from PySide2 import QtCore, QtWidgets, QtGui
from sshWidget import sshWidget

class NavigationWidget(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
             
        # initial ssh and rdp navigation widget
        self.navTab = QtWidgets.QTabWidget(self)
        self.treeWidgetSsh = QtWidgets.QTreeWidget(self.navTab)
        self.SetTreeWidgetStyle(self.treeWidgetSsh)
        sshSearch = QtWidgets.QLineEdit()
        sshSearch.textChanged.connect(self.searchSshItems)
        vLay = QtWidgets.QVBoxLayout()
        vLay.addWidget(sshSearch)
        vLay.addWidget(self.treeWidgetSsh)
        sshWidget = QtWidgets.QWidget()
        sshWidget.setLayout(vLay)
        
        self.treeWidgetRdp = QtWidgets.QTreeWidget(self.navTab)
        self.SetTreeWidgetStyle(self.treeWidgetRdp)        
        rdpSearch = QtWidgets.QLineEdit()
        rdpSearch.textChanged.connect(self.searchRdpItems)
        vLay = QtWidgets.QVBoxLayout()
        vLay.addWidget(rdpSearch)
        vLay.addWidget(self.treeWidgetRdp)
        rdpWidget = QtWidgets.QWidget()
        rdpWidget.setLayout(vLay)        

        self.navTab.addTab(sshWidget, "ssh")
        self.navTab.addTab(rdpWidget, "rdp")
        self.navTab.setMaximumWidth(220)
        self.navTab.setTabPosition(QtWidgets.QTabWidget.South)

        self.tabWidget = QtWidgets.QTabWidget(self)
        self.tabWidget.setTabsClosable(True)
        self.tabWidget.tabCloseRequested.connect(self.loginOut)
        hLayout = QtWidgets.QHBoxLayout(self)
        hLayout.addWidget(self.navTab)
        hLayout.addWidget(self.tabWidget)
        self.treeWidgetSsh.itemDoubleClicked.connect(self.itemDbClickedSsh)
        self.Update()


    def SetTreeWidgetStyle(self, tabWidet):
        tabWidet.setHeaderHidden(True)
        tabWidet.setRootIsDecorated(True)
        tabWidet.setStyle(QtWidgets.QStyleFactory.create("windows"))

    
    def walkSsh(self, para, parent):
        return self.walkBack(para, parent, self.treeWidgetSsh, "ssh", "./centos.ico")


    def walkRdp(self, para, parent):
        return self.walkBack(para, parent, self.treeWidgetRdp, "rdp", "./windows.ico")


    def walkBack(self, para, parent, tree, protocol, iconFileName):
        try:
            if not para.get("isFolder") and para.get("protocol").lower() != protocol: return None
            item  = self.constructTreeItem(para.get("tag"), para, parent)
            if not parent:tree.addTopLevelItem(item)
            else: parent.addChild(item)
            item.setExpanded(True)
            if para.get("isFolder") is True:
                item.setIcon(0, QtGui.QIcon("./folder.ico"))
            else:
                item.setIcon(0, QtGui.QIcon(iconFileName))
            return item
        except Exception as e:
            traceback.print_exc()
        return None


    def Update(self):
        ParaMgr.Instance.walk(self.walkSsh)
        ParaMgr.Instance.walk(self.walkRdp)


    def constructTreeItem(self, name, property, parent):
        treeItem = QtWidgets.QTreeWidgetItem(parent)
        treeItem.setText(0, name)
        treeItem.setData(0, QtCore.Qt.WhatsThisRole, property)
        return treeItem


    def itemDbClickedSsh(self, item, column):
        try:
            p =  item.data(column, QtCore.Qt.WhatsThisRole)
            if not p.get("isFolder"):
                self.tabWidget.setCurrentIndex(self.tabWidget.addTab(sshWidget(p.get("addr"), p.get("user"), p.get("pwd"), p.get("pkey"), p.get("key_filename")), item.text(0)))
        except Exception as e:
            traceback.print_exc()


    def loginOut(self, index):
        tabWidget = self.tabWidget.widget(index)
        self.tabWidget.removeTab(index)
        tabWidget.destroy(True, True)


    def searchRdpItems(self, text):
        self.hideNotMatchItems(self.treeWidgetRdp, text)


    def searchSshItems(self, text):
        self.hideNotMatchItems(self.treeWidgetSsh, text)


    def hideNotMatchItems(self, tree, text):
        setedItems = []
        showItems = tree.findItems(text, QtCore.Qt.MatchContains | QtCore.Qt.MatchRecursive&(~QtCore.Qt.MatchCaseSensitive), 0)
        for item in tree.findItems("", QtCore.Qt.MatchContains | QtCore.Qt.MatchRecursive, 0):
            if item not in showItems:
                if item not in setedItems: 
                    item.setHidden(True)
                    item.setSelected(False)
            else:                
                item.setHidden(False)
                item.setSelected(True)
                setedItems.append(item)
                ip = item.parent()
                while ip:
                    setedItems.append(ip)
                    ip.setHidden(False)
                    ip = ip.parent()