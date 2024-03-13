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

bool runmode = true; // E-stop

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
  (void) pvParameters;
  for(;;)
  {
    
    Serial.println(millis() + "," + String(angleEpaule) + "," + String(angleCoude) + "," + String(anglePoignet) + "," +
                   String(torqueEpaule) + "," + String(torqueCoude) + "," + String(torquePoignet));

    if(Serial.available())
    {
      String message = Serial.readStringUntil('\n');
      
      if(message == "E-STOP")
      {
        runmode = false;
      }
      else if(message.charAt(0) == 1)
      {
        //commande manuel
        commandeMoteurEpaule = message.substring(2, message.indexOf('\n')).toFloat();
      }
      else if(message.charAt(0) == 2)
      {
        commandeMoteurCoude = message.substring(2, message.indexOf('\n')).toFloat();
      }
      else if(message.charAt(0) == 3)
      {
        commandeMoteurPoignet = message.substring(2, message.indexOf('\n')).toFloat();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void taskCalculTorque(void *pvParameters)
{
  (void) pvParameters;

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
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
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

      xSemaphoreGive(mutex_data);
    }
  }
}

void taskCommICC(void *pvParameters)
{
  (void) pvParameters;

  float position1 = 0;
  float position2 = 0;
  float position3 = 0;

  String stringBuffer;
  const int lengthBuffer = 10;
  char charBuffer[lengthBuffer];

  for( ;; )
  {

    //////transmettre//////

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

    /////LIRE LES AFFAIRES ///////
    Wire.requestFrom(openRB_ID, 42);
    
    char c = '0';
    int currID = 1;

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
  }
}


  





