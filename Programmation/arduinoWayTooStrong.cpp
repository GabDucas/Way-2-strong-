// Arduino MEGA projet S4

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Wire.h>

/******* *************** *********/
/******* headers ajoutés *********/
/******* *************** *********/
#include <event_groups.h>
#include <semphr.h>
// #include <SoftwareSerial.h>

/******* *************** *********/
/***** Variables globales ********/
/******* *************** *********/

//float tableauData[7];
float anglePoignet;
float angleCoude;
float angleEpaule;

float torquePoignet;
float torqueCoude;
float torqueEpaule;




EventGroupHandle_t flags;

//c'était dans le template
void taskExemple( void *pvParameters );

void taskEnvoieInterface( void *pvParameters);
void taskCalculTorque(void *pvParameters);
void taskEnvoieCommande(void *pvParameters);
void taskReceptionInterface(void *pvParameters);
void taskReceptionOpenRB(void *pvParameters);

void ISR_test();

void setup()
{

    Serial.begin(9600);
    Wire.begin();
    //attachInterrupt(digitalPinToInterrupt(PIN_ISR), ISR_test, CHANGE);

    xTaskCreate(taskExemple,"Test",   128,NULL,1,NULL);

    xTaskCreate(taskEnvoieInterface,"inter",   128,NULL,1,NULL);
    xTaskCreate(taskCalculTorque,"calcul",   128,NULL,1,NULL);
    xTaskCreate(taskEnvoieCommande,"commande",   128,NULL,1,NULL);
    xTaskCreate(taskReceptionInterface,"reception inter",   128,NULL,1,NULL);
    xTaskCreate(taskReceptionOpenRB,"reception openrb",   128,NULL,1,NULL);

    
    Serial.println("Synth prototype ready");
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

void taskEnvoieInterface( void *pvParameters)
{
  (void) pvParameters;
  for(int i = 0; i < 7; i++)
  {
    Serial.print(tableauData[i]);
    if(i!=6)
      Serial.print(",");
  }
  Serial.println("");
}

void taskCalculTorque(void *pvParameters)
{
  (void) pvParameters;
  float cstPoignet = 0.08193312;
  float cstCoude = 0.4133795679;
  float cstEpaule = 0.5955808941;

  kt_gros = 1.8;
  io_gros = 0.142;
  kt_petit = 0.897;
  io_petit = 0.131;

  torquePoignet = sin(anglePoignet) * cstPoignet;
  torqueCoude = torquePoignet + sin(angleCoude) * cstCoude;
  torqueEpaule = torqueCoude + sin(angleEpaule) * cstEpaule;


  goalCourantPoignet = (torquePoignet + kt_petit*io_petit)/io_petit;
  goalCourantCoude = (torqueCoude + kt_petit*io_petit)/io_petit;
  goalCourantEpaule = (torqueEpaule + kt_gros*io)/io_gros;

  goalTensionPoignet = goalTensionPoignet/r_moteur;
  goalTensionCoude = goalCourantCoude/r_moteur;
}

void taskEnvoieCommande(void *pvParameters)
{
  (void) pvParameters;
  Wire.beginTransmission(10); // transmit to device #10 (OpenRB)
  Wire.write("num moteur, commandeÉpaule, commandeCoude, commandePoignet");
  Wire.endTransmission();    // stop transmitting
}

void taskReceptionInterface(void *pvParameters)
{
  (void) pvParameters;
  if(Serial.available())
  {
    valeurCommande = Serial.readStringUntil('\n');
  }
}

void taskReceptionOpenRB(void *pvParameters)
{
  (void) pvParameters;
  /*
  tableauData[0] = READOPENRB
  tableauData[1] = 
  tableauData[2] = 
  tableauData[3] = 
  tableauData[4] = 
  tableauData[5] = 
  tableauData[6] = 
  */
}

///////////////////////////
// ***      ISR      *** //
///////////////////////////
 
void ISR_test()
/*
  ISR test
*/ 
{
    
}