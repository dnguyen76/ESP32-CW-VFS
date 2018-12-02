
/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Creation d'un serveur BlueTooth that, qui une fois la connexion établie, envoie des annonces periodiques
   Ce serveur s'annonce en Bluetooth avec l'identifiant : 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   L'identifiant : 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - est utilisé pour recevoir des données ( mode "WRITE")
   L'identifiant : 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - est utilisé pour envoyer des données  ( mode "NOTIFY")

  Pour créer des services Bluetooth il faut:

   1. Creer un Serveur BLE (Bluetooth Low Energy)
   2. Creer un Service BLE
   3. Creer une Caracteristique BLE du Service
   4. Creer un descripteur BLE pour ce service
   5. Démarrer le service.
   6. Démarrer l'envoi de l'annonce du service

   rxValue contient les données reçues (only accessible inside that function).
   et txValue contient les données à envoyer .
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ===============================MORSE Declare ==============
const char* TableMorse[] = {
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //  de 0x0 à 0x07
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //  de 0x08 à 0x0F
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //  de 0x10 à 0x17
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, //  de 0x18 à 0x1F
  // space, !, ", #, $, %, &, '
  "s", "-.-.--", ".-..-.", NULL, NULL, NULL, NULL, ".----.",
  // ( ) * + , - . /
  "-.--.", "-.--.-", NULL, ".-.-.", "--..--", "-....-", ".-.-.-", "-..-.",
  // 0 1 2 3 4 5 6 7
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...",
  // 8 9 : ; < = > ?
  "---..", "----.", "---...", "-.-.-.", NULL, "-...-", NULL, "..--..",
  // @ A B C D E F G
  ".--.-.", ".-", "-...", "-.-.", "-..", ".", "..-.", "--.",
  // H I J K L M N O
  "....", "..", ".---", "-.-", ".-..", "--", "-.", "---",
  // P Q R S T U V W
  ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--",
  // X Y Z [ \ ] ^ _
  "-..-", "-.--", "--..", NULL, NULL, NULL, NULL, "..--.-",
  // ' a b c d e f g
  NULL, ".-", "-...", "-.-.", "-..", ".", "..-.", "--.",
  // h i j k l m n o
  "....", "..", ".---", "-.-", ".-..", "--", "-.", "---",
  // p q r s t u v w
  ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--",
  // x y z { | } ~ DEL
  "-..-", "-.--", "--..", NULL, NULL, NULL, NULL, NULL, // de 0x78 à 0x7F
};

const int LED = 2; // GPIO2 For MHetLive Board;   25 For Heltec Wifi LoRa Board.
const int BUZZER = 22 ; //GPIO22 connecté au Buzzer

#define ALLUME LOW
#define ETEINT HIGH
#define VRAI 1
#define FAUX 0

int volume = 2048 ; //  volume max = 2048
int freq = 1000 ;
int buflen ;
char buf[80];
char buf2[80] , buf3[80], buf4[80];
int veille = VRAI;

// 1. Un Tiret est égal à trois Points.
// 2. L’espacement entre deux éléments d’une même lettre est égal à un Point.
// 3. L’espacement entre deux lettres est égal à trois Points.
// 4. L’espacement entre deux mots est égal à sept Points.

int vitesse ;
int Point_Duree = 40 ;
int Tiret_Duree = Point_Duree * 3;
int k ;

//  Definition d'un timer pour generation signal PWM

#include <driver/ledc.h>

ledc_timer_config_t timer_conf;

// Configure a PWM channel.

ledc_channel_config_t ledc_conf;

void Point()
{
  digitalWrite(LED, ALLUME);

  //  ledc_conf.duty = volume;
  //  ledc_channel_config(&ledc_conf);
  ledc_set_duty (LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, volume);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

  //  timer_conf.freq_hz = freq;  // The frequency of the signal in Hz
  //  ledc_timer_config(&timer_conf);
  ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);

  ledc_timer_resume (LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0);

  delay(Point_Duree);

  ledc_timer_pause (LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0);

  digitalWrite(LED, ETEINT);


}

