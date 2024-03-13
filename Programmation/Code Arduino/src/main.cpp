// Arduino MEGA projet S4
// Équipe 6 - GRO68 H24



///////////////////////////
// ***    Headers    *** //
///////////////////////////
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <event_groups.h>
#include <semphr.h>
#include <SoftwareSerial.h>
#include <floatToString.h>



///////////////////////////
//   Variables globales  //
///////////////////////////

// Données de calcul
float anglePoignet = 0;
float angleCoude = 0;
float angleEpaule = 0;

float torquePoignet = 0;
float torqueCoude = 0;
float torqueEpaule = 0;

float goalCourantPoignet = 0;
float goalCourantCoude = 0;
float goalCourantEpaule = 0;
float goalTensionPoignet = 0;
float goalTensionCoude = 0;

float commandeMoteurEpaule = 0;
float commandeMoteurCoude = 0;
float commandeMoteurPoignet = 0;

// Structures FreeRTOS
SemaphoreHandle_t mutex_data = xSemaphoreCreateMutex();

int runmode = 0; // = 0 : E-stop
                 // = 1 : contrôle manuel de l'interface
                 // = 2 : contrôle automatique anti-gravité

const int openRB_ID = 0;        // TO DO : À définir
const int moteurPoignet_ID = 0; // TO DO : À définir
const int moteurCoude_ID = 0;   // TO DO : À définir
const int moteurEpaule_ID = 0;  // TO DO : À définir

void taskCommICC( void *pvParameters);
void taskCommInterface(void *pvParameters);
void taskCalculTorque(void *pvParameters);

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  xTaskCreate(taskCommICC,      "comm openrb",128,NULL,1,NULL);
  xTaskCreate(taskCommInterface,"comm inter", 128,NULL,1,NULL);
  xTaskCreate(taskCalculTorque, "calcul",     128,NULL,1,NULL);    
}

void loop()
{    

}

///////////////////////////
// ***     TASKS     *** //
///////////////////////////

