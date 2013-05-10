// Timer multivoie avec RTC DS1307 et carte SD 
// par X. HINAULT - www.mon-club-elec.fr - 04/2013
// tous droits réservés - GPL v3

#define nombreTimers 6 // définit le nombre de timer à utiliser.. 


#include <SPI.h> // fichier libairie pour communication SPI utilisée par carte SD
#include <Wire.h> // lib I2C pour le DS1307
#include <SD.h> // fichier librairie pour gestion de la carte SD

#include <RTClib.h> // lib native RTCLib pour gestion DS1307

#include <RTClibTimer.h> // lib perso pour implémentation timer avec RTCLib
#include <Utils.h> // inclusion de la librairie Utils 
#include <UtilsSD.h> // inclusion de la librairie Utils SD
#include <UtilsRTClibTimer.h> // inclusion de lib avec fonction analyse chaine pour RTClibTimer

// broche de sélection de la carte SD : à adapter au shield utilisé
//const int selectSD=10; // par défaut - 9 si shield ethernet en meme temps
const int selectSD=4; // pour le shield Arduino

// objets et variables utiles 
Utils utils; // déclare objet racine d'accès aux fonctions de la librairie Utils
UtilsSD utilsSD; // déclare objet racine d'accès aux fonctions de la librairie UtilsSD

//RTC_DS1307 RTC; // déclare objet représentant le DS1307
RTC_Millis RTC; // base temps avec Millis()

DateTime now; // objet date/heure global 

RTClibTimer timerRTC[nombreTimers]; // objet timer
UtilsRTClibTimer utilsRTC; 

String chaineReception=""; // déclare un String
String param=""; // déclare un String
long params[6]; // déclare un tableau de long - taille en fonction nombre max paramètres attendus

long seconde0=0; // mémorise dernière seconde
long delai=1; // délai entre 2 secondes en seconde = 1 seconde 

//--- fonction setup exécutée une fois au lancement
void setup(){

  // configuration des broches E/S
  //pinMode(10, OUTPUT); // IMPORTANT ++ : laisser la broche /SS(=10) en sortie - obligatoire avec librairie SD
  pinMode(53, OUTPUT); // IMPORTANT ++ : laisser la broche /SS(=53) en sortie - obligatoire avec librairie SD - pour la Mega
  
  // configuration de la communication série
  Serial.begin(115200); // utiliser le meme debit coté Terminal Serie

    Wire.begin(); // initialise I2C
    //RTC.begin(); // initialise le DS1307        
    RTC.begin(DateTime(__DATE__, __TIME__));// initialise base temps millis

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
  
} // fin setup 

//--- fonction loop exécutée en boucle infinie
void loop(){

     now = RTC.now(); // récupère l'heure courante
    
    // gestion des chaines en réception sur le port série 
    chaineReception=utils.waitingString();// sans debug

    if (chaineReception!="") { // si une chaine a été reçue 

      utilsRTC.analyseChaine(chaineReception, timerRTC, nombreTimers); // analyse chaine reçue sur port série


     
        //=========================================================================================
           
        //--- fonction avec parametre Texte - ls (chemin)
        param=""; // RAZ param...
        param=utils.testInstructionString(chaineReception, "ls(",  true); // extrait le paramètre de la chaine avec debug
 
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
        param=utils.testInstructionString(chaineReception, "createfile(",  true); // extrait le paramètre de la chaine avec debug

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
              //SD.remove(nomFichier); // efface le fichier
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
       param=utils.testInstructionString(chaineReception, "write(",  true); // extrait le paramètre de la chaine avec debug

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
        param=utils.testInstructionString(chaineReception, "read(",  true); // extrait le paramètre de la chaine avec debug

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
        param=utils.testInstructionString(chaineReception, "remove(",  true); // extrait le paramètre de la chaine avec debug

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
        param=utils.testInstructionString(chaineReception, "lines(",  true); // extrait le paramètre de la chaine avec debug

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
       param=utils.testInstructionString( chaineReception, "getline(", true); // extrait le paramètre de la chaine avec debug

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
        param=utils.testInstructionString(chaineReception, "size(",  true); // extrait le paramètre de la chaine avec debug

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
       param=utils.testInstructionString(chaineReception, "testdatalog(",  true); // extrait le paramètre de la chaine avec debug

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
              
                 Serial.println(i); 
                  
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

    //------- affichage des secondes -------------- 
      if (now.unixtime()-seconde0>=delai) { // si une seconde s'est écoulée 
    
      //Serial.println(now.unixtime(), DEC); // affiche secondes depuis le 1/1/1970
      // Serial.print("."); // visualisation des secondes - mais potentiellement genant... 
      seconde0=now.unixtime(); 

    } // fin si 1 seconde
   

    //====== gestion des timers ======== 
    for (int i=0; i<nombreTimers; i++) { // défile les timer 

      //--------- gestion du timer i -----------------------    

      timerRTC[i].service(now.unixtime(), timerEvent, i); // routine de service du timer qui appelle la fonction voulue et passe l'index... 
      // noter que le nom indiqué est le nom de la fonction a appeler par la routine service... 
      // la routine service doit recevoir un pointeur de meme format que la fonction appelée (parametres, void, etc) ici : void timerEvent (int ) 
      
    }// fin for 



} // fin loop


//------ fonction commune appelée lors délai timer écoulé  - reçoit l'index du timer ----- 
void timerEvent(int indexIn){
 
 // fonction appelée... 
 Serial.print("====== timer event "); 
 Serial.print(indexIn); 
 Serial.println( "=========="); 
 //infoTimer(indexIn); // info sur le Timer
 
 //if (indexIn==0) { // si timer 0 - pas besoin de tester l'indice - quelqu'il soit, on exécute un code basé sur l'indice... 
 
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
                 
                 //dataFile.println(analogRead(A3)); // enregistre mesure A3
                 dataFile.print(analogRead(indexIn)); // enregistre mesure analogique voie indexIn 
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


