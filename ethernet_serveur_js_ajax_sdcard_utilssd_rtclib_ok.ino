// Timer multivoie avec RTC DS1307 et carte SD 
// et serveur ethernet... 
// par X. HINAULT - www.mon-club-elec.fr - 04/2013
// tous droits réservés - GPL v3

#define nombreTimers 6 // définit le nombre de timer à utiliser.. 


#include <SPI.h> // fichier libairie pour communication SPI utilisée par carte SD
#include <Wire.h> // lib I2C pour le DS1307
#include <SD.h> // fichier librairie pour gestion de la carte SD
#include <Ethernet.h> // librairie Ethernet


#include <RTClib.h> // lib native RTCLib pour gestion DS1307
#include <RTClibTimer.h> // lib perso pour implémentation timer avec RTCLib
#include <RTClibTimerEthernet.h> // lib perso pour implémentation timer avec RTCLib pour gestion par Ethernet

#include <Utils.h> // inclusion de la librairie Utils 
#include <UtilsSD.h> // inclusion de la librairie Utils SD
#include <UtilsSDEthernet.h> // inclusion de la librairie Utils SD pour Ethernet

#include <UtilsRTClibTimer.h> // inclusion de lib avec fonction analyse chaine sur port série pour RTClibTimer
#include <UtilsRTClibTimerEthernet.h> // inclusion de lib avec fonction analyse chaine sur réseau Ethernet pour RTClibTimer

// broche de sélection de la carte SD : à adapter au shield utilisé
//const int selectSD=10; // 10 par défaut - utiliser une autre si Ethernet en meme temps
const int selectSD=4; // pour le shield Ethernet 

//--- l'adresse mac = identifiant unique du shield
// à fixer arbitrairement ou en utilisant l'adresse imprimée sur l'étiquette du shield
byte mac[] = {  0x90, 0xA2, 0xDA, 0x00, 0x1A, 0x71 };

//----- l'adresse IP fixe à utiliser pour le shield Ethernet --- 
IPAddress ipLocal(192,168,1,100); // l'adresse IP locale du shield Ethernet
// ATTENTION : il faut utiliser une adresse hors de la plage d'adresses du routeur DHCP
// pour connaitre la plage d'adresse du routeur : s'y connecter depuis un navigateur à l'adresse xxx.xxx.xxx.1
// par exemple : sur livebox : plage adresses DHCP entre .10 et .50 => on peut utiliser .100 pour le shield ethernet

//--- création de l'objet serveur ----
EthernetServer serveurHTTP(80); // crée un objet serveur utilisant le port 80 = port HTTP

// objets et variables utiles 
Utils utils; // déclare objet racine d'accès aux fonctions de la librairie Utils
UtilsSD utilsSD; // déclare objet racine d'accès aux fonctions de la librairie UtilsSD
UtilsSDEthernet utilsSDEthernet; // déclare objet racine d'accès aux fonctions de la librairie UtilsSDEthernet

//RTC_DS1307 RTC; // déclare objet représentant le DS1307 - activer RTC.begin() dans setup()
RTC_Millis RTC; // déclare objet base temps RTC basé sur millis() - utile pour debug

DateTime now; // objet date/heure global 

RTClibTimer timerRTC[nombreTimers]; // objet timer
UtilsRTClibTimer utilsRTC; 

RTClibTimerEthernet timerRTCEthernet[nombreTimers]; // objet timer avec lib Ethernet
UtilsRTClibTimerEthernet utilsRTCEthernet; 

//--- réception série 
String chaineReception=""; // déclare un String
String param=""; // déclare un String
long params[6]; // déclare un tableau de long - taille en fonction nombre max paramètres attendus

//--- réception réseau
String chaineRecue=""; // déclare un string vide global pour réception chaine requete
String chaineAnalyse=""; // string vide global pour cahine retenue pour analyse

int comptChar=0; // variable de comptage des caractères reçus 

long seconde0=0; // mémorise dernière seconde
long delai=1; // délai entre 2 secondes en seconde = 1 seconde 

//--- fonction setup exécutée une fois au lancement
void setup(){

  // configuration des broches E/S
  //pinMode(10, OUTPUT); // IMPORTANT ++ : laisser la broche /SS(=10) en sortie - obligatoire avec librairie SD
  pinMode(53, OUTPUT); // IMPORTANT ++ : laisser la broche /SS(=53) en sortie - obligatoire avec librairie SD - pour la Mega
  
  // configuration de la communication série
  Serial.begin(115200); // utiliser le meme debit coté Terminal Serie

 //---- initialise la connexion Ethernet avec l'adresse MAC du module Ethernet, l'adresse IP Locale 
  //----  +/- l'adresse IP du serveurDNS , l'adresse IP de la passerelle internet et le masque du réseau local
  
  //Ethernet.begin(mac); // forme pour attribution automatique DHCP - utilise plus de mémoire Flash (env + 6Ko)
  Ethernet.begin(mac, ipLocal); // forme conseillée pour fixer IP fixe locale
  //Ethernet.begin(mac, ipLocal, serverDNS, passerelle, masque); // forme complète
  
  delay(1000); // donne le temps à la carte Ethernet de s'initialiser
  
  Serial.print(F("Shield Ethernet OK : L'adresse IP du shield Ethernet est : " )); 
  
  Serial.println(Ethernet.localIP()); 
  
  //---- initialise le serveur ----
  serveurHTTP.begin(); 
  Serial.println(F("Serveur Ethernet OK : Ecoute sur port 80 (http)")); 


  //--- initialisation de la carte SD
  Serial.println(F("Initialisation de la carte SD en cours..."));

  if (!SD.begin(selectSD)) { // si initialisation avec broche selectSD en tant que CS n'est pas réussie
    Serial.println(F("Echec initialisation!")); // message port Série
    return; // sort de setup()
  } // if SD begin()
  
  //--- si initialisation réussie : on se place ici :
  Serial.println(F("Initialisation reussie !")); // message port Série
  Serial.println();

  //--- message 
  Serial.println(F("Saisir une chaine de la forme FONCTION(parametre)")); // message initial


  //------- initialisation RTCLib avec le DS1307 ---- 
    Wire.begin(); // initialise I2C
    //RTC.begin(); // initialise le DS1307          
    RTC.begin(DateTime(__DATE__, __TIME__));// initialise base temps millis  

} // fin setup 