void taskCommInterface( void *pvParameters)
{
  // Variables temporaires
  float anglePoignet_temp;
  float angleCoude_temp;
  float angleEpaule_temp;

  float torquePoignet_temp;
  float torqueCoude_temp;
  float torqueEpaule_temp;

  float commandeMoteurPoignet_temp;
  float commandeMoteurCoude_temp;
  float commandeMoteurEpaule_temp;
  
  int runmode_temp;
  String message;

  (void) pvParameters;
  for(;;)
  {
    // Receive global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      anglePoignet_temp = anglePoignet;
      angleCoude_temp = angleCoude;
      angleEpaule_temp = angleEpaule;

      torquePoignet_temp = torquePoignet;
      torqueCoude_temp = torqueCoude;
      torqueEpaule_temp = torqueEpaule;

      commandeMoteurPoignet_temp = commandeMoteurPoignet;
      commandeMoteurCoude_temp = commandeMoteurCoude;
      commandeMoteurEpaule_temp = commandeMoteurEpaule;

      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }
    
    ////// Envoi à l'interface //////
    Serial.println(millis() + "," + String(angleEpaule_temp) + "," + String(angleCoude_temp) + "," + String(anglePoignet_temp) + "," +
                   String(torqueEpaule_temp) + "," + String(torqueCoude_temp) + "," + String(torquePoignet_temp));


    ////// Réception de l'interface //////
    if(Serial.available())
    {
      message = Serial.readStringUntil('\n');
      //Défini le runmode en fonction du message envoyé par l'interface
      if(message.charAt(0) == 0)
      {
        runmode_temp = 0;
      }
      else if(message.charAt(0) == 1)
      {
        runmode_temp = 1;
      }
     else if(message.charAt(0) == 2)
      {
        runmode_temp = 2;
      }

      //S'assure du mode manuel, et regarde le numéro du moteur à envoyer une commande 
      //(message: 1,3,commande): mode manuel, moteur épaule, commande à envoyer à l'épaule

      if(message.charAt(2) == 1 && runmode_temp == 1)
      {
        commandeMoteurPoignet_temp = message.substring(2, message.indexOf('\n')).toFloat();
      }
      else if(message.charAt(2) == 2 && runmode_temp == 1)
      {
        commandeMoteurCoude_temp = message.substring(2, message.indexOf('\n')).toFloat();
      }
      else if(message.charAt(2) == 3 && runmode_temp == 1)
      {
        commandeMoteurEpaule_temp = message.substring(2, message.indexOf('\n')).toFloat();
      }
    }
      
    // Send global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      anglePoignet = anglePoignet_temp;
      angleCoude = angleCoude_temp;
      angleEpaule = angleEpaule_temp;

      torquePoignet = torquePoignet_temp;
      torqueCoude = torqueCoude_temp;
      torqueEpaule = torqueEpaule_temp;

      commandeMoteurPoignet = commandeMoteurPoignet_temp;
      commandeMoteurCoude = commandeMoteurCoude_temp;
      commandeMoteurEpaule = commandeMoteurEpaule_temp;

      runmode = runmode_temp;
      xSemaphoreGive(mutex_data);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void taskCalculTorque(void *pvParameters)
{
  (void) pvParameters;

  // Variables temporaires
  float anglePoignet_temp;
  float angleCoude_temp;
  float angleEpaule_temp;

  float torquePoignet_temp;
  float torqueCoude_temp;
  float torqueEpaule_temp;

  float commandeMoteurPoignet_temp;
  float commandeMoteurCoude_temp;
  float commandeMoteurEpaule_temp;
  
  int runmode_temp;

  // Constantes physiques
  const float cstPoignet = 0.08193312;
  const float cstCoude = 0.4133795679;
  const float cstEpaule = 0.5955808941;

  const float kt_gros = 1.8;
  const float io_gros = 0.142;
  const float kt_petit = 0.897;
  const float io_petit = 0.131;
  const float r_moteur = 1; // TO DO : à définir

  for( ;; )
  {
    
    // Receive global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      anglePoignet_temp = anglePoignet;
      angleCoude_temp = angleCoude;
      angleEpaule_temp = angleEpaule;

      torquePoignet_temp = torquePoignet;
      torqueCoude_temp = torqueCoude;
      torqueEpaule_temp = torqueEpaule;

      commandeMoteurPoignet_temp = commandeMoteurPoignet;
      commandeMoteurCoude_temp = commandeMoteurCoude;
      commandeMoteurEpaule_temp = commandeMoteurEpaule;

      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }
    
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      if(runmode_temp == 0) // E-stop
      {
        commandeMoteurEpaule_temp = 0;
        commandeMoteurCoude_temp = 0;
        commandeMoteurPoignet_temp = 0;
      }
      else if(runmode_temp == 1) // Interface gère les commandes
      {
        // do nothing
      }
      else if(runmode_temp == 2) // Mode anti-gravité
      {
        torquePoignet = sin(anglePoignet) * cstPoignet;
        torqueCoude = torquePoignet + sin(angleCoude) * cstCoude;
        torqueEpaule = torqueCoude + sin(angleEpaule) * cstEpaule;

        goalCourantPoignet = (torquePoignet + kt_petit*io_petit)/io_petit;
        goalCourantCoude = (torqueCoude + kt_petit*io_petit)/io_petit;
        goalCourantEpaule = (torqueEpaule + kt_gros*io_gros)/io_gros;

        goalTensionPoignet = goalCourantPoignet/r_moteur;
        goalTensionCoude = goalCourantCoude/r_moteur;

        commandeMoteurEpaule = goalCourantEpaule;
        commandeMoteurCoude = goalTensionCoude;
        commandeMoteurPoignet = goalTensionPoignet;
      }
      else if(runmode_temp == 3)
      {
        //autre méthode de controle!
      }
      xSemaphoreGive(mutex_data);
    }
  
    // Send global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      anglePoignet = anglePoignet_temp;
      angleCoude = angleCoude_temp;
      angleEpaule = angleEpaule_temp;

      torquePoignet = torquePoignet_temp;
      torqueCoude = torqueCoude_temp;
      torqueEpaule = torqueEpaule_temp;

      commandeMoteurPoignet = commandeMoteurPoignet_temp;
      commandeMoteurCoude = commandeMoteurCoude_temp;
      commandeMoteurEpaule = commandeMoteurEpaule_temp;

      runmode = runmode_temp;
      xSemaphoreGive(mutex_data);
    }
  
  }
}

