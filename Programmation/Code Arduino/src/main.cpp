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
struct joint
{
  float angle;
  float torque;
  float goalCourant;
  float goalTension;
  float commandeMoteur;
};

struct exoSquelette
{
  joint poignet;
  joint coude;
  joint epaule;
};

exoSquelette exo;

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
  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int runmode_temp = 0;
  String message = "";
  String commandeActuel = "";
  int moteurActuel = 0;

  (void) pvParameters;
  for(;;)
  {
    // Receive global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo_temp = exo;
      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }
    
    ////// Envoi à l'interface //////
    Serial.println(millis() + "," + String(exo_temp.poignet.angle) + "," + String(exo_temp.coude.angle) + "," + String(exo_temp.epaule.angle) + "," +
                   String(exo_temp.poignet.torque) + "," + String(exo_temp.coude.torque) + "," + String(exo_temp.epaule.torque));


    ////// Réception de l'interface //////
    if(Serial.available())
    {
      message = Serial.readStringUntil('\n');
      //Défini le runmode en fonction du message envoyé par l'interface
      if(message.charAt(0) == 0)
        runmode_temp = 0;
      else if(message.charAt(0) == 1)
        runmode_temp = 1;
     else if(message.charAt(0) == 2)
        runmode_temp = 2;

      // Parsing de la commande manuelle de l'interface
      if (runmode_temp == 1)
      {
        for (unsigned int i = 2; i < message.length(); i++)
        {
          if(message.charAt(i) == ',')
          {
            if(moteurActuel==0)
              exo_temp.poignet.commandeMoteur = commandeActuel.toFloat();
            else if(moteurActuel==1)
              exo_temp.coude.commandeMoteur = commandeActuel.toFloat();
            else
              exo_temp.epaule.commandeMoteur = commandeActuel.toFloat();

            moteurActuel++;
            commandeActuel = "";
          }
          else
            commandeActuel = commandeActuel + message.charAt(i);
        }
      }
    }
      
    // Send global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo = exo_temp;
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
  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int runmode_temp = 0;

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
      exo_temp = exo;
      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }
    
    if(runmode_temp == 0) // E-stop
    {
      exo_temp.epaule.commandeMoteur = 0;
      exo_temp.coude.commandeMoteur = 0;
      exo_temp.poignet.commandeMoteur = 0;
    }
    else if(runmode_temp == 1) // Interface gère les commandes
    {
      // do nothing
    }
    else if(runmode_temp == 2) // Mode anti-gravité
    {
      exo_temp.poignet.torque = sin(exo_temp.poignet.angle) * cstPoignet;
      exo_temp.coude.torque = exo_temp.poignet.torque + sin(exo_temp.coude.angle) * cstCoude;
      exo_temp.epaule.torque = exo_temp.coude.torque + sin(exo_temp.epaule.angle) * cstEpaule;

      exo_temp.poignet.goalCourant = (exo_temp.poignet.torque + kt_petit*io_petit)/io_petit;
      exo_temp.coude.goalCourant = (exo_temp.coude.torque + kt_petit*io_petit)/io_petit;
      exo_temp.epaule.goalCourant = (exo_temp.epaule.torque + kt_gros*io_gros)/io_gros;

      exo_temp.poignet.goalTension = exo_temp.poignet.goalCourant/r_moteur;
      exo_temp.coude.goalTension = exo_temp.coude.goalCourant/r_moteur;
      
      exo_temp.poignet.commandeMoteur = exo_temp.poignet.goalTension;
      exo_temp.coude.commandeMoteur = exo_temp.coude.goalTension;
      exo_temp.epaule.commandeMoteur = exo_temp.epaule.goalCourant;
      
    }
    else if(runmode_temp == 3)
    {
      //autre méthode de controle!
    }
  
    // Send global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo = exo_temp;
      runmode = runmode_temp;
      xSemaphoreGive(mutex_data);
    }
  }
}

void taskCommICC(void *pvParameters)
{
  (void) pvParameters;

  // Variables temporaires
  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int runmode_temp = 0;
  
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
      exo_temp = exo;
      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }
    
    
    ////// Envoi au openRB //////
    Wire.beginTransmission(openRB_ID);
    
    //Transmission poignet
    stringBuffer = String(moteurPoignet_ID) + "," + String(exo_temp.poignet.commandeMoteur);
    stringBuffer.toCharArray(charBuffer, lengthBuffer); 
    Wire.write(charBuffer);

    //Transmission coude
    stringBuffer = String(moteurCoude_ID) + "," + String(exo_temp.coude.commandeMoteur);
    stringBuffer.toCharArray(charBuffer, lengthBuffer); 
    Wire.write(charBuffer);

    //Transmission epaule
    stringBuffer = String(moteurEpaule_ID) + "," + String(exo_temp.epaule.commandeMoteur);
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
    // mapper position (tick encodeurs) sur 0 à 360 degrés
    // vérifier si ajout offset est nécesssaire
    exo_temp.poignet.angle = position1; // TO DO : vérifier si position1 va avec anglePoignet
    exo_temp.coude.angle = position2;
    exo_temp.epaule.angle = position3;

    // Send global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo = exo_temp;
      runmode = runmode_temp;
      xSemaphoreGive(mutex_data);
    } 
  }
}
