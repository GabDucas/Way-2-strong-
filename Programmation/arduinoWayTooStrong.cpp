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
// #include <SoftwareSerial.h>



///////////////////////////
//   Variables globales  //
///////////////////////////

// Constantes
const float cstPoignet = 0.08193312;
const float cstCoude = 0.4133795679;
const float cstEpaule = 0.5955808941;

const float kt_gros = 1.8;
const float io_gros = 0.142;
const float kt_petit = 0.897;
const float io_petit = 0.131;
const float r_moteur = ; // TO DO : à définir

// Données de calcul
float anglePoignet;
float angleCoude;
float angleEpaule;

float torquePoignet;
float torqueCoude;
float torqueEpaule;
//float tableauData[7];


// Structures FreeRTOS
EventGroupHandle_t flags;
bool runmode = true; // E-stop
SemaphoreHandle_t mutex_data = xSemaphoreCreateMutex();
int openRB_ID = 0; // TO DO : À définir

void taskEnvoieInterface( void *pvParameters);
void taskCalculTorque(void *pvParameters);
void taskEnvoieCommande(void *pvParameters);
void taskReceptionInterface(void *pvParameters);
void taskReceptionOpenRB(void *pvParameters);


void setup()
{
  Serial.begin(9600);
  Wire.begin();

  xTaskCreate(taskEnvoieInterface,   "inter",           128,NULL,1,NULL);
  xTaskCreate(taskCalculTorque,      "calcul",          128,NULL,1,NULL);
  xTaskCreate(taskEnvoieCommande,    "commande",        128,NULL,1,NULL);
  xTaskCreate(taskReceptionInterface,"reception inter", 128,NULL,1,NULL);
  xTaskCreate(taskReceptionOpenRB,   "reception openrb",128,NULL,1,NULL);    
}

void loop()
{    

}

///////////////////////////
// ***     TASKS     *** //
///////////////////////////

void taskExemple( void * pvParameters )  
 {
    (void) pvParameters;
    int x = 0;

     for( ;; )
     {
      Wire.beginTransmission(10); // transmit to device #10 (OpenRB)
      Wire.write(x);              // sends x 
      Wire.endTransmission();    // stop transmitting
      x++; // Increment x
      if (x > 5) x = 0; // `reset x once it gets 6
      vTaskDelay(1000/portTICK_PERIOD_MS);
     }
 }

void taskCommunicationSerieInterface( void *pvParameters)
{
  (void) pvParameters;
  for(;;)
  {
    Serial.println()
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

  float goalCourantPoignet = 0;
  float goalCourantCoude = 0;
  float goalCourantEpaule = 0;
  float goalTensionPoignet = 0;
  float goalTensionCoude = 0;

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

void taskCommunicationICC(void *pvParameters)
{
  (void) pvParameters;
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
    bool currID = 0;

    while (0 < Wire.available()) 
    { // loop through all char
      c = Wire.read(); // receive byte as a character
      if (c == ',') // check for motor ID and torque separator  // (c >= '0' && c <= '9')
      {
        currID += 1;
      }
      if (currID == 0)
      {
        cntrlMode = c;
      }
      if (currID == 1)
      {
        data1 = data1 * 10 + (c - '0');
      }
      if (currID == 2)
      {
        data2 = data2 * 10 + (c - '0');
      }
      if (currID == 3)
      {
        data3 = data3 * 10 + (c - '0');
      }
    }
  }
}


  





