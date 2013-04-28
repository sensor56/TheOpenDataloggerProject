# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/home/xavier/www/mon_arduino/python_avec_arduino/mes_pyQt/tuto_pyqt_qtcpsocket_serveur_ajax_terminal_analyse_SD/tuto_pyqt_qtcpsocket_serveur_ajax_terminal_analyse_SD.ui'
#
# Created: Wed Apr 10 18:36:43 2013
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName(_fromUtf8("Form"))
        Form.resize(707, 624)
        self.labelIPServeurDistant = QtGui.QLabel(Form)
        self.labelIPServeurDistant.setGeometry(QtCore.QRect(5, 45, 136, 16))
        self.labelIPServeurDistant.setObjectName(_fromUtf8("labelIPServeurDistant"))
        self.textEditReceptionReseau = QtGui.QTextEdit(Form)
        self.textEditReceptionReseau.setGeometry(QtCore.QRect(210, 15, 486, 151))
        self.textEditReceptionReseau.setLineWrapMode(QtGui.QTextEdit.NoWrap)
        self.textEditReceptionReseau.setObjectName(_fromUtf8("textEditReceptionReseau"))
        self.labelReceptionReseau = QtGui.QLabel(Form)
        self.labelReceptionReseau.setGeometry(QtCore.QRect(210, 0, 191, 16))
        self.labelReceptionReseau.setObjectName(_fromUtf8("labelReceptionReseau"))
        self.lineEditIPClientDistant = QtGui.QLineEdit(Form)
        self.lineEditIPClientDistant.setGeometry(QtCore.QRect(5, 60, 126, 23))
        self.lineEditIPClientDistant.setObjectName(_fromUtf8("lineEditIPClientDistant"))
        self.labelPortTCP = QtGui.QLabel(Form)
        self.labelPortTCP.setGeometry(QtCore.QRect(5, 85, 101, 16))
        self.labelPortTCP.setObjectName(_fromUtf8("labelPortTCP"))
        self.lineEditPortTCP = QtGui.QLineEdit(Form)
        self.lineEditPortTCP.setGeometry(QtCore.QRect(5, 100, 101, 23))
        self.lineEditPortTCP.setObjectName(_fromUtf8("lineEditPortTCP"))
        self.pushButtonPing = QtGui.QPushButton(Form)
        self.pushButtonPing.setGeometry(QtCore.QRect(135, 50, 61, 36))
        self.pushButtonPing.setObjectName(_fromUtf8("pushButtonPing"))
        self.labelIPServeurLocal = QtGui.QLabel(Form)
        self.labelIPServeurLocal.setGeometry(QtCore.QRect(5, 5, 136, 16))
        self.labelIPServeurLocal.setObjectName(_fromUtf8("labelIPServeurLocal"))
        self.lineEditIPServeurLocal = QtGui.QLineEdit(Form)
        self.lineEditIPServeurLocal.setGeometry(QtCore.QRect(5, 20, 126, 23))
        self.lineEditIPServeurLocal.setObjectName(_fromUtf8("lineEditIPServeurLocal"))
        self.pushButtonInitServeur = QtGui.QPushButton(Form)
        self.pushButtonInitServeur.setGeometry(QtCore.QRect(130, 90, 66, 36))
        self.pushButtonInitServeur.setObjectName(_fromUtf8("pushButtonInitServeur"))
        self.labelEnvoiReseau = QtGui.QLabel(Form)
        self.labelEnvoiReseau.setGeometry(QtCore.QRect(210, 175, 191, 16))
        self.labelEnvoiReseau.setObjectName(_fromUtf8("labelEnvoiReseau"))
        self.textEditEnvoiReseau = QtGui.QTextEdit(Form)
        self.textEditEnvoiReseau.setGeometry(QtCore.QRect(210, 190, 481, 131))
        self.textEditEnvoiReseau.setLineWrapMode(QtGui.QTextEdit.NoWrap)
        self.textEditEnvoiReseau.setObjectName(_fromUtf8("textEditEnvoiReseau"))
        self.pushButtonEnregistrer = QtGui.QPushButton(Form)
        self.pushButtonEnregistrer.setGeometry(QtCore.QRect(395, 400, 86, 21))
        self.pushButtonEnregistrer.setObjectName(_fromUtf8("pushButtonEnregistrer"))
        self.lineEditCheminRep = QtGui.QLineEdit(Form)
        self.lineEditCheminRep.setGeometry(QtCore.QRect(215, 345, 371, 23))
        self.lineEditCheminRep.setObjectName(_fromUtf8("lineEditCheminRep"))
        self.pushButtonOuvrir = QtGui.QPushButton(Form)
        self.pushButtonOuvrir.setGeometry(QtCore.QRect(305, 400, 85, 21))
        self.pushButtonOuvrir.setObjectName(_fromUtf8("pushButtonOuvrir"))
        self.lineEditData = QtGui.QLineEdit(Form)
        self.lineEditData.setGeometry(QtCore.QRect(490, 430, 161, 23))
        self.lineEditData.setObjectName(_fromUtf8("lineEditData"))
        self.pushButtonOuvrirRep = QtGui.QPushButton(Form)
        self.pushButtonOuvrirRep.setGeometry(QtCore.QRect(590, 340, 61, 27))
        self.pushButtonOuvrirRep.setObjectName(_fromUtf8("pushButtonOuvrirRep"))
        self.pushButtonNouveau = QtGui.QPushButton(Form)
        self.pushButtonNouveau.setGeometry(QtCore.QRect(215, 400, 86, 21))
        self.pushButtonNouveau.setObjectName(_fromUtf8("pushButtonNouveau"))
        self.line_5 = QtGui.QFrame(Form)
        self.line_5.setGeometry(QtCore.QRect(210, 325, 436, 16))
        self.line_5.setFrameShape(QtGui.QFrame.HLine)
        self.line_5.setFrameShadow(QtGui.QFrame.Sunken)
        self.line_5.setObjectName(_fromUtf8("line_5"))
        self.textEdit = QtGui.QTextEdit(Form)
        self.textEdit.setGeometry(QtCore.QRect(215, 425, 266, 191))
        self.textEdit.setLineWrapMode(QtGui.QTextEdit.NoWrap)
        self.textEdit.setObjectName(_fromUtf8("textEdit"))
        self.lineEditChemin = QtGui.QLineEdit(Form)
        self.lineEditChemin.setGeometry(QtCore.QRect(215, 377, 436, 21))
        self.lineEditChemin.setObjectName(_fromUtf8("lineEditChemin"))
        self.pushButtonAjouterData = QtGui.QPushButton(Form)
        self.pushButtonAjouterData.setGeometry(QtCore.QRect(490, 460, 136, 27))
        self.pushButtonAjouterData.setObjectName(_fromUtf8("pushButtonAjouterData"))

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        Form.setWindowTitle(QtGui.QApplication.translate("Form", "PyQt + QTcpSocket : GUI Base Terminal Serveur TCP + SD", None, QtGui.QApplication.UnicodeUTF8))
        self.labelIPServeurDistant.setText(QtGui.QApplication.translate("Form", "Adresse client distant : ", None, QtGui.QApplication.UnicodeUTF8))
        self.labelReceptionReseau.setText(QtGui.QApplication.translate("Form", "Chaines recues par le serveur :", None, QtGui.QApplication.UnicodeUTF8))
        self.lineEditIPClientDistant.setText(QtGui.QApplication.translate("Form", "192.168.1.1", None, QtGui.QApplication.UnicodeUTF8))
        self.labelPortTCP.setText(QtGui.QApplication.translate("Form", "Port à utiliser :", None, QtGui.QApplication.UnicodeUTF8))
        self.lineEditPortTCP.setText(QtGui.QApplication.translate("Form", "8080", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButtonPing.setText(QtGui.QApplication.translate("Form", "Ping!", None, QtGui.QApplication.UnicodeUTF8))
        self.labelIPServeurLocal.setText(QtGui.QApplication.translate("Form", "Adresse serveur local : ", None, QtGui.QApplication.UnicodeUTF8))
        self.lineEditIPServeurLocal.setText(QtGui.QApplication.translate("Form", "192.168.1.14", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButtonInitServeur.setText(QtGui.QApplication.translate("Form", "Init serv.", None, QtGui.QApplication.UnicodeUTF8))
        self.labelEnvoiReseau.setText(QtGui.QApplication.translate("Form", "Chaines envoyées vers le serveur :", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButtonEnregistrer.setText(QtGui.QApplication.translate("Form", "Enregistrer", None, QtGui.QApplication.UnicodeUTF8))
        self.lineEditCheminRep.setText(QtGui.QApplication.translate("Form", "/home/xavier/Bureau/data/texte", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButtonOuvrir.setText(QtGui.QApplication.translate("Form", "Ouvrir", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButtonOuvrirRep.setText(QtGui.QApplication.translate("Form", "Ouvrir", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButtonNouveau.setText(QtGui.QApplication.translate("Form", "Nouveau", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButtonAjouterData.setText(QtGui.QApplication.translate("Form", "Ajouter ligne au fichier", None, QtGui.QApplication.UnicodeUTF8))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    Form = QtGui.QWidget()
    ui = Ui_Form()
    ui.setupUi(Form)
    Form.show()
    sys.exit(app.exec_())

