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
    Serial.println();
    if(Serial.available())
    {
      runmode = Serial.readStringUntil('\n');
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

      goalTensionPoignet = goalTensionPoignet/r_moteur;
      goalTensionCoude = goalCourantCoude/r_moteur;

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

  for( ;; )
  {


    //////transmettre//////
    //mettre le nombre de bit à lire comme du monde
    Wire.beginTransmission(openRB_ID); // transmit to device #10 (OpenRB)
    Wire.write("1, commandeÉpaule, commandeCoude, commandePoignet");
    Wire.endTransmission();    // stop transmitting

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


  





