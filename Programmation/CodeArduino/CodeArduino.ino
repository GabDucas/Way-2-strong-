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
  float velocite;
  float goalPWM;
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



//TODO: VOIR CHANGEMENT ET IMPLEMENTER!
// JE VEUS UN ENUM QUI RELI CA À DES TRUCS CONCRET:
enum TypeDeMode{
  E_STOP = 0,
  MANUEL = 1,
  ANTI_GRATIVE = 2,
  CALIBRATION = 3,
  STATIQUE = 4,
  CURL = 5
};
uint16_t runmode = E_STOP; // = 0 : E-stop
                 // = 1 : contrôle manuel de l'interface
                 // = 2 : contrôle automatique anti-gravité
                 // = 3 : calibration
                 // = 4 : statique à la position actuelle

//++++++++++++++++++++++++++++++++++++++++++++++VARIABLE POUR MOTEURS++++++++++++++++++++++++++++++++++++++++++++++
// PARAMS SPÉCIFIQUE À OPENCR
#define DXL_SERIAL   Serial3
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = 84; // OpenCR Board's DIR PIN.
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
const float DXL_PROTOCOL_VERSION = 2.0;

const uint8_t ID_EPAULE = 1;
const uint8_t ID_COUDE = 2;
const uint8_t ID_POIGNET = 6;
//This namespace is required to use Control table item names
using namespace ControlTableItem;

//PAS BESOIN DE MUTEXT POUR SES VARIABLES. SEULEMENT UTILISER DANS UNE TÂCHE AUCUNE CONCURRENCE POSSIBLE.
double max_PWM_epaule = 0.0;
double max_PWM_coude = 0.0;
double max_PWM_poignet = 0.0;

float zero_offset_epaule = -7.0;
float zero_offset_coude = 23.0;// ??;
float zero_offset_poignet = 123.46;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


osThreadId thread_id_interface;
osThreadId thread_id_update;
osThreadId thread_id_moteurs_controls;
//osThreadId thread_id_anti_gravite;

//TaskHandle_t antigravite_loop;

const uint8_t IDS_MOTOR[] = {ID_EPAULE, ID_COUDE, ID_POIGNET};

void setup()
{
  Serial.begin(9600);

  //++++++++++++++++SET_UP POUR MOTEUR++++++++++++++++
  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(ID_EPAULE);
  dxl.ping(ID_COUDE);
  dxl.ping(ID_POIGNET);

  dxl.torqueOff(ID_EPAULE);
  dxl.setOperatingMode(ID_EPAULE, OP_EXTENDED_POSITION);
  dxl.torqueOn(ID_EPAULE);

  dxl.torqueOff(ID_COUDE);
  dxl.setOperatingMode(ID_COUDE, OP_EXTENDED_POSITION);
  dxl.torqueOn(ID_COUDE);  

  dxl.torqueOff(ID_POIGNET);
  dxl.setOperatingMode(ID_POIGNET, OP_EXTENDED_POSITION);
  dxl.torqueOn(ID_POIGNET);

  //WIRE TABLE
  // Limit the maximum velocity in Position Control Mode. Use 0 for Max speed
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_EPAULE, 30);
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_COUDE, 30);
  dxl.writeControlTableItem(PROFILE_VELOCITY, ID_POIGNET, 30);

  dxl.writeControlTableItem(VELOCITY_LIMIT, ID_EPAULE, 1000);
  dxl.writeControlTableItem(VELOCITY_LIMIT, ID_COUDE, 1000);
  dxl.writeControlTableItem(VELOCITY_LIMIT, ID_POIGNET, 1000);

  // TODO: À TESTER:
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_EPAULE, 30);// TODO: VOIR SI CA MARCHE LOL
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_COUDE, 30);
  dxl.writeControlTableItem(PROFILE_ACCELERATION, ID_POIGNET, 30);

  dxl.writeControlTableItem(DRIVE_MODE, ID_POIGNET, 0);
  dxl.writeControlTableItem(DRIVE_MODE, ID_COUDE, 0);
  dxl.writeControlTableItem(DRIVE_MODE, ID_EPAULE, 0);


  //++++++++++++++++++++++++++++++++++++++++++++++++++

  mutex_data = xSemaphoreCreateMutex();

  exo = updateExo(exo);

  osThreadDef(interface, taskCommInterface, osPriorityNormal,0,2056);

  osThreadDef(moving_moteurs, moteurs_controls, osPriorityAboveNormal,0,2056);//changer test pour machine état moteurs PRIORITÉ BASSE 
  thread_id_moteurs_controls = osThreadCreate(osThread(moving_moteurs), NULL);
  thread_id_interface = osThreadCreate(osThread(interface), NULL);


  osKernelStart();
  //SI JAMAIS ON A BESOIN
  //xTaskCreate(test,      "comm openrb",1528,NULL,15,NULL);
  // xTaskCreate(taskCalculTorque, "calcul",     2056,NULL,0,NULL); 
}

