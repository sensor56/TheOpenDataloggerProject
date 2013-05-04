// Timer multivoie avec RTC DS1307
// par X. HINAULT - www.mon-club-elec.fr - 04/2013
// tous droits réservés - GPL v3

#define nombreTimers 6 // définit le nombre de timers RTC à utiliser.. 

//--- inclusion des librairies utilisées 
#include <Wire.h> // lib I2C pour le DS1307

#include <RTClib.h> // lib native RTCLib pour gestion DS1307
#include <RTClibTimer.h> // lib perso pour implémentation timer avec RTCLib

#include <Utils.h> // inclusion de la librairie perso Utils pour réception chaines sur port série 
#include <UtilsRTClibTimer.h> // inclusion de lib avec fonction analyse chaine pour RTClibTimer

//--- déclaration des objets utiles --- 
// RTC_DS1307 RTC; // déclare objet représentant le DS1307
RTC_Millis RTC; // base temps avec Millis()

Utils utils; // déclare objet racine d'accès aux fonctions de la librairie Utils
RTClibTimer timerRTC[nombreTimers]; // objet timer
UtilsRTClibTimer utilsRTC; // objet racine donnant accès aux fonction pour gérer les timers par réception de chaine série

// variables utiles 
String chaineReception=""; // déclare un String
//long params[6]; // déclare un tableau de long - taille en fonction nombre max paramètres attendus

long seconde0=0; // mémorise dernière seconde
long delai=1; // délai entre 2 secondes en seconde = 1 seconde 

void setup () {
  
    Serial.begin(115200); // initialise la communicatin série
    Wire.begin(); // initialise I2C

    //RTC.begin(); // initialise le DS1307        
    RTC.begin(DateTime(__DATE__, __TIME__));// initialise base temps millis
    
    Serial.println("--- Test utilisation timer RTC (index : 0 - 5) ---");
    Serial.println("Saisir une chaine reconnue - ex : start(0,10) ");
    
} // fin setup 


void loop () {
  
    DateTime now = RTC.now(); // récupère l'heure courante
 
    //----- réception sur le port série --- 
    chaineReception=utils.waitingString();// sans debug

    if (chaineReception!="") { // si une chaine a été reçue 

      utilsRTC.analyseChaine(chaineReception, timerRTC, nombreTimers); // analyse chaine reçue sur port série

    } // fin si une chaine a été reçue 
    
    //=================== fin gestion réception sur port série ===================== 
    
    
    
    //------- affichage des secondes -------------- 
      if (now.unixtime()-seconde0>=delai) { // si une seconde s'est écoulée 
    
      //Serial.println(now.unixtime(), DEC); // affiche secondes depuis le 1/1/1970
      Serial.print("."); 
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
 Serial.print("\n====== timer event "); 
 Serial.print(indexIn); 
 Serial.println( "=========="); 
 //infoTimer(indexIn); // info sur le Timer
 
} // fin timerEvent 