//--- fonction loop exécutée en boucle infinie
void loop(){

    //================== gestion temps réel =======================
     now = RTC.now(); // récupère l'heure courante

    //------- affichage des secondes -------------- 
      if (now.unixtime()-seconde0>=delai) { // si une seconde s'est écoulée 
    
      //Serial.println(now.unixtime(), DEC); // affiche secondes depuis le 1/1/1970
      //Serial.print("."); // visualise les secondes mais pose problème d'affichage... 
      seconde0=now.unixtime(); 

    } // fin si 1 seconde
   

    //========== gestion port série ========================    
    // gestion des chaines en réception sur le port série 
    chaineReception=utils.waitingString();// sans debug

    if (chaineReception!="") { // si une chaine a été reçue 

      utilsRTC.analyseChaine(chaineReception, timerRTC, nombreTimers); // analyse chaine reçue sur port série

     
        //=========================================================================================
           
        //--- fonction avec parametre Texte - ls (chemin)
        param=""; // RAZ param...
        param=utils.testInstructionString("ls(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Contenu du repertoire : " ));
        Serial.println(param);
        
        char paramChar[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(paramChar,param.length()+1); // récupère le param dans le tableau de char  
        //Serial.println(paramChar); // debug
        utilsSD.getContentDirSerial(paramChar,0); // appelle la fonction listing contenu - passe le String en tableau de Char

        return; // quitte la fonction = passe au loop suivant
        } // fin if param!=""
      
        //=========================================================================================
  
        //--- fonction avec parametre Texte - createfile(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("createfile(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Nouveau fichier va etre cree :"));
        Serial.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
        
          //---- test existence fichier et suppression du fichier si il existe --- 
            //char nomFichier[]="testFile.txt"; // utiliser un nom de fichier format 3.8 +++ - doit etre un tableau de char
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              Serial.println(F("------")); 
              Serial.print(F("Le fichier existe : ")); // affiche message 
              Serial.println(F("Supprimer le fichier avec remove."));   // affiche message 
              
            } // fin si fichier existe
            else { // si fichier existe pas 

              utilsSD.createFile(nomFichier,true); // crée un fichier 
              
            }// fin else
          
           return; // quitte la fonction = passe au loop suivant

      } // fin if param!=""

        //=========================================================================================

        //--- fonction avec parametre Texte - write(chemin,texte)
       param=""; // RAZ param...
       param=utils.testInstructionString("write(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Chaine recue: "));
        Serial.println(param);
        
        if (param.indexOf(",")>0) { // si 1 virgule a été trouvée
        
          String cheminFichier=param.substring(0,param.indexOf(",")); // extrait du debut à la virgule = le chemin
          Serial.println("Chemin : "+ cheminFichier); 
          
          String toWrite=param.substring(param.indexOf(",")+1); // extrait de la virgule à la fin = la chaine à écrire
          Serial.println("Chaine : " + toWrite); 
          

        char cheminFichierChar[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        cheminFichier.toCharArray(cheminFichierChar,param.length()+1); // récupère le param dans le tableau de char  

        if (SD.exists(cheminFichierChar)) { // si le fichier existe 

              Serial.print(F("Le fichier existe : ")); // affiche message 

          utilsSD.writeFile(cheminFichierChar,toWrite, true); // ajoute une chaine dans le rép

        }// fin if exists

            else { // si fichier existe pas 
            
                  Serial.println(F("Fichier inexistant : veillez le creer d'abord")); 
                  // ou bien : le fichier va etre cree... 
              
            }// fin else

        } // fin if (param.indexOf(",")


         return; // quitte la fonction = passe au loop suivant
        
      } // fin if param!=""
      
        //=========================================================================================

        //--- fonction avec parametre Texte - read(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("read(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Contenu du fichier :"));
        Serial.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
        
          //---- test existence fichier et suppression du fichier si il existe --- 
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              Serial.println(F("------")); 
              Serial.print(F("Le fichier existe : ")); // affiche message 


               utilsSD.readSerial(nomFichier,false); // affiche le contenu du fichier
 
 
            } // fin si fichier existe
            
            else { // si fichier existe pas 
            
                  Serial.println(F("Fichier inexistant.")); 
              
            }// fin else
          
           return; // quitte la fonction = passe au loop suivant

      } // fin if param!=""

        //=========================================================================================

        //--- fonction avec parametre Texte - remove(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("remove(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Chaine :"));
        Serial.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
        
          //---- test existence fichier et suppression du fichier si il existe --- 
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              Serial.println(F("------")); 
              Serial.print(F("Le fichier existe : ")); // affiche message 

              if (SD.remove(nomFichier)){ // efface le fichier
                Serial.println(F("Effacement fichier OK.")); 
              }
              else{
                Serial.println(F("Probleme effacement fichier.")); 
              }
               
 
            } // fin si fichier existe
            
            else { // si fichier existe pas 
            
                  Serial.println(F("Fichier inexistant.")); 
              
            }// fin else
          
           return; // quitte la fonction = passe au loop suivant

      } // fin if param!=""

        //=========================================================================================

         //--- fonction avec parametre Texte - lines(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("lines(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Fichier : "));
        Serial.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
        
          //---- test existence fichier et suppression du fichier si il existe --- 
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              Serial.println(F("------")); 
              Serial.print(F("Le fichier existe : ")); // affiche message 

                 int nombreLignes=utilsSD.getNumberOfLines(nomFichier,false); // fonction nombre de lignes du fichier
  
                Serial.print(F("Nombre de lignes : ")); 
                Serial.println(nombreLignes); 
                Serial.println(F("------")); 

                
            } // fin si fichier existe
            
            else { // si fichier existe pas 
            
                  Serial.println(F("Fichier inexistant.")); 
              
            }// fin else
          
           return; // quitte la fonction = passe au loop suivant

      } // fin if param!=""

        //=========================================================================================
        //--- fonction avec parametre Texte - getline(chemin,ligne)
       param=""; // RAZ param...
       param=utils.testInstructionString("getline(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Chaine recue: "));
        Serial.println(param);
        
        if (param.indexOf(",")>0) { // si 1 virgule a été trouvée
        
          String cheminFichier=param.substring(0,param.indexOf(",")); // extrait du debut à la virgule = le chemin
          Serial.println("Chemin : "+ cheminFichier); 
          
          String numberLine=param.substring(param.indexOf(",")+1); // extrait de la virgule à la fin = la chaine à écrire
          Serial.println("Ligne a extraire : " + numberLine); 
          
          int intNumberLine=utils.stringToLong(numberLine);
          //Serial.println(intNumberLine); // debug 
          

        char cheminFichierChar[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        cheminFichier.toCharArray(cheminFichierChar,param.length()+1); // récupère le param dans le tableau de char  

        if (SD.exists(cheminFichierChar)) { // si le fichier existe 

              Serial.print(F("Le fichier existe : ")); // affiche message 

             String ligne=utilsSD.getLine(cheminFichierChar,intNumberLine, false); // fonction lecture de ligne avec messages de debug
 
             Serial.print (F("Ligne = ")); 
             Serial.println(ligne); 
 
        }// fin if exists

        } // fin if (param.indexOf(",")


         return; // quitte la fonction = passe au loop suivant
        
      } // fin if param!=""

        //=========================================================================================

         //--- fonction avec parametre Texte - size(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("size(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Fichier : "));
        Serial.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
        
          //---- test existence fichier et suppression du fichier si il existe --- 
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              Serial.println(F("------")); 
              Serial.println(F("Le fichier existe : ")); // affiche message 

                int tailleOctets=utilsSD.getFileSize(nomFichier,false); // renvoie la taille en octets 
  
                Serial.print(F("Taille du fichier : ")); 
                Serial.print(tailleOctets); 
                Serial.print(F(" octets.")); 
                
                Serial.println(F("------")); 

                
            } // fin si fichier existe
            
            else { // si fichier existe pas 
            
                  Serial.println(F("Fichier inexistant.")); 
              
            }// fin else
          
           return; // quitte la fonction = passe au loop suivant

      } // fin if param!=""
      
        //=========================================================================================

        //--- fonction avec parametre Texte - testDatalog(chemin,nombreMesures)
       param=""; // RAZ param...
       param=utils.testInstructionString("testdatalog(", chaineReception, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        Serial.print(F("Chaine recue: "));
        Serial.println(param);
        
        if (param.indexOf(",")>0) { // si 1 virgule a été trouvée
        
          String cheminFichier=param.substring(0,param.indexOf(",")); // extrait du debut à la virgule = le chemin
          Serial.println("Chemin : "+ cheminFichier); 
          
          String nombreMesures=param.substring(param.indexOf(",")+1); // extrait de la virgule à la fin = la chaine à écrire
          Serial.println("Nombre de mesures : " + nombreMesures); 
          
          int intnombreMesures=utils.stringToLong(nombreMesures);
          Serial.println(intnombreMesures); // debug 
          

        char cheminFichierChar[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        cheminFichier.toCharArray(cheminFichierChar,param.length()+1); // récupère le param dans le tableau de char  

        if (SD.exists(cheminFichierChar)) { // si le fichier existe 

              Serial.println(F("Le fichier existe : ")); // affiche message 


              //---- en n'ouvrant le fichier qu'une fois et en écrivant dedans toutes les lignes d'un coup
              // c'est beaucoup plus rapide et pas de problème pour 1000 mesures... 
 
              File dataFile=SD.open(cheminFichierChar, FILE_WRITE); // crée / ouvre un objet fichier et l'ouvre en mode écriture - NOM FICHIER en 8.3 ++++
              
              if (dataFile){ // le fichier est True si créé
              
                Serial.println(F("Ouverture fichier OK")); 
                
                //----- boucle d'affichage ----
                // chaque ligne au format index;millis;mesure
                for(int i=0; i<intnombreMesures; i++) { // réalise n mesures
              
                 Serial.println(i+1); 
                  
                 dataFile.print(i+1);            
                 dataFile.print(";");            

                 dataFile.print(millis());            
                 dataFile.print(";"); 
                 
                 dataFile.print(analogRead(A3)); // affiche mesure A3
                 dataFile.println(";"); 
                 
              } // fin for                
                
                dataFile.close(); // fermeture du fichier obligatoire après accès 
                Serial.println(F("Ecriture fichier OK")); 
              } // si fichier ==True 
              else { // sinon = si probleme creation
                Serial.println(F("Probleme ouverture fichier")); 
              } // fin else datafile

        }// fin if exists
        
        else { // si fichier existe pas 
            
                  Serial.println(F("Fichier inexistant.")); 
              
            }// fin else
          

        } // fin if (param.indexOf(",")


         return; // quitte la fonction = passe au loop suivant
        
      } // fin if param!=""

   
    } // fin // si une chaine a été reçue 
  