void loop()
{  
}

///////////////////////////
// ***     TASKS     *** //
///////////////////////////
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void moteurs_controls( void const *pvParameters)
{
  (void) pvParameters;

  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint16_t runmode_temp;
  uint16_t runmode_temp_prev = 24;

  float commande_prev_epaule = 15000.0;
  float commande_prev_coude = 15000.0;
  float commande_prev_poignet = 15000.0;

  bool state = 0;

  u_int8_t delay_task = 200;
  u_int32_t count_loop_calib = 0;
  bool first = true;

  //++++++++++++++++++POUR CALIB+++++++++++++++++++++++++++++++++++++
  int wait = 5000;//ms
  int N_moyenne = 20;
  float PWM_epaule = 0.0;
  float PWM_coude = 0.0;
  float PWM_poignet = 0.0;

  u_int32_t count_curls = 0;

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  for(;;)
  {
    if( xSemaphoreTake(mutex_data,5) == pdTRUE ) 
    {
      exo_temp = updateExo(exo);
      runmode_temp = runmode;
      // Serial.println(runmode_temp);
      xSemaphoreGive(mutex_data);
    }
    // runmode_temp = ANTI_GRATIVE;

    if(runmode_temp == runmode_temp_prev)
    {
      first = false;
    }
    else
    {
      first = true;
    }

    if(runmode_temp != E_STOP)
    {
      switch(runmode_temp)
      {
        case E_STOP:
          dxl.torqueOff(ID_COUDE);
          dxl.torqueOff(ID_POIGNET);
          dxl.torqueOff(ID_EPAULE);
        break;

        case MANUEL:
          if(first)
          {
            set_mode(OP_EXTENDED_POSITION);

            dxl.writeControlTableItem(VELOCITY_LIMIT, ID_EPAULE, 100);
            dxl.writeControlTableItem(VELOCITY_LIMIT, ID_COUDE, 100);
            dxl.writeControlTableItem(VELOCITY_LIMIT, ID_POIGNET, 100);

            dxl.writeControlTableItem(PROFILE_VELOCITY, ID_EPAULE, 15);
            dxl.writeControlTableItem(PROFILE_VELOCITY, ID_COUDE, 15);
            dxl.writeControlTableItem(PROFILE_VELOCITY, ID_POIGNET, 15);

            dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule + 200);
            dxl.setGoalPWM(ID_COUDE, max_PWM_coude + 200);
            dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet + 200);
            
          }
          if(exo_temp.epaule.commandeMoteur != commande_prev_epaule || exo_temp.coude.commandeMoteur != commande_prev_coude || exo_temp.poignet.commandeMoteur != commande_prev_poignet )
          {
            set_PosGoal_deg(ID_COUDE, -exo_temp.coude.commandeMoteur + zero_offset_coude);
            set_PosGoal_deg(ID_EPAULE, -exo_temp.epaule.commandeMoteur + zero_offset_epaule);
            set_PosGoal_deg(ID_POIGNET, -exo_temp.poignet.commandeMoteur + zero_offset_poignet);
          }
        break;
        
      case ANTI_GRATIVE:

        //+++++++++++++++++++++++++++++++++++++++++ANTI GRAVITE V3+++++++++++++++++++++++++++++++++++++++++
        if(first)
        {
          set_mode(OP_VELOCITY);

          dxl.writeControlTableItem(VELOCITY_LIMIT, ID_EPAULE, 1000);
          dxl.writeControlTableItem(VELOCITY_LIMIT, ID_COUDE, 1000);
          dxl.writeControlTableItem(VELOCITY_LIMIT, ID_POIGNET, 1000);

          dxl.writeControlTableItem(PROFILE_VELOCITY, ID_EPAULE, 300);
          dxl.writeControlTableItem(PROFILE_VELOCITY, ID_COUDE, 300);
          dxl.writeControlTableItem(PROFILE_VELOCITY, ID_POIGNET, 300);

          dxl.setGoalVelocity(ID_EPAULE,0);
          dxl.setGoalVelocity(ID_COUDE,0);
          dxl.setGoalVelocity(ID_POIGNET,0);
          dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule);
          dxl.setGoalPWM(ID_COUDE, max_PWM_coude);
          dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet);
          delay(100);// POUR ASSURER QUE LE BRAS TOMBE PAS LORS DE CHANGEMENT DE MODE
        }
        //========================================================POIGNET======================================================== 
        if(abs(dxl.getPresentVelocity(ID_POIGNET)) <= 0.5)
        {
          // count_low_velocity;
          dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet);

          dxl.setGoalVelocity(ID_POIGNET,0);
          // Serial.println("POIGNET ZEROSSS");
        }

        else if(dxl.getPresentVelocity(ID_POIGNET) > 1)
        {
          dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet);

          dxl.setGoalVelocity(ID_POIGNET,50);
          // Serial.println("POIGNET BAS");
        }

        else if(dxl.getPresentVelocity(ID_POIGNET) < -1)
        {
          dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet);

          dxl.setGoalVelocity(ID_POIGNET,-50);
          // Serial.println("POIGNET HAUT");
        }
        //========================================================COUDE======================================================== 
        if(abs(dxl.getPresentVelocity(ID_COUDE)) <= 0.5)
        {
          // count_low_velocity;
          dxl.setGoalPWM(ID_COUDE, max_PWM_coude);

          dxl.setGoalVelocity(ID_COUDE,0);
          // Serial.println("POIGNET ZEROSSS");
        }

        else if(dxl.getPresentVelocity(ID_COUDE) > 1)
        {
          dxl.setGoalPWM(ID_COUDE, max_PWM_epaule/100);

          dxl.setGoalVelocity(ID_COUDE,-1);
          // Serial.println("POIGNET BAS");
        }

        else if(dxl.getPresentVelocity(ID_COUDE) < -1)
        {
          dxl.setGoalPWM(ID_COUDE, max_PWM_coude*2/3);

          dxl.setGoalVelocity(ID_COUDE,-50);
          // Serial.println("POIGNET HAUT");
        }
        //========================================================EPAULE======================================================== 
        if(abs(dxl.getPresentVelocity(ID_EPAULE)) <= 0.5)
        {
          // count_low_velocity;
          dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule);

          dxl.setGoalVelocity(ID_EPAULE,0);
          // Serial.println("POIGNET ZEROSSS");
        }

        else if(dxl.getPresentVelocity(ID_EPAULE) > 1)
        {
          dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule/200);

          dxl.setGoalVelocity(ID_EPAULE,-1);
          // Serial.println("POIGNET BAS");
        }

        else if(dxl.getPresentVelocity(ID_EPAULE) < -1)
        {
          dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule*2/3);

          dxl.setGoalVelocity(ID_EPAULE,-50);
          // Serial.println("POIGNET HAUT");
        }
        break;

        case CALIBRATION:
          if(first)
          {
            count_loop_calib = 0;
            N_moyenne = 20;
            PWM_epaule = 0.0;
            PWM_coude = 0.0;
            PWM_poignet = 0.0;
            
            set_mode(OP_EXTENDED_POSITION);

            dxl.writeControlTableItem(VELOCITY_LIMIT, ID_EPAULE, 100);
            dxl.writeControlTableItem(VELOCITY_LIMIT, ID_COUDE, 100);
            dxl.writeControlTableItem(VELOCITY_LIMIT, ID_POIGNET, 100);

            dxl.writeControlTableItem(PROFILE_VELOCITY, ID_EPAULE, 20);
            dxl.writeControlTableItem(PROFILE_VELOCITY, ID_COUDE, 20);
            dxl.writeControlTableItem(PROFILE_VELOCITY, ID_POIGNET, 20);

            dxl.setGoalPWM(ID_EPAULE, 500);
            dxl.setGoalPWM(ID_COUDE, 500);
            dxl.setGoalPWM(ID_POIGNET, 500);

            set_PosGoal_deg(ID_EPAULE, zero_offset_epaule);//VALEUR POUR 90 deg TODO: REDEFINIR 0 COMME 90 DEG
            set_PosGoal_deg(ID_COUDE, zero_offset_coude);//VALEUR POUR 90 deg 
            set_PosGoal_deg(ID_POIGNET, zero_offset_poignet);//VALEUR POUR 90 deg  
          }
          if(count_loop_calib*delay_task >= wait && count_loop_calib < ((int)(wait/delay_task) + N_moyenne))
          {
            PWM_epaule += dxl.getPresentPWM(ID_EPAULE);
            PWM_coude += dxl.getPresentPWM(ID_COUDE);
            PWM_poignet += dxl.getPresentPWM(ID_POIGNET);
          }
          if(count_loop_calib - (int)(wait/delay_task) == N_moyenne)
          {
            max_PWM_epaule = abs(PWM_epaule/N_moyenne) + 15;
            max_PWM_coude = abs(PWM_coude/N_moyenne) + 15;
            max_PWM_poignet = abs(PWM_poignet/N_moyenne) + 15;
            // Serial.print("max_PWM_epaule: ");
            // Serial.println(max_PWM_epaule); 
          }
          count_loop_calib++;
        break;

        case CURL:
        if(first)
        {
          set_mode(OP_EXTENDED_POSITION);
          dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule + 200);
          dxl.setGoalPWM(ID_COUDE, max_PWM_coude + 200);
          dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet + 200);

          set_PosGoal_deg(ID_EPAULE, 80 + zero_offset_epaule);
          set_PosGoal_deg(ID_POIGNET, zero_offset_poignet);
          set_PosGoal_deg(ID_COUDE, zero_offset_coude);

          dxl.writeControlTableItem(VELOCITY_LIMIT, ID_EPAULE, 30);
          dxl.writeControlTableItem(VELOCITY_LIMIT, ID_COUDE, 30);
          dxl.writeControlTableItem(VELOCITY_LIMIT, ID_POIGNET, 30);

          dxl.writeControlTableItem(PROFILE_VELOCITY, ID_EPAULE, 15);
          dxl.writeControlTableItem(PROFILE_VELOCITY, ID_COUDE, 15);
          dxl.writeControlTableItem(PROFILE_VELOCITY, ID_POIGNET, 15);

          count_curls = 0;
        }
        if(count_curls*delay_task >= wait)
        {
          if(dxl.getPresentPosition(ID_COUDE, UNIT_DEGREE) - zero_offset_coude >= 0)
          {
            set_PosGoal_deg(ID_COUDE, -100 + zero_offset_coude);
            set_PosGoal_deg(ID_POIGNET, -74 + zero_offset_poignet);
            set_PosGoal_deg(ID_EPAULE, -80 + zero_offset_epaule);
          }
          else if(dxl.getPresentPosition(ID_COUDE, UNIT_DEGREE) - zero_offset_coude <= -90)
          {
            set_PosGoal_deg(ID_COUDE, zero_offset_coude);
            set_PosGoal_deg(ID_POIGNET, zero_offset_poignet);
            set_PosGoal_deg(ID_EPAULE, 80 + zero_offset_epaule);
          }
        }
        count_curls++;
        break;

        case STATIQUE:

        if (first)
        {
          set_mode(OP_VELOCITY);
          dxl.setGoalVelocity(ID_EPAULE,0);
          dxl.setGoalVelocity(ID_COUDE,0);
          dxl.setGoalVelocity(ID_POIGNET,0);
          dxl.setGoalPWM(ID_EPAULE, max_PWM_epaule);
          dxl.setGoalPWM(ID_COUDE, max_PWM_coude);
          dxl.setGoalPWM(ID_POIGNET, max_PWM_poignet);
        }
        // Serial.println(state); //TODO À ENLEVER
        if ( state == 0 && (abs(max_PWM_poignet)>=abs(dxl.getPresentPWM(ID_POIGNET)) && abs(max_PWM_poignet)<=abs(dxl.getPresentPWM(ID_POIGNET)+10)  || abs(max_PWM_coude)>=abs(dxl.getPresentPWM(ID_COUDE)) && abs(max_PWM_coude)<=abs(dxl.getPresentPWM(ID_COUDE)+10) || abs(max_PWM_epaule)>=abs(dxl.getPresentPWM(ID_EPAULE)) && abs(max_PWM_poignet)<=abs(dxl.getPresentPWM(ID_EPAULE)+10)))
        {
          state = 1;

          dxl.torqueOff(ID_COUDE);
          dxl.torqueOff(ID_POIGNET);
          dxl.torqueOff(ID_EPAULE);
        }
        if ( state == 1 && abs(dxl.getPresentVelocity(ID_POIGNET)) <= 1 && abs(dxl.getPresentVelocity(ID_COUDE)) <= 1 && abs(dxl.getPresentVelocity(ID_EPAULE)) <= 1 )
        {
          state = 0;
          dxl.torqueOn(ID_COUDE);
          dxl.torqueOn(ID_POIGNET);
          dxl.torqueOn(ID_EPAULE);
        }
        break;

        default:
          dxl.torqueOff(ID_COUDE);
          dxl.torqueOff(ID_POIGNET);
          dxl.torqueOff(ID_EPAULE);
        break;
      }
    }

    runmode_temp_prev = runmode_temp;
    commande_prev_epaule = exo_temp.epaule.commandeMoteur;
    commande_prev_coude = exo_temp.coude.commandeMoteur;
    commande_prev_poignet = exo_temp.poignet.commandeMoteur;
   
    osDelay(pdMS_TO_TICKS(delay_task));
  }
}

