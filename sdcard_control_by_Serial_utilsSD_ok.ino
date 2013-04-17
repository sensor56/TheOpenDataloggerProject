/*
 * Copyright (c) - Mars 2013 - by Xavier HINAULT - support@mon-club-elec.fr
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 3
 * or the GNU Lesser General Public License version 3, both as
 * published by the Free Software Foundation.
 */

/* Ce code fait partie du "The Open Datalogger Project"
https://github.com/sensor56/TheOpenDataloggerProject 

Ce code permet le controle de la carte SD à partir de chanes reçues sur le port série

Les chaînes reconnues en réception sur le port série par ce code sont :
  ls(chemin) : liste les fichiers présents dans un répertoire 
  createfile(chemin/nomfichier) : crée le fichier
  write(chemin/nomfichier) : ajoute une ligne + saut de ligne à la fin du fichier
  read(chemin/nomfichier) : lit et affiche sur le port série le contenu du fichier
  remove(chemin/nomfichier) : efface le fichier
  lines(chemin/nomfichier) : renvoie le nombre de lignes du fichier
  getline(index) : affiche la ligne voulue à partir du numéro de ligne. 1ère ligne = n°1
  size(chemin/nomfichier) : renvoie la taille du fichier
  testDatalog(chemin/nomfichier, nombremesure) : crée un fichier avec le nombre de mesures voulues, pour tests. Utilise une voie analogique. 
  
Ce code utilise mes 2 librairies suivantes : 
  Ma librairie Utils qui permet la reconnaissance de chaîne et l'extraction de paramètres : 
  A télécharger ici : https://github.com/sensor56/Utils
  
  Ma librairie UtilsSD qui  implémente toute une série de fonctions en surcouche de la librairie SD 
  et qui rendent la manipulation de la carte SD plus « naturelle » pour l'utilisateur :
  A télécharger ici :  https://github.com/sensor56/UtilsSD 
  
*/


#include <SPI.h> // fichier libairie pour communication SPI utilisée par carte SD
#include <SD.h> // fichier librairie pour gestion de la carte SD
#include <Utils.h> // inclusion de la librairie Utils 
#include <UtilsSD.h> // inclusion de la librairie Utils SD


// broche de sélection de la carte SD : à adapter au shield utilisé
//const int selectSD=10; // par défaut
const int selectSD=4; // pour le shield Ethernet

// objets et variables utiles 
Utils utils; // déclare objet racine d'accès aux fonctions de la librairie Utils
UtilsSD utilsSD; // déclare objet racine d'accès aux fonctions de la librairie UtilsSD

String chaineReception=""; // déclare un String
String param=""; // déclare un String
long params[6]; // déclare un tableau de long - taille en fonction nombre max paramètres attendus
  
//--- fonction setup exécutée une fois au lancement
void setup(){

  // configuration des broches E/S
  //pinMode(10, OUTPUT); // IMPORTANT ++ : laisser la broche /SS(=10) en sortie - obligatoire avec librairie SD - pour la UNO 
  pinMode(53, OUTPUT); // IMPORTANT ++ : laisser la broche /SS(=53) en sortie - obligatoire avec librairie SD - pour la Mega
  
  // configuration de la communication série
  Serial.begin(115200); // utiliser le meme debit coté Terminal Serie

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

    
    // gestion des chaines en réception sur le port série 
    chaineReception=utils.waitingString();// sans debug

    if (chaineReception!="") { // si une chaine a été reçue 

     
        //=========================================================================================
           
        //--- fonction avec parametre Texte - ls (chemin)
        param=""; // RAZ param...
        param=utils.testInstructionString("ls(", chaineReception, true); // extrait le paramètre de la chaine avec debug
        //param=utils.testInstructionString("LED(", chaineReception); // extrait le paramètre de la chaine sans debug

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

        //--- fonction avec parametre Texte - testdatalog(chemin,nombreMesures)
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
              
                 Serial.println(i); 
                  
                 dataFile.print(i+1);            
                 dataFile.print(";");            

                 dataFile.print(millis());            
                 dataFile.print(";"); 
                 
                 dataFile.println(analogRead(A0)); // affiche mesure A0
                 
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
  

} // fin loop

///////////////////////// FIN DU CODE ////////////////////////////////////