void Tiret()
{
  digitalWrite(LED, ALLUME);

  //  ledc_conf.duty = volume;
  //  ledc_channel_config(&ledc_conf);

  ledc_set_duty (LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, volume);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

  ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);

  ledc_timer_resume (LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0);


  delay(Tiret_Duree);

  ledc_timer_pause (LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0);

  digitalWrite(LED, ETEINT);


}



void EmettreTiretPoint(const char * morseCode)
{
  int i = 0;
  while (morseCode[i] != 0)
  {
    if (morseCode[i] == '.') {
      Point(); delay(Point_Duree);
    } else if (morseCode[i] == '-') {
      Tiret(); delay(Point_Duree);
    }
    else if (morseCode[i] == 's') {  // espace entre 2 mots (caractère blanc)
      digitalWrite(LED, ETEINT);
      delay(Point_Duree * 7);
    }
    i++;
  }
}

// ================================================

BLECharacteristic *pCharacteristic, *pCharacteristic2 ;

bool deviceConnected = false;
float txValue = 0;

std::string rxValue, rxValue2, rxValue3, rxValue4 ; // Chaine de caractère C++ reçue

String VolString ;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


#define CHARACTERISTIC_UUID_TR2 "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"   // Volume characteristic UUID
#define CHARACTERISTIC_UUID_TR3 "6E400005-B5A3-F393-E0A9-E50E24DCCA9E"   // Vitesse characteristic UUID
#define CHARACTERISTIC_UUID_TR4 "6E400006-B5A3-F393-E0A9-E50E24DCCA9E"   // Frequence characteristic UUID

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



class CallbacksText: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      rxValue = pCharacteristic->getValue();


      if (rxValue.length() > 0) {
        Serial.println("");
        Serial.print("*** Received : ");

        //  Pour des besoins de tests, impression sur la console Arduino du texte reçu dans la variable rxValue

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        // Si le 1er caractère reçu est 0, le programme passe en veille

        if (rxValue[0] == char('0')) veille = VRAI ;
        else veille = FAUX;
      }
    }
};



class CallbacksVol: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic2) {
      rxValue2 = pCharacteristic2->getValue();


      if (rxValue2.length()) {

        strcpy(buf2 , rxValue2.c_str());

        volume = atoi(buf2) ;
        volume *= 20 ;   //  2048
        Serial.print(" : ");
        Serial.println(volume);
      }
    }
};

class CallbacksVitesse: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic3) {
      rxValue3 = pCharacteristic3->getValue();

      if (rxValue3.length() > 0) {
        strcpy(buf3 , rxValue3.c_str());

        vitesse = atoi(buf3) ;  // 10 à 90
        Point_Duree = 100 - vitesse ;
        Tiret_Duree = Point_Duree * 3 ;

      }
    }
};


class CallbacksFreq: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic4) {
      rxValue4 = pCharacteristic4->getValue();

     
      if (rxValue4.length() > 0) {

        strcpy(buf4 , rxValue4.c_str());

        freq = atoi(buf4) ;  // 1 à 99
        freq *= 30 ;   // 30 Hz à 3000Hz
        Serial.print(" : ");
        Serial.println(freq);
      }
    }
};





