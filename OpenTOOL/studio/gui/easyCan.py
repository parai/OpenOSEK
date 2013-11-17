from PyQt4 import QtCore, QtGui
from PyQt4.QtGui import *
from PyQt4.QtCore import *
import sys
import threading
import socket
import time

__all__ = ['easyCanGui']

easyCanPortR = []
class easyCanMsgTbl(QTableWidget):
    time = None
    def __init__(self,parent=None):  
        super(QTableWidget,self).__init__(parent)  
        self.setColumnCount(5)  
        self.setRowCount(0)  
        self.setHorizontalHeaderLabels(QStringList(['Time(ms)','Id','Length','Data','Port']))
        self.setMinimumWidth(800); 
        self.setColumnWidth(3,400)
    def forwardCanMsg(self,msg):
        """Forward msg received from one node to others connected to this server, exclude address."""
        global easyCanPortR
        # get Port
        portR = (ord(msg[13])<<24)+(ord(msg[14])<<16)+(ord(msg[15])<<8)+(ord(msg[16]))
        flag = False
        for p in easyCanPortR:
            if(p == portR):
                flag = True
        if(flag == False):
            easyCanPortR.append(portR)
        for p in easyCanPortR:
            if(p != portR):
                try:
                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    sock.connect(('127.0.0.1', p))  
                    sock.send(msg)
                    sock.close()
                except:
                    print "Error when forward to ",p,'!'
                    easyCanPortR.remove(p);
                    sock.close()
                    continue
    def addCanMsg(self,msg=None):
        if(msg != None and len(msg) == 17):
            #get CanID
            canid =  (ord(msg[0])<<24)+(ord(msg[1])<<16)+(ord(msg[2])<<8)+(ord(msg[3]))
            # get Port
            port = (ord(msg[13])<<24)+(ord(msg[14])<<16)+(ord(msg[15])<<8)+(ord(msg[16]))
            dlc = ord(msg[4])
            cstr = '['
            for i in range(0,8):
                cstr += '0x%-2x,'%(ord(msg[5+i]))
            cstr += ']'
            if(self.time == None):
                recTime = 0
                self.time = time.time()
            else:
                recTime = (time.time() - self.time)*1000
                self.time = time.time()
            # ===================
            index = self.rowCount()
            self.setRowCount(self.rowCount()+1) 
            item0 = QTableWidgetItem(self.tr('%s'%(round(recTime,2))))
            item0.setFlags(Qt.ItemIsSelectable|Qt.ItemIsEnabled)
            item1 = QTableWidgetItem(self.tr('%s'%(canid)))
            item1.setFlags(Qt.ItemIsSelectable|Qt.ItemIsEnabled)
            item2 = QTableWidgetItem(self.tr('%s'%(dlc)))
            item2.setFlags(Qt.ItemIsSelectable|Qt.ItemIsEnabled)
            item3 = QTableWidgetItem(self.tr('%s'%(cstr)))
            item3.setFlags(Qt.ItemIsSelectable|Qt.ItemIsEnabled)
            item4 = QTableWidgetItem(self.tr('%s'%(port)))
            item4.setFlags(Qt.ItemIsSelectable|Qt.ItemIsEnabled)
            self.setItem(index,0,item0)    
            self.setItem(index,1,item1)  
            self.setItem(index,2,item2)  
            self.setItem(index,3,item3)  
            self.setItem(index,4,item4) 

class easyCanMsgTree(QTreeWidget):
    def __init__(self,parent=None):  
        super(QTreeWidget,self).__init__(parent)
        self.setHeaderLabel('easyCanMessage')
        self.addTopLevelItem(QTreeWidgetItem(QStringList('Id')))
        self.addTopLevelItem(QTreeWidgetItem(QStringList('Port')))
        self.setMaximumWidth(400);
        self.setMinimumWidth(200); 

class easyCanGui(QMainWindow,threading.Thread):
    easyTbl = None
    easyTree = None
    def __init__(self):
        QMainWindow.__init__(self, None)
        threading.Thread.__init__(self)
        #self.creToolbar()
        self.creGui()
        self.start()
    def run(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
        sock.bind(('127.0.0.1', 8000))  
        print 'Listen on 127.0.0.1:8000'
        sock.listen(32)  
        while True:  
            connection,address = sock.accept()  
            try:  
                connection.settimeout(5)  
                msg = connection.recv(1024) 
                self.easyTbl.addCanMsg(msg)   
                self.easyTbl.forwardCanMsg(msg)        
            except socket.timeout:  
                continue  
            connection.close()
    def creToolbar(self):
        toolbar = self.addToolBar('MainToolBar')
        tAction = QtGui.QAction(QtGui.QIcon('icons/tbOpen.png'), 'Open', self)
        tAction.setStatusTip('Open a openOSEK configure file.')  
        self.connect(tAction,SIGNAL('triggered()'),self.mOpen)    
        toolbar.addAction(tAction)
        tAction = QtGui.QAction(QtGui.QIcon('icons/tbSave.png'), 'Save', self)
        tAction.setStatusTip('Save the openOSEK configure file.')       
        toolbar.addAction(tAction)
        toolbar.setMovable(False)
        toolbar.addSeparator()
    def creGui(self):
        self.easyTree = easyCanMsgTree()
        self.easyTbl = easyCanMsgTbl()
        qSplitter = QSplitter(Qt.Horizontal,self)
        qSplitter.insertWidget(0,self.easyTree)
        qSplitter.insertWidget(1,self.easyTbl)
        self.setCentralWidget(qSplitter)
    def mOpen(self):
        pass
    def mSave(self):
        pass