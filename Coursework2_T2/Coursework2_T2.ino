#include <Ticker.h>
#include <B31DGMonitor.h>
#include <stdint.h>

B31DGCyclicExecutiveMonitor monitor(0);

const int TASK1_PIN = 0;
const int TASK2_PIN = 1;
const int TASK3_PIN = 2;
const int TASK4_PIN = 3;

const int TASK6_PIN = 8;

const int BUTTON_PIN = 5;
const int TASK7_PIN = 9;
bool task7State = false;

unsigned long F1 = 0;
unsigned long F2 = 0;

SemaphoreHandle_t task3Done, task4Done;
TaskHandle_t task7handle = NULL;

void runFrame();
void task1();
void task2();
void task3();
void task4();
void task5();
void task6();
void task7();

void IRAM_ATTR buttonISR() {
    //button interupt which singles the task7 task
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(task7handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void setup() {
  Serial.begin(9600);
  //pin setups
  pinMode(TASK1_PIN, OUTPUT);
  pinMode(TASK2_PIN, OUTPUT);

  pinMode(TASK3_PIN, INPUT);
  pinMode(TASK4_PIN, INPUT);

  pinMode(TASK6_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(TASK7_PIN, OUTPUT);

  //wait until serial is avaiable
  while (!Serial)
    ;

  Serial.println("monitor starting");

  //creates mutexs for both frequancy tasks
  task3Done = xSemaphoreCreateBinary();
  task4Done = xSemaphoreCreateBinary();

  //creates tasks and sets priorities
  xTaskCreate(task1, "TASK1", 4000, NULL, 4, NULL);
  xTaskCreate(task2, "TASK2", 4000, NULL, 4, NULL);
  xTaskCreate(task3, "TASK3", 4000, NULL, 2, NULL);
  xTaskCreate(task4, "TASK4", 4000, NULL, 2, NULL);
  xTaskCreate(task5, "TASK5", 4000, NULL, 2, NULL);
  xTaskCreate(task6, "TASK6", 4000, NULL, 2, NULL);
  xTaskCreate(task7, "TASK7", 4000, NULL, 2, &task7handle);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  monitor.startMonitoring();

}

void loop() {
}


void task1(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount(); // gets current tickcount
  while(true){
    monitor.jobStarted(1);
    digitalWrite(TASK1_PIN, HIGH);
    delayMicroseconds(250);
    digitalWrite(TASK1_PIN, LOW);
    delayMicroseconds(50);
    digitalWrite(TASK1_PIN, HIGH);
    delayMicroseconds(300);
    digitalWrite(TASK1_PIN, LOW);
    monitor.jobEnded(1);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(4));
  }
}

void task2(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){
    monitor.jobStarted(2);
    digitalWrite(TASK2_PIN, HIGH);
    delayMicroseconds(100);
    digitalWrite(TASK2_PIN, LOW);
    delayMicroseconds(50);
    digitalWrite(TASK2_PIN, HIGH);
    delayMicroseconds(200);
    digitalWrite(TASK2_PIN, LOW);
    monitor.jobEnded(2);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3));
  }
}

void task3(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){
    monitor.jobStarted(3);
    //delayMicroseconds(1500);
    F1 = getFreq(TASK3_PIN);
    monitor.jobEnded(3);
    xSemaphoreGive(task3Done); //singles the mutex that F1 has new value
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
  }
}

void task4(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){
    monitor.jobStarted(4);
    //delayMicroseconds(1200);
    F2 = getFreq(TASK4_PIN);
    monitor.jobEnded(4);
    xSemaphoreGive(task4Done); //singles the mutex that F2 has new value
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
  }
}

void task5(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true){
    monitor.jobStarted(5);
    monitor.doWork();
    monitor.jobEnded(5);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
  }
}

void task6(void *pvParamters){
  while(true){
    //waits for both F1 and F2 signels before running
    xSemaphoreTake(task3Done, portMAX_DELAY);
    xSemaphoreTake(task4Done, portMAX_DELAY);

    // Sets the LED dependant on F1 + F2
    if (F1 < 800 || F2 < 600) digitalWrite(TASK6_PIN, LOW);
    digitalWrite(TASK6_PIN, (F1 + F2 > 1500));
  }
}

void task7(void *pvParameters){
  while(true){
    //waits until notified from button interupt
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    monitor.doWork();
    task7State = task7State ? 0 : 1;
    // sets LED based on state
    digitalWrite(TASK7_PIN, task7State);
  }
}

//used to get the frequancy
unsigned long getFreq(int PIN) {
  return 500000.0 / pulseIn(PIN, !digitalRead(PIN), 1500);
}