void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);


  pinMode(BUZZER, OUTPUT); // Mettre en mode sortie la connexion au buzzer

  // configuration du timer pour la generation de la frequence PWM

  timer_conf.duty_resolution = LEDC_TIMER_12_BIT;  // 0 à 4095
  timer_conf.freq_hz = freq;  // The frequency of the signal in Hz
  timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
  timer_conf.timer_num = LEDC_TIMER_0;
  ledc_timer_config(&timer_conf);

  ledc_conf.channel = LEDC_CHANNEL_0;
  ledc_conf.duty = volume;                 // Impulsion de 50% (2048/4096)  volume son maximum
  ledc_conf.gpio_num = BUZZER;
  ledc_conf.intr_type = LEDC_INTR_DISABLE;
  ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_conf.timer_sel = LEDC_TIMER_0;
  ledc_channel_config(&ledc_conf);

  ledc_timer_pause (LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0);

  // Creation du périphérique BLE
  BLEDevice::init("BLE ESP32"); // création et initialisation avec BLE ESP32 comme nom de périphérique

  // Crée le Serveur BLE
  BLEServer *pServer = BLEDevice::createServer();

  if (pServer == 0)  Serial.println("Create server fails");

  pServer->setCallbacks(new MyServerCallbacks());

  // Creation du Service BLE UART
  BLEService *pService = pServer->createService(SERVICE_UUID);
  if (pService == 0)  Serial.println("Create service fails");

  // Creation de la caracteristique BLE en envoi (TX)
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  if (pService == 0)  Serial.println("Create characteristic TX fails");

  pCharacteristic->addDescriptor(new BLE2902());


  // Creation de la caracteristique BLE en réception (RX)
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  if (pService == 0)  Serial.println("Create characteristic RX fails");




  BLECharacteristic *pCharacteristic2 = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_TR2,
                                          BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
                                        );
  if (pService == 0)  Serial.println("Create characteristic RXTX fails");

 

  BLECharacteristic *pCharacteristic3 = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_TR3,
                                          BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
                                        );
  if (pService == 0)  Serial.println("Create characteristic RXTX fails");

BLECharacteristic *pCharacteristic4 = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_TR4,
                                          BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
                                        );
  if (pService == 0)  Serial.println("Create characteristic RXTX fails");

  pCharacteristic2->setValue("Hello RCN World 2 ");
  pCharacteristic3->setValue("Hello RCN World 3 ");
  pCharacteristic4->setValue("Hello RCN World 4 ");

  pCharacteristic->setCallbacks(new CallbacksText());
  pCharacteristic2->setCallbacks(new CallbacksVol());
  pCharacteristic3->setCallbacks(new CallbacksVitesse());
  pCharacteristic4->setCallbacks(new CallbacksFreq());



  // Démarrer les services

  pService->start();

  pServer->getAdvertising()->start();

  Serial.println("V0.1 Attente de la connexion d'un client Bluetooth");


  digitalWrite(LED, ETEINT);

}

// ==========================================================


void loop() {

  if (deviceConnected) {
    if (veille == FAUX ) {




      char carac; int index;


      strcpy(buf ,  rxValue.c_str());   // une copie des caractères reçus est faite dans buf
      buflen = strlen(buf);             // calcul de la longueur de la chaine de caractères

      //
      if (buflen > 0 ) {
        for ( index = 0 ; index < buflen ; index++ )    // Parcours caractère par caractère de la chaine reçue
        {
          carac = buf[index];                // le caractère à convertir en code Morse
          Serial.print(carac);
          if (carac == 0x85) carac = 'a';                   //  On remplace le caractère à par par le caractère a
          if ((carac == 0x82 || carac == 0x8A )) carac = 'e';    //  On remplace les caractères é et è par par le caractère e
          if (carac >= 0x7F) carac = '?';                  // Les caractères de code ASCII > ou = à 127 sont remplacés par  ?
          if (TableMorse[carac] != NULL ) {
            EmettreTiretPoint(TableMorse[carac]);         // Utilisation du tableau TableMorse pour obtenir le code morse du caractère
            delay(Point_Duree * 3);                   //  Attente d'une durée de 3 points entre 2 lettres d'un même mot
          }
        }
        delay(Point_Duree * 7);        // fin du dernier mot reçu, attente d'une durée de 7 points
        // iteration...
        txValue += 1.00 ; // Compte le nombre de messages morse émis !

        // Conversion de la valeur en caractères

        char txString[8]; // 8 caractères maxi
        dtostrf(txValue, 1, 0, txString); // float_val, min_width, digits_after_decimal, char_buffer

        pCharacteristic->setValue(txString);

        pCharacteristic->notify(); // Envoi de la valeur au client !

        Serial.print("\n*** Sent Value: ");
        Serial.print(txValue);
        Serial.println(" ***");

        //    Mise à jour du volume



      }
    }
    else {
      delay(1000);

    }
  }
}
