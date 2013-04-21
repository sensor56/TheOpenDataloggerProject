#!/usr/bin/python
# -*- coding: utf-8 -*-

# par X. HINAULT - Fév 2013 - Tous droits réservés
# GPLv3 - www.mon-club-elec.fr

# modules a importer 
from PyQt4.QtGui import *
from PyQt4.QtCore import *  # inclut QTimer..

import os,sys

import time # temps
import serial # communication serie
import re # module pour analyse de chaîne avec expressions régulières

sys.path.append(os.getenv('HOME')+'/mes_libs_python') # le rép où se trouve la lib'

#from utils import * # importe librairie perso
import utils # importe librairie perso
from PyQt4.QtNetwork import * # module réseau Qt4

from pyqt_qtcpsocket_serveur_ajax_terminal_analyse_SD import * # fichier obtenu à partir QtDesigner et pyuic4

class myApp(QWidget, Ui_Form): # la classe reçoit le Qwidget principal ET la classe définie dans test.py obtenu avec pyuic4
	def __init__(self, parent=None):
		QWidget.__init__(self) # initialise le Qwidget principal 
		self.setupUi(parent) # Obligatoire 

		#Ici, personnalisez vos widgets si nécessaire
        
		#Réalisez les connexions supplémentaires entre signaux et slots
		# connecte le signal Clicked de l'objet bouton à l'appel de la fonction voulue 
		
		#-- fichiers 
		self.connect(self.pushButtonOuvrir, SIGNAL("clicked()"), self.pushButtonOuvrirClicked) # connecte le signal Clicked de l'objet bouton à l'appel de la fonction voulue 
		self.connect(self.pushButtonEnregistrer, SIGNAL("clicked()"), self.pushButtonEnregistrerClicked) # connecte le signal Clicked de l'objet bouton à l'appel de la fonction voulue 
		self.connect(self.pushButtonNouveau, SIGNAL("clicked()"), self.pushButtonNouveauClicked) # connecte le signal Clicked de l'objet bouton à l'appel de la fonction voulue 

		self.connect(self.pushButtonAjouterData, SIGNAL("clicked()"), self.pushButtonAjouterDataClicked) # connecte le signal Clicked de l'objet bouton à l'appel de la fonction voulue 

		self.connect(self.pushButtonOuvrirRep, SIGNAL("clicked()"), self.pushButtonOuvrirRepClicked) # connecte le signal Clicked de l'objet bouton à l'appel de la fonction voulue 

		
		# réseau
		self.connect(self.pushButtonInitServeur, SIGNAL("clicked()"), self.pushButtonInitServeurClicked) 
		self.connect(self.pushButtonPing, SIGNAL("clicked()"), self.pushButtonPingClicked) 

		#self.clientLocal=socket.socket(socket.AF_INET, socket.SOCK_STREAM) # initialise l'interface reseau
		# la famille d'adresse IPV4, ipV6 ou Unix parmi : AF_INET (the default), AF_INET6 or AF_UNIX
		# le type d'interface réseau parmi :  socket.SOCK_STREAM et socket.SOCK_DGRAM
		
		# -- initialisation du serveur tcp -- 
		self.serveurTcp=QTcpServer(self) # déclare serveur- ne pas oublier self...

		ipServeurLocal=self.lineEditIPServeurLocal.text() # ip du serveur local à partir champ texte - ifconfig au besoin 
		portTCP=int(self.lineEditPortTCP.text())  # port du serveur local à partir champ texte
		
		#self.serveurTcp.listen( QHostAddress("192.168.1.14"), portTCP) # lance écoute du serveur
		#self.serveurTcp.listen( QHostAddress("192.168.1.100"), portTCP) # lance écoute du serveur
		#self.serveurTcp.listen( QHostAddress.Any, portTCP) # lance écoute du serveur
		#self.serveurTcp.listen( ipServeurLocal, portTCP) # lance écoute du serveur
			
		#self.serveurTcp.waitForNewConnection (50000)
		#print ("Connexion détectée")
				
		#test=self.serveurTcp.listen( QHostAddress("192.168.1.14"), 8080) # lance écoute du serveur - utiliser port libre ++
		test=self.serveurTcp.listen( QHostAddress(QHostAddress.Any), 8080) # lance écoute du serveur - utiliser port libre ++
		# l'adresse est à priori l'adresse du serveur local
		# laisser QHostAddress("0.0.0.0")
		print test 
		if test==True : 
			print ("Serveur en écoute")
			#-- change aspect bouton init
			self.pushButtonInitServeur.setStyleSheet(QString.fromUtf8("background-color: rgb(0, 255, 0);")) # bouton en vert
			self.pushButtonInitServeur.setText("OK")  # change titre bouton 
		else: 
			print ("problème lancement serveur")
			#-- change aspect bouton init
			self.pushButtonInitServeur.setStyleSheet(QString.fromUtf8("background-color: rgb(255, 127, 0);")) # bouton en vert
			self.pushButtonInitServeur.setText("PB")  # change titre bouton 
		
		# connexion du signal "client entrant" à la fonction voulue 
		self.connect(self.serveurTcp, SIGNAL("newConnection()"), self.gestionConnexionClient) 
		#self.connect(self.serveurTcp, SIGNAL("newConnection()"), self.gestionConnexionClient, Qt.DirectConnection) 

		# chaine entete http
		self.enteteHttp=(
		"""
HTTP/1.1 200 OK
Content-Type: text/html
Connnection: close

""")
		
		self.lineEditCheminRep.setText(os.getenv('HOME')+'/data/texte') # chemin par défaut du champ
		
	# les fonctions appelées, utilisées par les signaux 
	
	
	#========== fonctions des signaux pour la gestion du fichier ====================
	def pushButtonOuvrirClicked(self):
		print("Bouton <OUVRIR> appuyé")
		
		# ouvre fichier en tenant compte du chemin déjà saisi dans le champ 
		if self.lineEditChemin.text()=="":
			self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', os.getenv('HOME')) # ouvre l'interface fichier - home par défaut
			#self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', QDir.currentPath()) # ouvre l'interface fichier - chemin courant par défaut
		else:
			info=QFileInfo(self.lineEditChemin.text()) # définit un objet pour manipuler info sur fichier à partir chaine champ
			print info.absoluteFilePath() # debug	
			self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', info.absoluteFilePath()) # ouvre l'interface fichier - à partir chemin 
	
		#self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', os.getenv('HOME')) # ouvre l'interface fichier - home par défaut
		#self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', QDir.currentPath()) # ouvre l'interface fichier - chemin courant par défaut
		# getOpenFileName ouvre le fichier sans l'effacer
		
		print(self.filename) # affiche le chemin obtenu dans la console
		self.lineEditChemin.setText(self.filename) # affiche le chemin obtenu dans le champ texte
				
		
		#-- ouverture du fichier Ui et récupération du contenu 
		myFile=open(self.filename,"r") # ouvre le fichier en lecture
		myFileContent=myFile.read() # lit le contenu du fichier
		myFile.close() # ferme le fichier - tant que le fichier reste ouvert, il est inacessible à d'autres ressources
		
		self.textEdit.setText(myFileContent) # copie le contenu dans la zone texte 
		
		
		"""
		#-- ouverture du fichier et récupération du contenu - version avec fonctions PyQt 
		myFile=QFile(self.filename) # définit objet fichier
		myFile.open(QFile.ReadOnly) # ouvre le fichier en lecture
		myFileContent=myFile.readAll() # lit le contenu du fichier - attentino renvoie un QByteArray... 
		myFile.close() # ferme le fichier - tant que le fichier reste ouvert, il est inacessible à d'autres ressources

		self.textEdit.setText(str(myFileContent)) # copie le contenu dans la zone texte 
		"""
	# -- fin def pushButtonOuvrirClicked

	def pushButtonNouveauClicked(self):
		print("Bouton NOUVEAU appuyé")
		
		# ouvre fichier en tenant compte du chemin déjà saisi dans le champ 
		if self.lineEditChemin.text()=="":
			self.filename=QFileDialog.getSaveFileName(self, 'Ouvrir fichier', os.getenv('HOME')) # ouvre l'interface fichier - home par défaut
			#self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', QDir.currentPath()) # ouvre l'interface fichier - chemin courant par défaut
		else:
			info=QFileInfo(self.lineEditChemin.text()) # définit un objet pour manipuler info sur fichier à partir chaine champ
			print info.absoluteFilePath() # debug	
			self.filename=QFileDialog.getSaveFileName(self, 'Ouvrir fichier', info.absoluteFilePath()) # ouvre l'interface fichier - à partir chemin 

		#self.filename=QFileDialog.getSaveFileName(self, 'Nouveau fichier', os.getenv('HOME')) # ouvre l'interface fichier
		# self.filename=QFileDialog.getSaveFileName(self, 'Save File', QDir.currentPath()) # ouvre l'interface fichier - alternative chemin

		# self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', os.getenv('HOME')) # ouvre l'interface fichier
		# getOpenFileName ouvre le fichier sans l'effacer et getSaveFileName l'efface si il existe 
		
		print(self.filename)
		self.lineEditChemin.setText(self.filename)

		#--- efface le contenu du fichier --- 
		if self.lineEditChemin.text()!="":
			#self.myFile = open(self.filename, 'a') # ouverture du fichier en mode écriture append
			self.myFile = open(self.filename, 'w') # ouverture du fichier en mode écriture write - efface contenu existant
			# open est une fonction du langage python : http://docs.python.org/2/library/functions.html#open
			# mode peut-être r, w, a (append)		
			self.myFile.write("") # écrit les données dans le fichier		
			self.myFile.close() # ferme le fichier 
		

	def pushButtonEnregistrerClicked(self):		
		print("Bouton <ENREGISTRE> appuyé")			
	
		if self.lineEditChemin.text()!="":
			#self.myFile = open(self.filename, 'a') # ouverture du fichier en mode écriture append
			self.myFile = open(self.filename, 'w') # ouverture du fichier en mode écriture write - efface contenu existant
			# open est une fonction du langage python : http://docs.python.org/2/library/functions.html#open
			# mode peut-être r, w, a (append)		
			self.myFile.write(str(self.textEdit.toPlainText())) # écrit les données dans le fichier		
			self.myFile.close() # ferme le fichier 

	def pushButtonAjouterDataClicked(self):
		print("Bouton <Ajouter Data> appuyé")
		
		#-- ajoute une ligne de donnée au fichier
		if self.lineEditChemin.text()!="":
			self.myFile = open(self.filename, 'a') # ouverture du fichier en mode écriture append
			#self.myFile = open(self.filename, 'w') # ouverture du fichier en mode écriture write - efface contenu existant
			# open est une fonction du langage python : http://docs.python.org/2/library/functions.html#open
			# mode peut-être r, w, a (append)		
			self.myFile.write(str(self.lineEditData.text()+"\n")) # écrit les données dans le fichier		
			self.myFile.close() # ferme le fichier 		
			
			#-- ajoute une ligne de donnée au champt texte
			self.textEdit.append(self.lineEditData.text()) # copie le champ texte dans la zone de texte + saut de ligne

	#====== gestion signaux repertoire ==========
	def pushButtonOuvrirRepClicked(self):
		print("Bouton <Sélectionner Répertoire> appuyé")
		
		#self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', os.getenv('HOME')) # ouvre l'interface fichier - home par défaut
		#self.filename=QFileDialog.getOpenFileName(self, 'Ouvrir fichier', QDir.currentPath()) # ouvre l'interface fichier - chemin courant par défaut
		# getOpenFileName ouvre le fichier sans l'effacer
		
		self.dirname = QFileDialog.getExistingDirectory(self,"Choisir repertoire",os.getenv('HOME'),QFileDialog.ShowDirsOnly| QFileDialog.DontResolveSymlinks)
		
		print(self.dirname) # affiche le chemin obtenu dans la console
		self.lineEditCheminRep.setText(QString.fromUtf8(self.dirname)) # affiche le chemin obtenu dans le champ texte
		
		
		#-- ouverture du fichier et récupération du contenu 
		#myFile=open(self.filename,"r") # ouvre le fichier en lecture
		#myFileContent=myFile.read() # lit le contenu du fichier
		#myFile.close() # ferme le fichier - tant que le fichier reste ouvert, il est inacessible à d'autres ressources
		
		#self.textEdit.setText(myFileContent) # copie le contenu dans la zone texte 

		"""
		#-- ouverture du fichier et récupération du contenu - version avec fonctions PyQt 
		myFile=QFile(self.filename) # définit objet fichier
		myFile.open(QFile.ReadOnly) # ouvre le fichier en lecture
		myFileContent=myFile.readAll() # lit le contenu du fichier - attentino renvoie un QByteArray... 
		myFile.close() # ferme le fichier - tant que le fichier reste ouvert, il est inacessible à d'autres ressources

		self.textEdit.setText(str(myFileContent)) # copie le contenu dans la zone texte 
		"""

		"""
		#-- ouverture du répertoire et récupération du contenu - fonctions Python 
		path=str(self.lineEditChemin.text())  # le chemin du répertoire
		dirList=os.listdir(path) # la liste du contenu du fichier
		
		for filename in dirList: # affiche les noms
			print filename		
		"""

		#-- ouverture du répertoire et récupération du contenu - fonctions PyQt
		myDir=QDir(self.lineEditChemin.text()) # définit objet répertoire
		#filesList=myDir.entryList() # liste des entrées... = liste du contenu 
		#filesList=myDir.entryList(["*.*"], QDir.Files, QDir.Name) # liste des entrées... avec filtres 
		filesList=myDir.entryList(["*.txt"], QDir.Files, QDir.Name) # liste des entrées... avec filtres 
		# ici que les fichier triés par nom - filtre *.txt
		
		# QStringList QDir.entryList (self, Filters filters = QDir.NoFilter, SortFlags sort = QDir.NoSort)
		# QStringList QDir.entryList (self, QStringList nameFilters, Filters filters = QDir.NoFilter, SortFlags sort = QDir.NoSort)
		
		# les filtres possibles http://pyqt.sourceforge.net/Docs/PyQt4/qdir.html#Filter-enum
		# classement possibles : http://pyqt.sourceforge.net/Docs/PyQt4/qdir.html#SortFlag-enum
		self.textEdit.setText("") # efface le champ texte 
		
		for fileName in filesList: # défile les noms des fichiers..
			print fileName # affiche le fichier
			self.textEdit.append(fileName) # ajoute le fichier à la zone texte


	#---- fonctions communes fichiers utiles ------- 

	"""		
	def getContentDir(self,pathIn):
		
		QString.fromUtf8(pathIn) # chemin reçu 
			
		#-- ouverture du répertoire et récupération du contenu - fonctions PyQt
		myDir=QDir(pathIn) # définit objet répertoire
		filesList=myDir.entryList() # liste des entrées... = liste du contenu 
		#filesList=myDir.entryList(["*.*"], QDir.Files, QDir.Name) # liste des entrées... avec filtres 
		#filesList=myDir.entryList(["*.txt"], QDir.Files, QDir.Name) # liste des entrées... avec filtres 
		# ici que les fichier triés par nom - filtre *.txt
		
		# QStringList QDir.entryList (self, Filters filters = QDir.NoFilter, SortFlags sort = QDir.NoSort)
		# QStringList QDir.entryList (self, QStringList nameFilters, Filters filters = QDir.NoFilter, SortFlags sort = QDir.NoSort)
		
		# les filtres possibles http://pyqt.sourceforge.net/Docs/PyQt4/qdir.html#Filter-enum
		# classement possibles : http://pyqt.sourceforge.net/Docs/PyQt4/qdir.html#SortFlag-enum
		
		contentOut="" # chaine de contenu
				
		for fileName in filesList: # défile les noms des fichiers..
			print fileName +"\t"+str(QFileInfo(pathIn+"/"+fileName).size())+"\t Octets " # affiche le fichier + tab + taille
			contentOut=contentOut+fileName+"\t"+str(QFileInfo(pathIn+"/"+fileName).size())+"\t Octets \n" # ajoute ligne saut de ligne 

		#self.textEdit.setText("") # efface le champ texte 
		self.textEdit.setText(contentOut) # efface le champ texte et affiche le fichier à la zone texte 
		return(contentOut) # renvoie la chaîne de contenu
	"""

	"""
	def writeFile(self, absoluteFilenameIn, strIn):		
		
		absoluteFilenameIn=str(absoluteFilenameIn) # chemin reçu 
		strIn=str(strIn) # chaine à écrire
		
		if not os.path.isfile(absoluteFilenameIn): return("Echec : Le fichier n'existe pas!") # si le fichier n'existe pas

		myFile = open(absoluteFilenameIn, 'a') # ouverture du fichier en mode écriture append
		#myFile = open(absoluteFilenameIn, 'w') # ouverture du fichier en mode écriture write - efface contenu existant
		# open est une fonction du langage python : http://docs.python.org/2/library/functions.html#open
		# mode peut-être r, w, a (append)		
		myFile.write(strIn) # écrit les données dans le fichier		
		myFile.close() # ferme le fichier 

		return("Ecriture fichier OK"); 
	"""
	"""
	def readFile(self,absoluteFilenameIn):

		absoluteFilenameIn=str(absoluteFilenameIn) # chemin reçu 
		
		#-- ouverture du fichier Ui et récupération du contenu 
		myFile=open(absoluteFilenameIn,"r") # ouvre le fichier en lecture
		myFileContent=myFile.read() # lit le contenu du fichier
		myFile.close() # ferme le fichier - tant que le fichier reste ouvert, il est inacessible à d'autres ressources
		
		#self.textEdit.setText(myFileContent) # copie le contenu dans la zone texte 
		return(myFileContent)
	"""
	"""
	def getNumberOfLines(self,absoluteFilenameIn):

		absoluteFilenameIn=str(absoluteFilenameIn) # chemin reçu 
		
		myFileContent=self.readFile(absoluteFilenameIn) # récupère le contenu du fichier
		#print myFileContent # debug
		
		lines=myFileContent.count("\n") +1 # comptage du nombre de saut de ligne + 1 et donc de lignes.. !
		# +1 car la ligne courante est une ligne même si pas de saut de ligne..
		print ("Nombre de lignes =" + str(lines))
		
		return(lines)
	"""
	"""
	def getLine(self,absoluteFilenameIn, lineNumberIn):

		absoluteFilenameIn=str(absoluteFilenameIn) # chemin reçu 
		lineNumberIn=int(lineNumberIn) # numéro de ligne à extraire
		
		myFileContent=self.readFile(absoluteFilenameIn) # récupère le contenu du fichier
		#print myFileContent # debug
		
		allLines=myFileContent.splitlines() # extrait toutes les lignes
		print(len(allLines))
		print(allLines)
		
		if lineNumberIn<=len(allLines): # si ligne demandée cohérente... 
			line=str(allLines[lineNumberIn-1]) # extrait la ligne voulue - la ligne 1 a l'index 0 
			print ("Ligne à extraire" + line)
			return(line)
		else:
			return("Depassement nombre lignes"); 
	"""	
		
	"""	
	def createFile(self, absoluteFilenameIn):		
		
		absoluteFilenameIn=str(absoluteFilenameIn) # chemin reçu 

		if os.path.isfile(absoluteFilenameIn): return("Echec : Le fichier existe !") # si le fichier existe 

		#self.myFile = open(self.filename, 'a') # ouverture du fichier en mode écriture append
		myFile = open(absoluteFilenameIn, 'w') # ouverture du fichier en mode écriture write - efface contenu existant
		# open est une fonction du langage python : http://docs.python.org/2/library/functions.html#open
		# mode peut-être r, w, a (append)		
		myFile.write("") # écrit les données dans le fichier		
		myFile.close() # ferme le fichier 

		return("Creation fichier OK"); 
	"""
	"""
	def removeFile(self, absoluteFilenameIn):		
		
		absoluteFilenameIn=str(absoluteFilenameIn) # chemin reçu 

		if not os.path.isfile(absoluteFilenameIn): return("Echec : Le fichier n'existe pas!") # si le fichier existe 
		else : 
			os.remove(absoluteFilenameIn)
			return("Effacement fichier OK !")
	"""		
	"""
	def sizeFile(self, absoluteFilenameIn):		
		
		absoluteFilenameIn=str(absoluteFilenameIn) # chemin reçu 

		if not os.path.isfile(absoluteFilenameIn): return("Echec : Le fichier n'existe pas!") # si le fichier existe 
		else : 
			#size=long(os.path.getsize(absoluteFilenameIn))
			size=QFileInfo(absoluteFilenameIn).size() # idem avec fonctions PyQt
			return(size)
	"""
	
	def testDatalog(self, absoluteFilenameIn,nombreLignesIn):

		absoluteFilenameIn=str(absoluteFilenameIn) # chemin reçu 
		nombreLignesIn=int(nombreLignesIn) # nombre de lignes
		
		if os.path.isfile(absoluteFilenameIn): return("Echec : Le fichier existe !") # si le fichier existe : on sort 
		
		utils.createFile(absoluteFilenameIn) # crée le fichier

		myFile = open(absoluteFilenameIn, 'w') # ouverture du fichier en mode écriture write - efface contenu existant
		# open est une fonction du langage python : http://docs.python.org/2/library/functions.html#open
		# mode peut-être r, w, a (append)		

		for i in range(0,nombreLignesIn): # défile n lignes	
			myFile.write(str(i+1)+";"+str(time.time())+"\n") # écrit les données dans le fichier	+ saut ligne 	
		
		myFile.close() # ferme le fichier 
		
		return(str(nombreLignesIn) +" valeurs ajoutees dans le fichier")
			
	#============== gestion du réseau ====================	
	def gestionConnexionClient(self): # fonction appelée lorsqu'une connexion client entrante survient 
		print ("Une connexion entrante est détectée")
		
		# -- connexion du client -- 
		self.clientTcpDistant=self.serveurTcp.nextPendingConnection() # récupère le TcpSocket correspondant au client connecté
		# l'objet client est un  TcpSocket et dispose donc de toutes les fonctions du TcpSocket
		
		etatConnexion=self.clientTcpDistant.waitForConnected(5000) # attend connexion pendant 5 secondes maxi
		
		# message état connexion 
		if etatConnexion : # si la connexion est OK .. 
			print ("Connexion au serveur OK !")
			print ("IP du client connecté : " + str(self.clientTcpDistant.peerAddress().toString())) # affiche IP du client distant 
			self.lineEditIPClientDistant.setText(self.clientTcpDistant.peerAddress().toString()) # MAJ champ IP client
			print ("IP du serveur local : " + str(self.clientTcpDistant.localAddress().toString())) # affiche IP du serveur local auquel le client est connecté
			self.lineEditIPServeurLocal.setText(self.clientTcpDistant.localAddress().toString()) # MAJ champ IP serveur
		else : 
			print ("Connexion au serveur impossible...")
			#exit # sort du try mais reste dans la fonction def
			return # sort de def 
		
		# suite si la connexion est ok...
		
		# -- lecture des données en réception = réception de la requête du client distant
		test=self.clientTcpDistant.waitForReadyRead() # attendre que client soit prêt pour réception de données
		if test==True : print("Données client distant prêtes")
		else: print ("Données client distant pas prêtes")
		
		chaineTrans =str(self.clientTcpDistant.readAll()) # lit les données en réception - première lecture - readAll lit ligne à ligne...
		#chaineTrans =str(self.clientTcpLocal.readData(1024)) # lit les données en réception - première lecture - read() lit ligne à ligne...
		chaineReception = ""
		# print chaineTrans - debug
		
		#while len(chaineTrans): # tant que chaine Trans pas vide - obligé car réception ligne à ligne 
		while chaineTrans!="": # tant que chaine Trans pas vide - obligé car réception ligne à ligne 
			chaineReception = chaineReception + chaineTrans
			chaineTrans="" # RAZ chaine Trans
			
			test=self.clientTcpDistant.waitForReadyRead(1000) # attendre que client soit à nouveau prêt pour réception de données
			if test==True : # si prêt à recevoir données 
				#print ("Client prêt à recevoir des données")
				chaineTrans =str(self.clientTcpDistant.readAll()) # lit la suite des données en réception
				#chaineTrans =str(self.clientTcpLocal.readData(1024)) # lit la suite des données en réception - read() lit ligne à ligne...
				#print self.clientTcpLocal.isOpen() # debug
				#print (">"+chaineTrans) # debug
			#else:
			#	print("Client pas prêt à recevoir des données")
			
		#-- fin réception réponse client
		
		# si on veut message erreur si problème, utiliser readData(taillemax)
		print ("Données reçues : ")
		print ("-------------------------------------------")
		print chaineReception
		print ("-------------------------------------------")
		
		self.textEditReceptionReseau.append(chaineReception) # ajoute réception à la zone texte 		
		
		# -- +/- analyse de la requete reçue --
		
		#------> analyse si la chaine reçue est une requete GET avec chaine format /&chaine= càd si requête Ajax--------
		if chaineReception.startswith("GET /&"):
			print ("Requête AJAX reçue") 
			reponseAjax="" # pour mémoriser éléments de la réponse Ajax au fur et à mesure analyse
			
			#----- extraction de la chaine allant de & à =
			indexStart=chaineReception.find("&") # position debut
			print ("indexStart = " + str(indexStart) ) # debug
			indexEnd=chaineReception.find("=") # position fin
			print ("indexEnd = " + str(indexEnd) ) # debug
			
			chaineAnalyse=chaineReception[indexStart+1:indexEnd] # garde chaine fonction(xxxx) à partir de GET /&fonction(xxxx)=
			# [a:b] 1er caractère inclusif (d'où le +1) , dernier exclusif 
			print ("Chaine recue : "+ chaineAnalyse) # debug
			
			#---------------------->>  +/- ici analyse de la chaine  <<------------------ 

			#--- ls ( ) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "ls(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax

				#if (result=="/"): 
				cheminAbsolu=self.lineEditCheminRep.text()+result # le chemin absolu à utiliser
				contenu = utils.getContentDir(cheminAbsolu) # getContentDir renvoie chaîne
				print(contenu) # affiche le contenu du rép - 
				self.textEdit.setText(contenu) # efface le champ texte et affiche le fichier à la zone texte 
				reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax

			#--- read ( ) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "read(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax

				#if (result=="/"): 
				cheminAbsolu=self.lineEditCheminRep.text()+"/"+result # le chemin absolu à utiliser
				#contenu = self.getContentDir(cheminAbsolu) # getContentDir renvoie chaîne
				contenu=utils.readFile(cheminAbsolu)# readFile renvoie chaine 
				print(contenu) # affiche le contenu du fichier
				reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax

			#--- lines ( ) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "lines(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax

				#if (result=="/"): 
				cheminAbsolu=self.lineEditCheminRep.text()+"/"+result # le chemin absolu à utiliser
				#contenu = self.getContentDir(cheminAbsolu) # getContentDir renvoie chaîne
				contenu=str(utils.getNumberOfLines(cheminAbsolu))# getNumberOfLines renvoie int - nombre de lignes
				print(contenu) # affiche le contenu du fichier
				reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax

			#--- size ( fichier) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "size(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax

				#if (result=="/"): 
				cheminAbsolu=self.lineEditCheminRep.text()+"/"+result # le chemin absolu à utiliser
				#contenu = self.getContentDir(cheminAbsolu) # getContentDir renvoie chaîne
				contenu=str(utils.sizeFile(cheminAbsolu))# getNumberOfLines renvoie int - nombre de lignes
				print(contenu) # affiche le contenu du fichier
				reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax

			#--- write ( ) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "write(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax
					
				subResult=result.split(',') # sépare les sous chaînes séparées par ,
				print subResult
				
				if len(subResult)==2: # si on a bien 2 sous chaînes
					#if (result=="/"): 
					cheminAbsolu=self.lineEditCheminRep.text()+"/"+subResult[0] # le chemin absolu à utiliser
					#contenu = self.getContentDir(cheminAbsolu) # getContentDir renvoie chaîne
					contenu=utils.writeFile(cheminAbsolu,str(subResult[1])+"\n")# writeFile renvoie chaine 
					print(contenu) # affiche le contenu du fichier
					reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax
				else:
					reponseAjax=reponseAjax+"Erreur format\n" # ajoute à la réponse Ajax

			#--- getline ( fichier, ligne) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "getline(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax
					
				subResult=result.split(',') # sépare les sous chaînes séparées par ,
				print subResult
				
				if len(subResult)==2: # si on a bien 2 sous chaînes
					#if (result=="/"): 
					cheminAbsolu=self.lineEditCheminRep.text()+"/"+subResult[0] # le chemin absolu à utiliser
					if subResult[1].isalnum() : # si 2ème param est bien en chiffres
						contenu=utils.getLine(cheminAbsolu,str(subResult[1]))# getLine renvoie chaine 
						print(contenu) # affiche le contenu du fichier
						reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax
					else:
						reponseAjax=reponseAjax+"Erreur format\n" # ajoute à la réponse Ajax

			#--- testdatalog ( fichier, nombrelignes) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "testdatalog(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax
					
				subResult=result.split(',') # sépare les sous chaînes séparées par ,
				print subResult
				
				if len(subResult)==2: # si on a bien 2 sous chaînes
					#if (result=="/"): 
					cheminAbsolu=self.lineEditCheminRep.text()+"/"+subResult[0] # le chemin absolu à utiliser
					if subResult[1].isalnum() : # si 2ème param est bien en chiffres
						contenu=self.testDatalog(cheminAbsolu,str(subResult[1]))# testDatalog renvoie chaine 
						print(contenu) # affiche le contenu du fichier
						reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax
					else:
						reponseAjax=reponseAjax+"Erreur format\n" # ajoute à la réponse Ajax
					
			#--- createfile ( ) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "createfile(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax

				#if (result=="/"): 
				cheminAbsolu=self.lineEditCheminRep.text()+"/"+result # le chemin absolu à utiliser
				#contenu = self.getContentDir(cheminAbsolu) # getContentDir renvoie chaîne
				contenu=utils.createFile(cheminAbsolu)# readFile renvoie chaine 
				print(contenu) # affiche le contenu du fichier
				reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax

			#--- remove ( ) --
			#testInstructionString (self, chaineTestIn, chaineRefIn, debugIn)
			result=utils.testInstructionString(chaineAnalyse, "remove(", True) # appelle fonction test instruction format fonction(chaine)
			if(result):
				print ("result = " + result ) 
				reponseAjax=reponseAjax+result+"\n" # ajoute à la réponse Ajax

				#if (result=="/"): 
				cheminAbsolu=self.lineEditCheminRep.text()+"/"+result # le chemin absolu à utiliser
				contenu=utils.removeFile(cheminAbsolu)# readFile renvoie chaine 
				print(contenu) # affiche le contenu du fichier
				reponseAjax=reponseAjax+contenu # ajoute à la réponse Ajax

			#----- avec params chiffrés --- 
			"""
			args=utils.testInstructionLong(chaineAnalyse, "lines(", True) # extrait paramètre chaine au format racine (xx,xx,xx,..)
			if args:  # args est True si 1 ou plusieurs paramètres numériques sont trouvés - None sinon 
				print args
			"""	
				
			#--- construction de la réponse complète 
			#reponse=self.plainTextEditReponseHttp.toPlainText() +chaineAnalyse+"\n" # Utf-8
			#reponse=self.enteteHttp+chaineAnalyse+"\n"+result+"\n" # +str(args)+"\n" # Utf-8
			reponse=self.enteteHttp+chaineAnalyse+"\n"+reponseAjax+"\n"
			
			self.envoiChaineClientTcp(reponse) # envoi la reponse au client - appelle fonction commune
			
			"""
			self.textEditEnvoiReseau.append(reponse) # ajoute à la zone texte d'envoi 
			
			print reponse.toAscii() # convertit en ascii le String - avec conversion unicode... 
			
			#reponse=QString.fromUtf8(reponse) # 2ers octets UTF-8 seulement 
			
			reponseByteArray=reponse.toAscii() # convertit en ascii le String - avec conversion unicode... 
			#byteArray=QByteArray()
			#byteArray.append(reponse)
			
			test=self.clientTcpDistant.write(reponseByteArray) # écrit les données vers le serveur avec CRLF		
			if test==-1 : print ("Erreur en écriture vers le client")
			else: print (str(test)+ " octets envoyés vers le client")
			
			#self.textEditReceptionReseau.append("Reponse envoyee au client : " + str(reponseByteArray)+"\n")		

			test=self.clientTcpDistant.waitForBytesWritten() # attend fin envoi
			if test==False : print("Problème envoi")
			else: print ("envoi réussi")
			"""
			
		#-- fin si GET /&
		
		# ---------> si la chaine recue commence par GET et pas une réponse précédente = on envoie page initiale entiere
		elif chaineReception.startswith("GET"): 
					
		
			# -- écriture des données vers le client = envoi de la réponse du serveur local --
			#test=self.clientTcpLocal.writeData("GET") # écrit les données vers le serveur
			#test=self.clientTcpDistant.writeData(str(self.lineEditReponse.text())) # écrit les données vers le serveur	
			#reponse=str(QString.fromUtf8(self.plainTextEditReponse.toPlainText()))+str("\n")
			
			#reponse=self.plainTextEditReponseHttp.toPlainText() +self.plainTextEditReponseHtml.toPlainText()+"\n" # Utf-8
			#reponseHtml=self.plainTextEditReponseHtml.toPlainText() # partie Html de la réponse
			
			#reponseHtml.replace(self.lineEditChaineSubstHtml.text(), "var val = new Array(100,200,300,400,500,600);"); # debug - remplace chaine des valeurs à passer au client
			# typiquement, la réponse html contient du code javascript avec un tableau de valeurs var val = new Array(0,0,0,0,0,0);
			# celui-ci est remplacé par le tableau de nouvelles valeurs
			
			"""
			reponseHtml.replace(self.lineEditChaineSubstHtml.text(), "var val = new Array("
			+str(self.lcdNumber_A0.intValue())+","
			+str(self.lcdNumber_A1.intValue()) +","
			+str(self.lcdNumber_A2.intValue())+","
			+str(self.lcdNumber_A3.intValue())+","
			+str(self.lcdNumber_A4.intValue())+","
			+str(self.lcdNumber_A5.intValue()) +");"); # remplace chaine des valeurs à passer au client - ici les valeurs des lcdNumber
			"""
			#--- la réponse HTML + Javascript
			reponseHtml=""
			
			#-- début html + le head avec javascript -- 
			#### ATTENTION : les sections """ """ qui suivent ne sont pas des commentaires mais du code HTML/Javascript actif envoyé au client
			#### NE PAS EFFACER +++ 
			
			reponseHtml=(reponseHtml+
"""
<!DOCTYPE html>
<html>

<head>
<meta charset=\"utf-8\" />
<title>Titre</title>

<!-- Debut du code Javascript  -->
<script language=\"javascript\" type=\"text/javascript\">
<!-- 

//-- variables et objets globaux 
var textarea=null;
var textInputX=null;
	
//--- fonction appelée sur clic bouton
function onclickButton() { // click Button ON
	requeteAjax(\"&\"+textInput.value+\"=\", drawData); // envoi requete avec &chaine= et fonction de gestion resultat	
} // fin onclickButton

//--- fonction executee au lancement
window.onload = function () { // au chargement de la page
	textarea = document.getElementById(\"textarea\"); // declare objet canvas a partir id = nom
	textarea.value=\"\";// efface le contenu 
	
	textInput= document.getElementById(\"valeur\"); // declare objet champ text a partir id = nom"
	
	} // fin onload

//--- fonction de requete AJAX	
function requeteAjax(chaineIn, callback) {
	
	var xhr = XMLHttpRequest();
	xhr.open(\"GET\", chaineIn, true); // envoi requete avec chaine personnalisee
	xhr.send(null);
	
	//------ gestion de l'évènement onreadystatechange ----- 
	xhr.onreadystatechange = function() {

		if (xhr.readyState == 4 && xhr.status == 200) {
			//alert(xhr.responseText);
			callback(xhr.responseText);
		} // fin if
		
	}; // fin function onreadystatechange		
		
} // fin fonction requeteAjax

//-- fonction de gestion de la reponse a la requete AJAX --
function drawData(stringDataIn) {
	
	// ajoute la réponse au champ texte 
	textarea.value=textarea.value+stringDataIn; // ajoute la chaine au début - décale vers le bas...
	textarea.setSelectionRange(textarea.selectionEnd-1,textarea.selectionEnd-1); // se place à la fin -1 pour avant saut de ligne
	
} // fin fonction drawData

//-->
</script>
<!-- Fin du code Javascript -->

</head>

"""
			) # les parenthèses encadrent la chaîne et la variable comme si c'était la même ligne
			
			#-- le body + fin HTML -- 
			reponseHtml=(reponseHtml+
"""
<body>

Serveur Python

<br/>
<input type=\"text\" id=\"valeur\" size=\"50\" />
<button type=\"button\" onclick=\"onclickButton()\">Envoyer</button>
<br/>
En provenance du serveur :
<br/>
<textarea id=\"textarea\" rows=\"10\" cols=\"50\" > </textarea>
<br/>
</body>

</html>
"""
			) # les parenthèses encadrent la chaîne et la variable comme si c'était la même ligne			
			
			#--- construction de la réponse complète 
			#reponse=self.plainTextEditReponseHttp.toPlainText() +reponseHtml+"\n" # Utf-8
			reponse=self.enteteHttp+reponseHtml+"\n" # Utf-8
			
			self.envoiChaineClientTcp(reponse) # envoi la reponse au client - appelle fonction commune

			"""
			self.textEditEnvoiReseau.append(reponse) # ajoute à la zone texte d'envoi 
			
			print reponse.toAscii() # convertit en ascii le String - avec conversion unicode... 
			
			#reponse=QString.fromUtf8(reponse) # 2ers octets UTF-8 seulement 
			
			reponseByteArray=reponse.toAscii() # convertit en ascii le String - avec conversion unicode... 
			#byteArray=QByteArray()
			#byteArray.append(reponse)
			
			test=self.clientTcpDistant.write(reponseByteArray) # écrit les données vers le serveur avec CRLF		
			if test==-1 : print ("Erreur en écriture vers le client")
			else: print (str(test)+ " octets envoyés vers le client")
			
			#self.textEditReceptionReseau.append("Reponse envoyee au client : " + str(reponseByteArray)+"\n")		

			test=self.clientTcpDistant.waitForBytesWritten() # attend fin envoi
			if test==False : print("Problème envoi")
			else: print ("envoi réussi")
			"""
			
		#-- fin si "GET" = fin envoi page initiale complète 

		# -- fin de la connexion -- 
		self.clientTcpDistant.close() # fin de la connexion 
		print ("Fermeture du client tcp distant effectuée")
		print ("===========================================")
		print ("")

	# ---- fin def gestionConnexionClient
		
	def pushButtonInitServeurClicked(self):
		print("Bouton Init Serveur cliqué")
		

	
	def pushButtonPingClicked(self):
		print("Bouton Ping! cliqué")

		chaineCommande="ping -c 5 " + str(self.lineEditIPClientDistant.text())
		
		
		print("Commande à exécuter : " + chaineCommande)
		self.textEditReceptionReseau.append(QString.fromUtf8("Commande à exécuter : " + chaineCommande)) # message dans console GUI 
		
		
		commandeList=chaineCommande.split() # récupère la liste des éléments séparés par 1 ou plusieurs " "
		print ("Liste = " + str(commandeList)) # affiche la liste 
		
		if len(commandeList)>0 : # si la liste n'est pas vide 
			cmd=commandeList[0] # extrait la commande si existe 
			print ("Commande : " + cmd) # affiche la commande
			
			del(commandeList[0]) # enlève 1er élément de la liste donc la commande pour ne garder que paramètres 
			args=commandeList # copie les arguments			
			
			if args==[]: args="" # chaine vide si Args=None 
			print ("Arguments : " + str(args))
			
		
		#-- exécution de la commande --
		process=QProcess() # crée le QProcess
		
		#process.start("ls -a") # exemple de commande simple
		print ("Lance la commande :" + str(cmd) + str(args)) # affiche la commande dans la console
		self.textEditReceptionReseau.append(QString.fromUtf8("Lance la commande :" + str(cmd) + str(args))) # message dans console GUI 
		process.start(cmd,args) # lance la commande
				
		process.waitForFinished(5000) # attend la fin pendant nmsec
		
		#-- récupère et affiche la sortie console 
		#output=process.readAllStandardOutput() # lit la sortie
		
		# pour affichage progressif type console... ?
		while True:
			outputLine=process.readLine()
			#outputLine=str(outputLine).rstrip() # enlève caractères additionnels 
			if outputLine != '':
				outputLine=str(outputLine)[:-1] # enlève saut de ligne - après if==''
				print(outputLine)
				self.textEditReceptionReseau.append(QString.fromUtf8(outputLine)) # message dans console GUI 
				#self.update()
				#print "test:", line.rstrip()
			else:
				break

		outputErr=process.readAllStandardError() # lit la sortie
		print(outputErr)
		self.textEditReceptionReseau.append(QString.fromUtf8(outputErr)) # message dans console GUI
		
	#-- fonctions communes réseau tcp --
	
	def envoiChaineClientTcp(self, chaineIn):
		
			chaineIn=QString(chaineIn) # cast le string en QString... 
			
			self.textEditEnvoiReseau.append(chaineIn) # ajoute à la zone texte d'envoi 
			
			print chaineIn.toAscii() # convertit en ascii le QString - avec conversion unicode... 
			
			#reponse=QString.fromUtf8(reponse) # 2ers octets UTF-8 seulement 
			
			chaineInByteArray=chaineIn.toAscii() # convertit en ascii le QString - avec conversion unicode... 
			#byteArray=QByteArray()
			#byteArray.append(reponse)
			
			test=self.clientTcpDistant.write(chaineInByteArray) # écrit les données vers le serveur avec CRLF		
			if test==-1 : print ("Erreur en écriture vers le client")
			else: print (str(test)+ " octets envoyés vers le client")
			
			#self.textEditReceptionReseau.append("Reponse envoyee au client : " + str(reponseByteArray)+"\n")		

			test=self.clientTcpDistant.waitForBytesWritten() # attend fin envoi
			if test==False : print("Problème envoi")
			else: print ("envoi réussi")

	
def main(args):
	a=QApplication(args) # crée l'objet application 
	f=QWidget() # crée le QWidget racine
	c=myApp(f) # appelle la classe contenant le code de l'application 
	f.show() # affiche la fenêtre QWidget
	r=a.exec_() # lance l'exécution de l'application 
	return r

if __name__=="__main__": # pour rendre le code exécutable 
	main(sys.argv) # appelle la fonction main