void set_mode(int mode){
  // NICE TO HAVE:
  // PT FAUT TYPE CAST
  if ((int)dxl.readControlTableItem(OPERATING_MODE,ID_EPAULE) == mode){
    dxl.torqueOn(ID_COUDE);
    dxl.torqueOn(ID_EPAULE);
    dxl.torqueOn(ID_POIGNET);
    return;
  }
  else{
    dxl.torqueOff(ID_COUDE);

    dxl.setOperatingMode(ID_COUDE, mode);
    dxl.torqueOn(ID_COUDE);

    dxl.torqueOff(ID_EPAULE);
    dxl.setOperatingMode(ID_EPAULE, mode);
    dxl.torqueOn(ID_EPAULE);

    dxl.torqueOff(ID_POIGNET);
    dxl.setOperatingMode(ID_POIGNET, mode);
    dxl.torqueOn(ID_POIGNET);
  }
}

bool set_PosGoal_deg(const uint8_t ID, float goal){
  if(ID == ID_EPAULE)
  {
    if(goal - zero_offset_epaule > 89.0)
      goal = 89.0 + zero_offset_epaule;

    if(goal - zero_offset_epaule < -89.0)
      goal = -89.0 + zero_offset_epaule;
  }
  
  // if(ID == ID_COUDE)
  // {
  //   if(goal - zero_offset_coude > 110.0)//TODO: JSP L'ANGLE À VERIF
  //     goal = 110.0 + zero_offset_coude;

  //   if(goal - zero_offset_coude < zero_offset_coude)
  //     goal = zero_offset_coude;
  // }

  if(ID == ID_POIGNET)
  {
    if(goal - zero_offset_poignet > 74.0)//TODO: JSP L'ANGLE À VERIF
      goal = 74.0 + zero_offset_poignet;

    if(goal - zero_offset_poignet < -74.0)//TODO: JSP L'ANGLE À VERIF
      goal = -74.0 + zero_offset_poignet;
  }

  return dxl.setGoalPosition(ID, goal, UNIT_DEGREE);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void taskCommInterface(void const *pvParameters)
{
  (void) pvParameters;

  // Variables temporaires
  exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint16_t runmode_temp = 0;
  String message = "";
  String commandeActuel = "";
  int moteurActuel = 0;
  
  uint32_t start;
  uint32_t end;
  for(;;)
  {
    start=osKernelSysTick();
    // Receive global variables
    if( xSemaphoreTake(mutex_data,5) == pdTRUE ) 
    {
      exo_temp = updateExo(exo);
      runmode_temp = runmode;
      xSemaphoreGive(mutex_data);
    }

    ////// Envoi à l'interface //////
    Serial.println(String(millis()/1000) + "," + String(exo_temp.poignet.angle) + "," + String(exo_temp.coude.angle) + "," + String(exo_temp.epaule.angle) + "," +
                    String(-exo_temp.poignet.velocite) +"," + String(-exo_temp.coude.velocite) + "," + String(-exo_temp.epaule.velocite));

    ////// Réception de l'interface //////
    if(Serial.available())
    {
      message = Serial.readStringUntil('\n');

      //Défini le runmode en fonction du message envoyé par l'interface. Format: b'(mode),commandePoignet,commandeCoude,commandeEpaule
      runmode_temp = (int)message.charAt(0) - 48;


      // Si mode manuel activé, traverse chaque lettre du message afin de lire la commande. 
      if (runmode_temp == 1)
      {
        //  Serial.println(message);
        for (int i = 2; i < message.length(); i++)
        {
          if(message.charAt(i) == ',')
          {
              // Serial.println(moteurActuel);
            if(moteurActuel==0)
            {
              exo_temp.poignet.commandeMoteur = commandeActuel.toFloat();
              moteurActuel++;
              //  Serial.println(exo_temp.poignet.commandeMoteur);
            }
            else if(moteurActuel==1)
            {
              exo_temp.coude.commandeMoteur = commandeActuel.toFloat();
              moteurActuel++;
              //  Serial.println(exo_temp.coude.commandeMoteur);             
            }
            else if(moteurActuel == 2)
            {
              exo_temp.epaule.commandeMoteur = commandeActuel.toFloat();
              //  Serial.println(exo_temp.epaule.commandeMoteur);
              moteurActuel = 0;
            }
            //Lorsqu'on croise une virgule, on a trouvé la fin de la commande. On doit donc la reset et changer de moteur
            commandeActuel = "";
          }
          //Ajoute chaque caractère dans commandeActuel, SAUF si la caractère est une virgule
          else 
            commandeActuel = commandeActuel + message.charAt(i);
            // Serial.println(commandeActuel);
        }
      }
    }
      
    // Send global variables
    if( xSemaphoreTake(mutex_data,5) == pdTRUE ) 
    {
      exo = exo_temp;
      runmode = runmode_temp;
      xSemaphoreGive(mutex_data);
    }

    osDelay(pdMS_TO_TICKS(80));
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Calcul le torque, récupères les différentes données des moteurs et update la struct de l'Exo
exoSquelette updateExo(exoSquelette exo_temp)
{

  // Variables temporaires
// exoSquelette exo_temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Constantes physiques
const float cstPoignet = 0.081559331552;
const float cstCoude = 0.41533795679;
const float cstEpaule = 0.59558089415;

const float kt_gros = 15.8;
const float io_gros = 0.1542;
const float kt_petit = 0.897;
const float io_petit = 0.15315;
const float r_moteur = 15; // TO DO : à définir

  exo_temp.poignet.angle = (dxl.getPresentPosition(ID_POIGNET, UNIT_DEGREE) - zero_offset_poignet);//*3.15415592/180;
  exo_temp.coude.angle = (dxl.getPresentPosition(ID_COUDE, UNIT_DEGREE) - zero_offset_coude);//*3.15415592/180;
  exo_temp.epaule.angle = (dxl.getPresentPosition(ID_EPAULE, UNIT_DEGREE) - zero_offset_epaule);//*3.15415592/180;


  exo_temp.poignet.velocite = dxl.getPresentVelocity(ID_POIGNET, UNIT_RPM);
  exo_temp.coude.velocite = dxl.getPresentVelocity(ID_COUDE, UNIT_RPM);
  exo_temp.epaule.velocite = dxl.getPresentVelocity(ID_EPAULE, UNIT_RPM);
  

  exo_temp.poignet.torque = sin(exo_temp.poignet.angle) * cstPoignet;
  exo_temp.coude.torque = exo_temp.poignet.torque + sin(exo_temp.coude.angle) * cstCoude;
  exo_temp.epaule.torque = exo_temp.coude.torque + sin(exo_temp.epaule.angle) * cstEpaule;

  return exo_temp; 
}
