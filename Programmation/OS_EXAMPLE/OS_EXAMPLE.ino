#include <RTOS.h>


SemaphoreHandle_t xSemaphore;

osThreadId thread_id_loop;
osThreadId thread_id_led;

uint32_t cnt = 0;

static void Thread_Loop(void const *argument)
{
  (void) argument;


  for(;;)
  {
    loop();
  }
}


void setup() 
{
  Serial.begin(115200);
  xSemaphore = xSemaphoreCreateMutex();

  // define thread
  osThreadDef(THREAD_NAME_LOOP, Thread_Loop, osPriorityNormal, 0, 1024);
  osThreadDef(THREAD_NAME_LED,  Thread_Led,  osPriorityNormal, 0, 1024);

  // create thread
  thread_id_loop = osThreadCreate(osThread(THREAD_NAME_LOOP), NULL);
  thread_id_led  = osThreadCreate(osThread(THREAD_NAME_LED), NULL);

  // start kernel
  osKernelStart();

}

void loop() 
{

  Serial.print("RTOS Cnt : ");
  if( xSemaphoreTake(xSemaphore,15) == pdTRUE ) 
  {
    Serial.println(cnt);
    xSemaphoreGive(xSemaphore);       
  }
  osDelay(100);  
}

static void Thread_Led(void const *argument)
{
  (void) argument;

  for(;;)
  {
    if( xSemaphoreTake(xSemaphore,15) == pdTRUE ) 
    {
      cnt++;
      xSemaphoreGive(xSemaphore);       
    }

    osDelay(100);  
  }
}