//====================== fin gestion port série ====================



//======================= gestion client réseau =======================

// crée un objet client basé sur le client connecté au serveur
  EthernetClient client = serveurHTTP.available(); 
  
  if (client) { // si l'objet client n'est pas vide
   // le test est VRAI si le client existe 
  
   
    // message d'accueil dans le Terminal Série
      Serial.println (F("--------------------------")); 
      Serial.println (F("Client present !")); 
      Serial.println (F("Voici la requete du client:")); 

  ////////////////// Réception de la chaine de la requete //////////////////////////
  
      //-- initialisation des variables utilisées pour l'échange serveur/client
      chaineRecue=""; // vide le String de reception
      comptChar=0; // compteur de caractères en réception à 0  

    if (client.connected()) { // si le client est connecté

      /////////////////// Réception de la chaine par le réseau ////////////////////////      
      while (client.available()) { // tant que  des octets sont disponibles en lecture
      // le test est vrai si il y a au moins 1 octet disponible
       
        char c = client.read(); // l'octet suivant reçu du client est mis dans la variable c
        comptChar=comptChar+1; // incrémente le compteur de caractère reçus

        Serial.print(c); // affiche le caractère reçu dans le Terminal Série

        //--- on ne mémorise que les n premiers caractères de la requete reçue
        //--- afin de ne pas surcharger la RAM et car cela suffit pour l'analyse de la requete
        if (comptChar<=100) chaineRecue=chaineRecue+c; // ajoute le caractère reçu au String pour les N premiers caractères
        //else break; // une fois le nombre de caractères dépassés sort du while 
           
      } // --- fin while client.available = fin "tant que octet en lecture"
      
    Serial.println (F("Reception requete terminee")); 
    
    /////////////////// Affichage de la requete reçue //////////////////////
    Serial.println(F("------------ Affichage de la requete recue ------------")); // affiche le String de la requete
    Serial.println (F("Chaine prise en compte pour analyse : "));
    Serial.println(chaineRecue); // affiche le String de la requete pris en compte pour analyse

    /////////////////// Analyse de la requete reçue //////////////////////
    Serial.println(F("------------ Analyse de la requete recue ------------")); // analyse le String de la requete
    

    
    //------ analyse si la chaine reçue est une requete GET avec chaine format /&chaine= --------
    if (chaineRecue.startsWith("GET /&")) {

          //----- extraction de la chaine allant de & à =
          int indexStart=chaineRecue.indexOf("&"); 
          int indexEnd=chaineRecue.indexOf("="); 
          Serial.print (F("index debut =")); 
          Serial.println (indexStart);     
          Serial.print (F("index fin =")); 
          Serial.println (indexEnd);     
    
          chaineAnalyse=chaineRecue.substring(indexStart+1,indexEnd); // garde chaine fonction(xxxx) à partir de GET /&fonction(xxxx)=
          // substring : 1er caractère inclusif (d'où le +1) , dernier exclusif 

             // -- message debug -- 
             Serial.print (F("Chaine recue = "));
             Serial.println (chaineAnalyse);
          
             // -- analyse de la chaine à analyser -- 
             
             if (chaineAnalyse!="") { // si une chaine à analyser non vide 

                //======================================
                
                utilsRTCEthernet.analyseChaine(client, chaineAnalyse, timerRTCEthernet, nombreTimers); // analyse chaine reçue sur port ethernet pour gestion timers


                //=========================================================================================
                   
                //--- fonction avec parametre Texte - ls (chemin)
                param=""; // RAZ param...
                param=utils.testInstructionString("ls(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug
        
                if (param!="") { // si une chaine a été reçue en paramètre
        
                
                // envoi reponse à la requete Ajax
                envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client
                client.print(F("Contenu du repertoire :")); 
                client.println(param); 
                //-- une fois la réponse Ajax terminée, la fonction de callback drawData est exécutée

                //---- action à exécuter ---- 
                char paramChar[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
                
                param.toCharArray(paramChar,param.length()+1); // récupère le param dans le tableau de char  
                utilsSDEthernet.getContentDir(client, paramChar,0); // appelle la fonction listing contenu - passe le String en tableau de Char
        
                } // fin if param!=""
             
             } // fin // si une chaine analyse a été reçue 


                //=========================================================================================
          
                  //--- fonction avec parametre Texte - createfile(chemin/nom.txt)
            param=""; // RAZ param...
            param=utils.testInstructionString("createfile(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug
    
            if (param!="") { // si une chaine a été reçue en paramètre

                // envoi reponse à la requete Ajax
                envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client    
                  client.println(F("------")); 
                client.print(F("Nouveau fichier va etre cree :"));
                client.println(param);
            
            char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
            
            param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
            
              //---- test existence fichier et suppression du fichier si il existe --- 
                //char nomFichier[]="testFile.txt"; // utiliser un nom de fichier format 3.8 +++ - doit etre un tableau de char
              
                if (SD.exists(nomFichier)) { // si le fichier existe 
                
                  client.println(F("------")); 
                  client.print(F("Le fichier existe : ")); // affiche message 
                  client.println(F("Supprimer le fichier avec remove."));   // affiche message 
                  
                } // fin si fichier existe
                else { // si fichier existe pas 
    
    
                  utilsSDEthernet.createFile(client, nomFichier,true); // crée un fichier 
                  
                }// fin else
              
    
          } // fin if param!=""

             
       //=========================================================================================

        //--- fonction avec parametre Texte - write(chemin,texte)
       param=""; // RAZ param...
       param=utils.testInstructionString("write(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        // envoi reponse à la requete Ajax
        envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client   
        client.println(F("------")); 
        client.print(F("Chaine recue: "));
        client.println(param);
        
        if (param.indexOf(",")>0) { // si 1 virgule a été trouvée
        
          String cheminFichier=param.substring(0,param.indexOf(",")); // extrait du debut à la virgule = le chemin
          client.println("Chemin : "+ cheminFichier); 
          
          String toWrite=param.substring(param.indexOf(",")+1); // extrait de la virgule à la fin = la chaine à écrire
          client.println("Chaine : " + toWrite); 
          

        char cheminFichierChar[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        cheminFichier.toCharArray(cheminFichierChar,param.length()+1); // récupère le param dans le tableau de char  

        if (SD.exists(cheminFichierChar)) { // si le fichier existe 

              client.print(F("Le fichier existe : ")); // affiche message 


          utilsSDEthernet.writeFile(client, cheminFichierChar,toWrite, true); // ajoute une chaine dans le rép

        }// fin if exists

            else { // si fichier existe pas 
            
                  client.println(F("Fichier inexistant : veillez le creer d'abord")); 
                  // ou bien : le fichier va etre cree... 
              
            }// fin else

        } // fin if (param.indexOf(",")

        
      } // fin if param!=""
      
        //=========================================================================================

        //--- fonction avec parametre Texte - read(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("read(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        // envoi reponse à la requete Ajax
        envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client   
        client.println(F("------")); 
        client.print(F("Contenu du fichier :"));
        client.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
        
          //---- test existence fichier et suppression du fichier si il existe --- 
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              client.println(F("------")); 
              client.print(F("Le fichier existe : ")); // affiche message 


               utilsSDEthernet.read(client,nomFichier,false); // affiche le contenu du fichier
 
 
            } // fin si fichier existe
            
            else { // si fichier existe pas 
            
                  client.println(F("Fichier inexistant.")); 
              
            }// fin else
          
           //return; // quitte la fonction = passe au loop suivant

      } // fin if param!=""


        //=========================================================================================

        //--- fonction avec parametre Texte - remove(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("remove(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        // envoi reponse à la requete Ajax
        envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client   
        client.println(F("------")); 
        client.print(F("Chaine :"));
        client.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
        
          //---- test existence fichier et suppression du fichier si il existe --- 
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              client.println(F("------")); 
              client.print(F("Le fichier existe : ")); // affiche message 

              if (SD.remove(nomFichier)){ // efface le fichier
                client.println(F("Effacement fichier OK.")); 
              }
              else{
                client.println(F("Probleme effacement fichier.")); 
              }
               
 
            } // fin si fichier existe
            
            else { // si fichier existe pas 
            
                  client.println(F("Fichier inexistant.")); 
              
            }// fin else
          

      } // fin if param!=""

     //=========================================================================================

         //--- fonction avec parametre Texte - lines(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("lines(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug
  
        if (param!="") { // si une chaine a été reçue en paramètre

        // envoi reponse à la requete Ajax
        envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client   
        client.println(F("------")); 
        client.print(F("Fichier : "));
        client.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  

          //---- test existence fichier et suppression du fichier si il existe --- 
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              client.println(F("------")); 
              client.print(F("Le fichier existe : ")); // affiche message 

                 int nombreLignes=utilsSDEthernet.getNumberOfLines(client, nomFichier,false); // fonction nombre de lignes du fichier
  
                client.print(F("Nombre de lignes : ")); 
                client.println(nombreLignes); 
                client.println(F("------")); 

                
            } // fin si fichier existe
            
            else { // si fichier existe pas 
            
                  client.println(F("Fichier inexistant.")); 
              
            }// fin else
          

      } // fin if param!=""

        //=========================================================================================
        //--- fonction avec parametre Texte - getline(chemin,ligne)
       param=""; // RAZ param...
       param=utils.testInstructionString("getline(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        // envoi reponse à la requete Ajax
        envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client   
        client.println(F("------")); 
        client.print(F("Chaine recue: "));
        client.println(param);
        
        if (param.indexOf(",")>0) { // si 1 virgule a été trouvée
        
          String cheminFichier=param.substring(0,param.indexOf(",")); // extrait du debut à la virgule = le chemin
          client.println("Chemin : "+ cheminFichier); 
          
          String numberLine=param.substring(param.indexOf(",")+1); // extrait de la virgule à la fin = la chaine à écrire
          client.println("Ligne a extraire : " + numberLine); 
          
          int intNumberLine=utils.stringToLong(numberLine);
          

        char cheminFichierChar[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        cheminFichier.toCharArray(cheminFichierChar,param.length()+1); // récupère le param dans le tableau de char  

        if (SD.exists(cheminFichierChar)) { // si le fichier existe 

              client.print(F("Le fichier existe : ")); // affiche message 

             String ligne=utilsSDEthernet.getLine(client, cheminFichierChar,intNumberLine, false); // fonction lecture de ligne avec messages de debug
 
             client.print (F("Ligne = ")); 
             client.println(ligne); 
 
        }// fin if exists

        } // fin if (param.indexOf(",")


         //return; // quitte la fonction = passe au loop suivant
        
      } // fin if param!=""

        //=========================================================================================

         //--- fonction avec parametre Texte - size(chemin/nom.txt)
        param=""; // RAZ param...
        param=utils.testInstructionString("size(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        // envoi reponse à la requete Ajax
        envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client   
        client.println(F("------")); 
        client.print(F("Fichier : "));
        client.println(param);
        
        char nomFichier[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        param.toCharArray(nomFichier,param.length()+1); // récupère le param dans le tableau de char  
        
          //---- test existence fichier et suppression du fichier si il existe --- 
          
            if (SD.exists(nomFichier)) { // si le fichier existe 
            
              client.println(F("------")); 
              client.println(F("Le fichier existe : ")); // affiche message 

                int tailleOctets=utilsSDEthernet.getFileSize(client, nomFichier,false); // renvoie la taille en octets 
  
                client.print(F("Taille du fichier : ")); 
                client.print(tailleOctets); 
                client.print(F(" octets.")); 
                
                client.println(F("------")); 

                
            } // fin si fichier existe
            
            else { // si fichier existe pas 
            
                  client.println(F("Fichier inexistant.")); 
              
            }// fin else
          

      } // fin if param!=""

        //=========================================================================================

        //--- fonction avec parametre Texte - testdatalog(chemin,nombreMesures)
       param=""; // RAZ param...
       param=utils.testInstructionString("testdatalog(", chaineAnalyse, true); // extrait le paramètre de la chaine avec debug

        if (param!="") { // si une chaine a été reçue en paramètre

        // envoi reponse à la requete Ajax
        envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client   
        client.println(F("------")); 
        client.print(F("Chaine recue: "));
        client.println(param);
        
        if (param.indexOf(",")>0) { // si 1 virgule a été trouvée
        
          String cheminFichier=param.substring(0,param.indexOf(",")); // extrait du debut à la virgule = le chemin
          client.println("Chemin : "+ cheminFichier); 
          
          String nombreMesures=param.substring(param.indexOf(",")+1); // extrait de la virgule à la fin = la chaine à écrire
          client.println("Nombre de mesures : " + nombreMesures); 
          
          int intnombreMesures=utils.stringToLong(nombreMesures);
          client.println(intnombreMesures); // debug 
          

        char cheminFichierChar[param.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        cheminFichier.toCharArray(cheminFichierChar,param.length()+1); // récupère le param dans le tableau de char  

        if (SD.exists(cheminFichierChar)) { // si le fichier existe 

              client.println(F("Le fichier existe : ")); // affiche message 

              //---- en n'ouvrant le fichier qu'une fois et en écrivant dedans toutes les lignes d'un coup
              // c'est beaucoup plus rapide et pas de problème pour 1000 mesures... 
 
              File dataFile=SD.open(cheminFichierChar, FILE_WRITE); // crée / ouvre un objet fichier et l'ouvre en mode écriture - NOM FICHIER en 8.3 ++++
              
              if (dataFile){ // le fichier est True si créé
              
                client.println(F("Ouverture fichier OK")); 
                
                //----- boucle d'affichage ----
                // chaque ligne au format index;millis;mesure
                for(int i=0; i<intnombreMesures; i++) { // réalise n mesures
              
                 client.println(i+1); 
                  
                 dataFile.print(i+1);            
                 dataFile.print(";");            

                 dataFile.print(millis());            
                 dataFile.print(";"); 
                 
                 dataFile.println(analogRead(A3)); // affiche mesure A3 - mieux sur shieldEthernet - A0 est fixe à 1023... 
                 
              } // fin for                
                
                dataFile.close(); // fermeture du fichier obligatoire après accès 
                client.println(F("Ecriture fichier OK")); 
                
              } // si fichier ==True 
              else { // sinon = si probleme creation
                client.println(F("Probleme ouverture fichier")); 
              } // fin else datafile

        }// fin if exists
        
        else { // si fichier existe pas 
            
                  client.println(F("Fichier inexistant.")); 
              
            }// fin else
          

        } // fin if (param.indexOf(",")

        
      } // fin if param!=""

          
    } // fin if GET /&    = fin si chaine requete entre & et =
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    
    //-----------------------  si aucune chaine reconnue => envoi de la page HTML + Javascript initiale ----------------------
    else if (chaineRecue.startsWith("GET")) { // si la chaine recue commence par GET et pas une réponse précédente = on envoie page entiere

     Serial.println (F("Requete HTTP valide !"));

          envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client
     
     //--- la réponse HTML à afficher dans le navigateur

          //------ début de la page HTML ------- 
          client.println(F("<!DOCTYPE html>"));
          client.println(F("<html>"));

          //------- head = entete de la page HTML ----------          
         client.println(F("<head>"));

           client.println(F("<meta charset=\"utf-8\" />")); // fixe encodage caractères - utiliser idem dans navigateur
           client.println(F("<title>Titre</title>"));// titre de la page HTML

           //=============== bloc de code javascript ================           
             client.println(F("<!-- Début du code Javascript  -->")); 
             client.println(F("<script language=\"javascript\" type=\"text/javascript\">"));  
             client.println(F("<!--       "));

            // variables / objets globaux - a declarer avant les fonctions pour eviter problemes de portee
            client.println(F("var textarea=null;"));
            client.println(F("var textInputX=null;"));
            client.println(F(""));
            
            //client.println(F("var delai=10;"));
  
           //---------------Fonctions de gestion des événements -------------------
           
             client.println(F("function onclickButton() { // click Button ON"));

               //client.println(F("setTimeout(function () {requeteAjax(drawData);}, delai);"));
               client.println(F("requeteAjax(\"&\"+textInput.value+\"=\", drawData);")); // envoi requete avec &chaine= et fonction de gestion resultat
      
             client.println(F("} // fin onclickButton"));


           //----------------- Fonction principale éxécutée au chargement de la page ----------------
           
             client.println(F("window.onload = function () { // au chargement de la page"));

                //client.println(F("canvas = document.getElementById(\"nomCanvas\"); // declare objet canvas a partir id = nom "));
                client.println(F("textarea = document.getElementById(\"textarea\"); // declare objet canvas a partir id = nom "));
                client.println(F("textarea.value=\"\";")); // efface le contenu 
                client.println(F("textInput= document.getElementById(\"valeur\"); // declare objet champ text a partir id = nom"));
                client.println(F(""));

               //client.println(F("setTimeout(function () {requeteAjax(drawData);}, delai);"));
      
             client.println(F("} // fin onload"));
             
           //----------------------------------
           
            client.println(F("function requeteAjax(chaineIn, callback) { ")); // debut envoi requete avec chaine personnalisee
            
               client.println(F("var xhr = XMLHttpRequest(); "));

               client.println(F("xhr.open(\"GET\", chaineIn, true);")); // envoi requete avec chaine personnalisee
               client.println(F("xhr.send(null);"));

               //------ gestion de l'évènement onreadystatechange ----- 
               client.println(F("xhr.onreadystatechange = function() { "));

                 client.println(F("if (xhr.readyState == 4 && xhr.status == 200) {"));

                   client.println(F("//alert(xhr.responseText);"));
                   client.println(F("callback(xhr.responseText);"));

                 client.println(F("} // fin if "));

               client.println(F("}; // fin function onreadystatechange"));
               //------ fin gestion de l'évènement onreadystatechange ----- 
               
             client.println(F("} // fin fonction requeteAjax"));

            // ------------------------------ 

            client.println(F("function drawData(stringDataIn) { "));

              // ajoute la réponse au champ texte 
               //client.println(F("textarea.value=stringDataIn+textarea.value;")); // ajoute la chaine au début - décale vers le bas...
               //client.println(F("textarea.value=stringDataIn;")); // ajoute la chaine - efface chaine precedente
               client.println(F("textarea.value=textarea.value+stringDataIn;")); // ajoute la chaine au début - décale vers le bas...
               client.println(F("textarea.setSelectionRange(textarea.selectionEnd-1,textarea.selectionEnd-1) ;")); // se place à la fin -1 pour avant saut de ligne

               //client.println(F("alert(stringDataIn);"));
               //client.println(F("setTimeout(function () {requeteAjax(drawData);}, delai);"));
              
            client.println(F("} // fin fonction drawData"));

            // ------------------------------ 
                 
             client.println(F("//-->"));  
             client.println(F("</script>")); 
             client.println(F("<!-- Fin du code Javascript --> ")); 
             
          //=============== fin du bloc de code javascript ================
          
          client.println(F("</head>"));
          //------- fin head = fin entete de la page HTML ----------          
          
           //------- body = corps de la page HTML ----------          
           client.println("<body>");           
           
           // affiche chaines caractères simples
           //client.println(F("<CENTER>")); //pour centrer la suite de la page                
           //client.println(F("<canvas id=\"nomCanvas\" width=\"300\" height=\"300\"></canvas>"));
           //client.println(F("<br/>"));
           client.println(F("Serveur Arduino : Test envoi chaine par requete Ajax sur clic Bouton"));
           client.println(F("<br/>"));
           client.println(F("<input type=\"text\" id=\"valeur\" />"));
           client.println(F("<button type=\"button\" onclick=\"onclickButton()\">Envoyer</button>"));  
           client.println(F("<br/>"));
           client.println(F("En provenance du serveur Arduino :"));
           client.println(F("<br/>"));
           client.println(F("<textarea id=\"textarea\" rows=\"10\" cols=\"50\" > </textarea>")); // ajoute zone texte vide à la page 
           client.println(F("<br/>"));

           
           client.println(F("</body>"));           
          //------- fin body = fin corps de la page ----------          
            
         client.println(F("</html>"));
         //-------- fin de la page HTML -------      
     
    } // fin if GET
    
     else { // si la chaine recue ne commence pas par GET
     Serial.println (F("Requete HTTP non valide !"));    
    } // fin else
    
    //------ fermeture de la connexion ------ 
    
    // fermeture de la connexion avec le client après envoi réponse
    delay(1); // laisse le temps au client de recevoir la réponse 
    client.stop();
    Serial.println(F("------------ Fermeture de la connexion avec le client ------------")); // affiche le String de la requete
    Serial.println (F(""));
  
    } // --- fin if client connected
    
  } //---- fin if client ---- 


//======================= fin client réseau =============================

//==================== gestion des timers RTC =============================== 
    for (int i=0; i<nombreTimers; i++) { // défile les timer 

      //--------- gestion du timer i -----------------------    

      //-- gestion timerRTC par port série 
      timerRTC[i].service(now.unixtime(), timerEvent, i); // routine de service du timer qui appelle la fonction voulue et passe l'index... 
      // noter que le nom indiqué est le nom de la fonction a appeler par la routine service... 
      // la routine service doit recevoir un pointeur de meme format que la fonction appelée (parametres, void, etc) ici : void timerEvent (int ) 

      //--- idem mais géré par ethernet 
      timerRTCEthernet[i].service(now.unixtime(), timerEventEthernet, i); // routine de service du timer qui appelle la fonction voulue et passe l'index... - Ethernet
      // noter que le nom indiqué est le nom de la fonction a appeler par la routine service... 
      // la routine service doit recevoir un pointeur de meme format que la fonction appelée (parametres, void, etc) ici : void timerEvent (int ) 
      
    }// fin for 



} // fin loop

/////////////////////////////// Fonctions utiles pour carte SD ///////////////////////////////



//-------- gestion des timers port série ----------
//------ fonction commune appelée lors délai timer écoulé  - reçoit l'index du timer ----- 
void timerEvent(int indexIn){
 
 // fonction appelée... 
 Serial.print("====== timer event "); 
 Serial.print(indexIn); 
 Serial.println( "=========="); 
 //infoTimer(indexIn); // info sur le Timer
 
 //if (indexIn==0) { // si timer 1 - pas besoin de tester l'indice - quelqu'il soit, on exécute un code basé sur l'indice... 
 
         //----- crée le nom du fichier à partir de la date courante au format 20130m0d.tx0
         String nomFichier=String(now.year());
         if (now.month()<10) nomFichier=nomFichier+"0"+String(now.month()); else nomFichier=nomFichier+String(now.month()); // pour format 08.. 
         if (now.day()<10) nomFichier=nomFichier+"0"+String(now.day()); else nomFichier=nomFichier+String(now.day()); // pour format 08.. 
         nomFichier=nomFichier+".tx"+String(indexIn);
         
         Serial.println("Fichier data : "+ nomFichier); 

        char nomFichierChar[nomFichier.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        nomFichier.toCharArray(nomFichierChar,nomFichier.length()+1); // récupère le param dans le tableau de char  

        //-------- message selon existence du fichier ou pas ---
       if (SD.exists(nomFichierChar)) { // si le fichier existe 

              Serial.println(F("Le fichier existe : ")); // affiche message 

        }// fin if exists

        else { // si fichier existe pas 
            
                  Serial.println(F("Fichier inexistant : creation nouveau fichier.")); 
              
            }// fin else

         //---------- écriture des données dans le fichier ----------- 
               File dataFile=SD.open(nomFichierChar, FILE_WRITE); // crée / ouvre un objet fichier et l'ouvre en mode écriture - NOM FICHIER en 8.3 ++++
              
              if (dataFile){ // le fichier est True si créé
              
                Serial.println(F("Ouverture/Creation fichier OK")); 

                // chaque ligne au format index;time;mesure
                 dataFile.print(timerRTC[indexIn].compt);// index = nombre événements timerRTC
                 dataFile.print(";");                 

                 //dataFile.print(millis());            
                 dataFile.print(now.unixtime());   
                 dataFile.print(";");                  
                 //dataFile.println(analogRead(A0)); // affiche mesure A0
                 dataFile.print(analogRead(indexIn)); // affiche mesure voie index
                 dataFile.println(";");                  

                                 
                dataFile.close(); // fermeture du fichier obligatoire après accès 
                Serial.println(F("Ecriture fichier OK")); 
              } // si fichier ==True 
              else { // sinon = si probleme creation
                Serial.println(F("Probleme ouverture/creation fichier")); 
              } // fin else datafile

        
 
 //return; // sort de la fonction
 
 //} // fin si index==0
 
 
} // fin timerEvent 


//-------- gestion des timers Ethernet  ----------
//------ fonction commune appelée lors délai timer écoulé  - reçoit l'index du timer - EThernet ----- 
void timerEventEthernet(int indexIn){
 
 // fonction appelée... 
 Serial.print("====== timer event ethernet "); 
 Serial.print(indexIn); 
 Serial.println( "=========="); 
 //infoTimer(indexIn); // info sur le Timer
 
 //if (indexIn==0) { // si timer 1 - pas besoin de tester l'indice - quelqu'il soit, on exécute un code basé sur l'indice... 
 
         //----- crée le nom du fichier à partir de la date courante au format 20130m0d.tx0
         String nomFichier=String(now.year());
         if (now.month()<10) nomFichier=nomFichier+"0"+String(now.month()); else nomFichier=nomFichier+String(now.month()); // pour format 08.. 
         if (now.day()<10) nomFichier=nomFichier+"0"+String(now.day()); else nomFichier=nomFichier+String(now.day()); // pour format 08.. 
         nomFichier=nomFichier+".tx"+String(indexIn);
         
         Serial.println("Fichier data : "+ nomFichier); 

        char nomFichierChar[nomFichier.length()+1]; // tableau de char de la taille du String param+1 (caractère de fin de ligne) 
        
        nomFichier.toCharArray(nomFichierChar,nomFichier.length()+1); // récupère le param dans le tableau de char  

        //-------- message selon existence du fichier ou pas ---
       if (SD.exists(nomFichierChar)) { // si le fichier existe 

              Serial.println(F("Le fichier existe : ")); // affiche message 

        }// fin if exists

        else { // si fichier existe pas 
            
                  Serial.println(F("Fichier inexistant : creation nouveau fichier.")); 
              
            }// fin else

         //---------- écriture des données dans le fichier ----------- 
               File dataFile=SD.open(nomFichierChar, FILE_WRITE); // crée / ouvre un objet fichier et l'ouvre en mode écriture - NOM FICHIER en 8.3 ++++
              
              if (dataFile){ // le fichier est True si créé
              
                Serial.println(F("Ouverture/Creation fichier OK")); 

                // chaque ligne au format index;time;mesure
                 dataFile.print(timerRTCEthernet[indexIn].compt);// index = nombre événements timerRTCEthernet
                 dataFile.print(";");                 

                 //dataFile.print(millis());            
                 dataFile.print(now.unixtime());   
                 dataFile.print(";");                  
                 //dataFile.println(analogRead(A3)); // affiche mesure A3
                 dataFile.print(analogRead(indexIn)); // affiche mesure voie index
                 dataFile.println(";");                  
                                 
                dataFile.close(); // fermeture du fichier obligatoire après accès 
                Serial.println(F("Ecriture fichier OK")); 
              } // si fichier ==True 
              else { // sinon = si probleme creation
                Serial.println(F("Probleme ouverture/creation fichier")); 
              } // fin else datafile

        
 
 
 //} // fin si index==0
 
 
} // fin timerEvent Ethernet

//------- fonctions communes ethernet ----- 

void envoiEnteteHTTP(EthernetClient clientIn){
 
 if (clientIn) {

   //-- envoi de la réponse HTTP --- 
           clientIn.println(F("HTTP/1.1 200 OK")); // entete de la réponse : protocole HTTP 1.1 et exécution requete réussie
           clientIn.println(F("Content-Type: text/html")); // précise le type de contenu de la réponse qui suit 
           clientIn.println(F("Connnection: close")); // précise que la connexion se ferme après la réponse
           clientIn.println(); // ligne blanche 
           
           //--- envoi en copie de la réponse http sur le port série 
           Serial.println(F("La reponse HTTP suivante est envoyee au client distant :")); 
           Serial.println(F("HTTP/1.1 200 OK"));
           Serial.println(F("Content-Type: text/html"));
           Serial.println(F("Connnection: close"));

 } // fin si client
 
} // fin envoiEnteteHTTP

