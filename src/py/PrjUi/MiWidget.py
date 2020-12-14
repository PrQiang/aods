import traceback, math
from UpdateDetailDlg import UpdateDetailDlg
from PySide2 import QtCore, QtWidgets, QtGui

PI = 3.1415926

class VersionGraphicItem(QtWidgets.QGraphicsItem):
    def __init__(self, prj, module, v, dt, gid, hash, url, detail, rc):
        super().__init__()
        self.ver, self.dt, self.rc, self.prj, self.module, self.gid, self.hash, self.url, self.detail = v, dt, rc, prj, module, gid, hash, url, detail
        self.setFlag(QtWidgets.QGraphicsItem.ItemIsSelectable)
        self.setCursor(QtCore.Qt.PointingHandCursor)
        self.setToolTip("version: %s\r\nurl: %s\r\nhash: %s\r\ngroup: %s\r\nlast update time: %s\r\ndetail: \r\n%s"%(v, url, hash, gid, dt, detail))


    def boundingRect(self):
        return self.rc


    def paint(self, painter, option, widget):
        try:
            painter.setBrush(QtGui.QBrush(QtGui.QColor(0x1E, 0x9F, 0xFF)))
            painter.drawRoundedRect(self.boundingRect(), 5, 5)
            painter.drawText(self.boundingRect(), QtCore.Qt.AlignCenter, "%s"%(self.ver))
        except Exception as e:
            print(e)


    def mousePressEvent (self, event):
        ud = UpdateDetailDlg(self.prj, self.module, self.ver)
        ud.showMaximized()
        ud.exec_()


class ArrowLineItem(QtWidgets.QGraphicsItem):
    def __init__(self, beginItem, endItem):
        self.beginItem, self.endItem = beginItem, endItem
        super().__init__()
        self.pen = QtGui.QPen()
        self.pen.setColor(QtCore.Qt.blue)
        self.pen.setWidth(2)


    def boundingRect(self):
        # simple deal
        extra = (self.pen.width() + 20) / 2 + 16
        line = self.__calc2SpecialRectCentorLineIntersect()
        rc = QtCore.QRect(line[0], QtCore.QSize(line[1].x() - line[0].x(), 1))
        rc.normalized().adjust(-extra, -extra, extra, extra)
        return rc


    def __calc2SpecialRectCentorLineIntersect(self):
        try:            
            pt1, pt2, w1, w2, h1, h2 = self.beginItem.pos(), self.endItem.pos(), self.beginItem.boundingRect().width(), self.endItem.boundingRect().width(), self.beginItem.boundingRect().height(), self.endItem.boundingRect().height()
            pt1 += QtCore.QPoint(w1 / 2, h1 / 2)
            pt2 += QtCore.QPoint(w2 / 2, h2 / 2)
            if pt1.y() == pt2.y(): # at the same row
                if pt1.x() < pt2.x(): # first rect is at the left
                    return [QtCore.QPoint(pt1.x() + w1 / 2, pt2.y()), QtCore.QPoint(pt2.x() - w2 / 2, pt2.y())]
                else: # first rect is at the right
                    return [QtCore.QPoint(pt1.x() - w1 / 2, pt2.y()), QtCore.QPoint(pt2.x() + w2 / 2, pt2.y())]
            else: # make sure pt1.x() == pt2.x()                
                return [QtCore.QPoint(pt1.x(), pt1.y() + h1 / 2), QtCore.QPoint(pt2.x(), pt2.y() - h2 / 2)]
        except Exception as e:
            traceback.print_exc()
            return []


    def paint(self, painter, option, widget):
        try:
            line = self.__calc2SpecialRectCentorLineIntersect()
            line = QtCore.QLine(line[0], line[1])
            painter.setPen(self.pen)
            painter.drawLine(line)
            pt1 = line.p1() # draw arrow
            angle, arrowLen, arrowAngle = math.atan2(line.dy(), line.dx()), 16, PI / 6
            pt2 = pt1 + QtCore.QPoint(math.cos(angle + arrowAngle) * arrowLen, math.sin(angle + arrowAngle) * arrowLen)
            pt3 = pt1 + QtCore.QPoint(math.cos(angle - arrowAngle) * arrowLen, math.sin(angle - arrowAngle) * arrowLen)
            painter.drawLine(pt1, pt2)
            painter.drawLine(pt1, pt3)
        except Exception as e:
            traceback.print_exc()


class MiWidget(QtWidgets.QGraphicsView):
    """description of class"""
    def __init__(self):
        super().__init__()
        self.gs, self.items = QtWidgets.QGraphicsScene(), []
        self.setScene(self.gs)


    def Update(self, prj, module, data):
        try:
            for item in self.items:
                self.gs.removeItem(item)
            self.items.clear()
            data.reverse()            
            items, index, sz, lineNum, bw, span = [], 0, 96, 4, 32, 280
            for (v, (dt, gid, hash, url, detail)) in data[:12]:
                item = VersionGraphicItem(prj, module, v, dt, gid, hash, url, detail, QtCore.QRect(0, 0, sz, sz))
                self.gs.addItem(item)
                self.items.append(item)
                if (int(index / lineNum) % 2) == 0: # even row
                    x, y  = int(index % lineNum) * span + bw, int(index / lineNum) * span + bw
                else:
                    x, y  = (lineNum - int(index % lineNum) - 1) * span + bw, int(index / 4) * span + bw
                item.setPos(x, y)
                items.append(item)
                items = items[-2:]
                if len(items) == 2: 
                    item = ArrowLineItem(items[0], items[1])
                    self.items.append(item)
                    self.gs.addItem(item)
                index += 1
            self.gs.update()
            self.update()
        except Exception as e:
            print(e)


    def resizeEvent(self, event):
        try:
            self.gs.setSceneRect(0, 0, event.size().width(), event.size().height())
            self.gs.setBackgroundBrush(QtGui.QBrush(QtGui.QPixmap("./image/login_back-1.jpg").scaled(event.size(), mode=QtCore.Qt.SmoothTransformation)))
            self.gs.update()
        except Exception as e:
            print(e)
        return super().resizeEvent(event)      
        