void taskCommICC(void *pvParameters)
{
  (void) pvParameters;

  // Variables temporaires
  float anglePoignet_temp;
  float angleCoude_temp;
  float angleEpaule_temp;

  float torquePoignet_temp;
  float torqueCoude_temp;
  float torqueEpaule_temp;

  float commandeMoteurPoignet_temp;
  float commandeMoteurCoude_temp;
  float commandeMoteurEpaule_temp;
  
  int runmode_temp;

  float position1 = 0;
  float position2 = 0;
  float position3 = 0;

  String stringBuffer;
  const int lengthBuffer = 10;
  char charBuffer[lengthBuffer];

  char c = '0';
  int currID = 1;

  for( ;; )
  {

    // Receive global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      anglePoignet_temp = anglePoignet;
      angleCoude_temp = angleCoude;
      angleEpaule_temp = angleEpaule;

      torquePoignet_temp = torquePoignet;
      torqueCoude_temp = torqueCoude;
      torqueEpaule_temp = torqueEpaule;

      commandeMoteurPoignet_temp = commandeMoteurPoignet;
      commandeMoteurCoude_temp = commandeMoteurCoude;
      commandeMoteurEpaule_temp = commandeMoteurEpaule;

      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }
    
    
    ////// Envoi au openRB //////
    Wire.beginTransmission(openRB_ID);
    
    //Transmission poignet
    stringBuffer = String(moteurPoignet_ID) + "," + String(commandeMoteurPoignet);
    stringBuffer.toCharArray(charBuffer, lengthBuffer); 
    Wire.write(charBuffer);

    //Transmission coude
    stringBuffer = String(moteurCoude_ID) + "," + String(commandeMoteurCoude);
    stringBuffer.toCharArray(charBuffer, lengthBuffer); 
    Wire.write(charBuffer);

    //Transmission epaule
    stringBuffer = String(moteurEpaule_ID) + "," + String(commandeMoteurEpaule);
    stringBuffer.toCharArray(charBuffer, lengthBuffer); 
    Wire.write(charBuffer);
    
    Wire.endTransmission();


    ///// Réception du openRB ///////
    
    // Parsing
    Wire.requestFrom(openRB_ID, 32);
    
    c = '0';
    currID = 1;

    while (0 < Wire.available()) 
    { // loop through all char
      c = Wire.read(); // receive byte as a character
      if (c == ',') // check for motor ID and torque separator  // (c >= '0' && c <= '9')
      {
        currID += 1;
      }
      else if (currID == 1)
      {
        position1 = position1 * 10 + (c - '0');
      }
      else if (currID == 2)
      {
        position2 = position2 * 10 + (c - '0');
      }
      else if (currID == 3)
      {
        position3 = position3 * 10 + (c - '0');
      }
    }

    // Conversion
    // mapper valeur encodeur sur 0 à 360 degrés
    // vérifier si ajout offset est nécesssaire

    // Storing dans var globales
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      anglePoignet = position1; // TO DO : vérifier si position1 va avec anglePoignet
      angleCoude = position2;
      angleEpaule = position3;
      xSemaphoreGive(mutex_data);
    }
    // Send global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      anglePoignet = anglePoignet_temp;
      angleCoude = angleCoude_temp;
      angleEpaule = angleEpaule_temp;

      torquePoignet = torquePoignet_temp;
      torqueCoude = torqueCoude_temp;
      torqueEpaule = torqueEpaule_temp;

      commandeMoteurPoignet = commandeMoteurPoignet_temp;
      commandeMoteurCoude = commandeMoteurCoude_temp;
      commandeMoteurEpaule = commandeMoteurEpaule_temp;

      runmode = runmode_temp;
      xSemaphoreGive(mutex_data);
    } 
  }
}
