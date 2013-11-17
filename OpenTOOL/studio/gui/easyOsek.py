from PyQt4 import QtCore, QtGui
from PyQt4.QtGui import *
from PyQt4.QtCore import *
import sys,os
from easyCan import easyCanGui

__all__ = ['easyOsek']

class easyDockWidget(QDockWidget):
    isClosed = False
    def __init__(self,title,parent=None):
        QDockWidget.__init__(self,title,parent)
        self.setAllowedAreas(QtCore.Qt.LeftDockWidgetArea|QtCore.Qt.RightDockWidgetArea)  
        #self.setFeatures(QDockWidget.DockWidgetClosable|QDockWidget.DockWidgetMovable)
    def closeEvent(self,event):
        self.isClosed = True
class easyOsekGui(QMainWindow):
    easyCan = None
    def __init__(self):
        QMainWindow.__init__(self, None)
        self.setWindowTitle('easy Osek Studio( parai@foxmail.com ^_^)');
        self.resize(800,600)
        self.creMenu()
        self.creStatusBar()
    def creMenu(self):
        # File
        tMenu=self.menuBar().addMenu(self.tr('File'))
        ## Open Ctrl+O  
        sItem=QAction(self.tr('Open'),self) 
        sItem.setShortcut('Ctrl+O'); 
        sItem.setStatusTip('Open a openOSEK configure file.')
        self.connect(sItem,SIGNAL('triggered()'),self.mOpen)  
        tMenu.addAction(sItem)  
        ## Save Ctrl+S
        sItem=QAction(self.tr('Save'),self) 
        sItem.setShortcut('Ctrl+S'); 
        sItem.setStatusTip('Save the openOSEK configure file.')
        self.connect(sItem,SIGNAL('triggered()'),self.mSave)  
        tMenu.addAction(sItem)  
        # Tool
        tMenu=self.menuBar().addMenu(self.tr('Tool'))
        ## easyCan
        sItem=QAction(self.tr('easyCan'),self) 
        self.connect(sItem,SIGNAL('triggered()'),self.measyCan) 
        sItem.setStatusTip('Open easyCan console.') 
        tMenu.addAction(sItem)  
    def mOpen(self):
        pass
    def mSave(self):
        pass
    def measyCan(self):
        if(self.easyCan==None or self.easyCan.isClosed==True):
            self.easyCan = easyDockWidget('easyCan', self)  
            self.easyCan.setWidget(easyCanGui())  
            self.addDockWidget(QtCore.Qt.RightDockWidgetArea, self.easyCan)
        else:
            print('easyCan already started.')
    def creStatusBar(self):
        self.statusBar = QStatusBar()
        self.setStatusBar(self.statusBar)
        self.statusBar.showMessage('easy Osek Studio Platform',0)
        
def easyOsek():
    qtApp = QtGui.QApplication(sys.argv)
    if(os.name == 'nt'):
        qtApp.setFont(QFont('Consolas', 12)) 
    elif(os.name == 'posix'):
        qtApp.setFont(QFont('Monospace', 12))
    else:
        print('unKnown platform.')
    qtGui = easyOsekGui()
    qtGui.show()
    qtApp.exec_()