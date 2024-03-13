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



///////////////////////////
//   Variables globales  //
///////////////////////////

// Données de calcul
float anglePoignet;
float angleCoude;
float angleEpaule;

float torquePoignet;
float torqueCoude;
float torqueEpaule;

float goalCourantPoignet = 0;
float goalCourantCoude = 0;
float goalCourantEpaule = 0;

float goalTensionPoignet = 0;
float goalTensionCoude = 0;


// Structures FreeRTOS
SemaphoreHandle_t mutex_data = xSemaphoreCreateMutex();

bool runmode = true; // E-stop

const int openRB_ID = 0;        // TO DO : À définir
const int moteurPoignet_ID = 0; // TO DO : À définir
const int moteurCoude_ID = 0;   // TO DO : À définir
const int moteurEpaule_ID = 0;  // TO DO : À définir

void taskEnvoieInterface( void *pvParameters);
void taskCalculTorque(void *pvParameters);
void taskEnvoieCommande(void *pvParameters);
void taskReceptionInterface(void *pvParameters);
void taskReceptionOpenRB(void *pvParameters);


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

void taskCommunicationSerieInterface( void *pvParameters)
{
  (void) pvParameters;
  for(;;)
  {
    for(int i = 0; i < 7; i++)
    {
      Serial.print(tableauData[i]);
      if(i!=6)
        Serial.print(",");
    }

     if(Serial.available())
    {
      rundmode = Serial.readStringUntil('\n');
    }

    Serial.println("");
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
  const float io = ;       // TO DO : à définir
  const float r_moteur = ; // TO DO : à définir

  for( ;; )
  {
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      torquePoignet = sin(anglePoignet) * cstPoignet;
      torqueCoude = torquePoignet + sin(angleCoude) * cstCoude;
      torqueEpaule = torqueCoude + sin(angleEpaule) * cstEpaule;

      goalCourantPoignet = (torquePoignet + kt_petit*io_petit)/io_petit;
      goalCourantCoude = (torqueCoude + kt_petit*io_petit)/io_petit;
      goalCourantEpaule = (torqueEpaule + kt_gros*io)/io_gros;

      goalTensionPoignet = goalTensionPoignet/r_moteur;
      goalTensionCoude = goalCourantCoude/r_moteur;

      xSemaphoreGive(mutex_data);
    }
  }
}

void taskCommICC(void *pvParameters)
{
  (void) pvParameters;
  for( ;; )
  {
    //mettre le nombre de bit à lire comme du monde
    Wire.requestFrom(openRB_ID, 8);
    Wire.beginTransmission(openRB_ID); // transmit to device #10 (OpenRB)
    Wire.write("num moteur, commandeÉpaule, commandeCoude, commandePoignet");
    Wire.endTransmission();    // stop transmitting


  }


}


