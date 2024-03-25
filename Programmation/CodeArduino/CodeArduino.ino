// Arduino MEGA projet S4
// Équipe 6 - GRO68 H24



///////////////////////////
// ***    Headers    *** //
///////////////////////////
#include <RTOS.h>
#include <Dynamixel2Arduino.h>

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

exoSquelette exo = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Structures FreeRTOS
SemaphoreHandle_t mutex_data;

int runmode = 0; // = 0 : E-stop
                 // = 1 : contrôle manuel de l'interface
                 // = 2 : contrôle automatique anti-gravité

const int openRB_ID = 0;        // TO DO : À définir
const int moteurPoignet_ID = 0; // TO DO : À définir
const int moteurCoude_ID = 0;   // TO DO : À définir
const int moteurEpaule_ID = 0;  // TO DO : À définir

void taskCommInterface(void *pvParameters);
void taskCalculTorque(void *pvParameters);

osThreadId thread_id_interface;
osThreadId thread_id_test;
osThreadId thread_id_test;

void setup()
{
  Serial.begin(115200);
  mutex_data = xSemaphoreCreateMutex();
  //xTaskCreate(test,      "comm openrb",128,NULL,1,NULL);
  // xTaskCreate(taskCalculTorque, "calcul",     2056,NULL,0,NULL); 

  osThreadDef(interface, taskCommInterface, osPriorityNormal,0,2056);
  osThreadDef(ttestt, test, osPriorityNormal,0,2056);

  thread_id_interface = osThreadCreate(osThread(interface), NULL);
  thread_id_test = osThreadCreate(osThread(ttestt), NULL);

  osKernelStart();
}

void loop()
{    
}

///////////////////////////
// ***     TASKS     *** //
///////////////////////////
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void test( void const *pvParameters)
{
  (void) pvParameters;

  uint32_t start;
  uint32_t end;

  for(;;)
  {
    start=osKernelSysTick();

    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo.poignet.angle = exo.poignet.angle + 1;
      exo.poignet.torque = exo.poignet.torque + 2;
      exo.coude.angle = exo.coude.angle + 3;
      exo.coude.torque = exo.coude.torque + 4;
      exo.epaule.angle = exo.epaule.angle + 5;
      exo.epaule.torque = exo.epaule.torque + 6;
      xSemaphoreGive(mutex_data);
    }

    end=osKernelSysTick();
    osDelay(pdMS_TO_TICKS(1000) - (end-start));
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void taskCommInterface(void const *pvParameters)
{
  (void) pvParameters;

  // Variables temporaires
  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int runmode_temp = 0;
  String message = "";
  String commandeActuel = "";
  int moteurActuel = 0;
  
  uint32_t start;
  uint32_t end;
  for(;;)
  {
    start=osKernelSysTick();

    // Receive global variables
    if( xSemaphoreTake(mutex_data,15) == pdTRUE ) 
    {
      exo_temp = exo;
      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }

    ////// Envoi à l'interface //////
    Serial.println(String(millis()) + "," + String(exo_temp.poignet.angle) + "," + String(exo_temp.coude.angle) + "," + String(exo_temp.epaule.angle) + "," +
                   String(exo_temp.poignet.torque) + "," + String(exo_temp.coude.torque) + "," + String(exo_temp.epaule.torque));
    

    ////// Réception de l'interface //////
    if(Serial.available())
    {
      message = Serial.readStringUntil('\n');
      
      //Défini le runmode en fonction du message envoyé par l'interface. Format: b'(mode),commandePoignet,commandeCoude,commandeEpaule
      if(message.charAt(2) == '0')
        runmode_temp = 0;
      else if(message.charAt(2) == '1')
        runmode_temp = 1;
     else if(message.charAt(2) == '2')
        runmode_temp = 2;

      // Si mode manuel activé, traverse chaque lettre du message afin de lire la commande. 
      if (runmode_temp == 1)
      {
        for (int i = 4; i < message.length(); i++)
        {
          if(message.charAt(i) == ',')
          {
            if(moteurActuel==0)
            {
              exo_temp.poignet.commandeMoteur = commandeActuel.toFloat();
            }
            else if(moteurActuel==1)
              exo_temp.coude.commandeMoteur = commandeActuel.toFloat();
            else
              exo_temp.epaule.commandeMoteur = commandeActuel.toFloat();
            
            //Lorsqu'on croise une virgule, on a trouvé la fin de la commande. On doit donc la reset et changer de moteur
            moteurActuel++;
            commandeActuel = "";
          }
          //Ajoute chaque caractère dans commandeActuel, SAUF si la caractère est une virgule
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

    end=osKernelSysTick();
    osDelay(pdMS_TO_TICKS(1000) - (end-start));
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Tâche permettant de calculer les valeurs de torque théoriquep..
void taskCalculTorque(void *pvParameters)
{
  (void) pvParameters;

  // Variables temporaires
  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int runmode_temp = 2;

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
