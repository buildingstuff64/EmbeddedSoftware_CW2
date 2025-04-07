#include <Ticker.h>
#include <B31DGMonitor.h>
#include <stdint.h>

B31DGCyclicExecutiveMonitor monitor(5929 - 4415);

// defines the schedual as a 2d array
int frameInstruction[30][5] = {
  { 1, 1, 0, 0, 1 },
  { 0, 0, 1, 0, 0 },
  { 0, 1, 0, 1, 0 },
  { 1, 1, 0, 0, 0 },
  { 1, 0, 0, 0, 1 },
  { 0, 1, 0, 0, 1 },
  { 1, 1, 0, 0, 0 },
  { 0, 0, 1, 0, 0 },
  { 0, 1, 0, 1, 0 },
  { 1, 1, 0, 0, 1 },
  { 1, 1, 0, 0, 1 },
  { 0, 0, 1, 0, 0 },
  { 1, 1, 0, 0, 0 },
  { 0, 0, 0, 1, 1 },
  { 1, 1, 0, 0, 0 },
  { 0, 1, 0, 0, 1 },
  { 0, 0, 1, 0, 0 },
  { 1, 1, 0, 0, 0 },
  { 0, 1, 0, 1, 0 },
  { 1, 0, 0, 0, 1 },
  { 0, 1, 0, 1, 0 },
  { 1, 1, 0, 0, 1 },
  { 0, 0, 1, 0, 0 },
  { 1, 1, 0, 0, 0 },
  { 0, 1, 0, 0, 1 },
  { 1, 0, 0, 0, 1 },
  { 1, 1, 0, 0, 0 },
  { 0, 1, 0, 1, 0 },
  { 0, 0, 1, 0, 0 },
  { 1, 1, 0, 0, 1 }
};

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

Ticker ticker;
int framecounter = 0;
int frameSuccess[30][5]; // used for debugging to see if the frame finished all tasks

void runFrame();

// button interupt for task 7
void ISR_Button(void){
  // sets LED dependant on the state of task7 and toggles the state
  task7State = task7State ? false : true;
  digitalWrite(TASK7_PIN, task7State);
  //runs dowork
  runJob(5);

  Serial.print("Button pressed");
}

void setup() {
  //sets the pins
  Serial.begin(9600);
  pinMode(TASK1_PIN, OUTPUT);
  pinMode(TASK2_PIN, OUTPUT);

  pinMode(TASK3_PIN, INPUT);
  pinMode(TASK4_PIN, INPUT);

  pinMode(TASK6_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(TASK7_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ISR_Button, FALLING);

  while (!Serial)
    ;

  Serial.println("monitor starting");
  ticker.attach_ms(2, runFrame);
  int start = monitor.startMonitoring();
}

void loop() {
}

void runFrame() {

  //runs the schedual 
  for (int i = 0; i < 5; i++) {
    if (frameInstruction[framecounter%30][i] == 1) {
      runJob(i);
    }
    frameSuccess[framecounter % 30][i] = 1;
  }

  // check for F1 + F2 every 10 frames
  if (framecounter % 10 == 0) {
    if (F1 + F2 > 1500) {
      digitalWrite(TASK6_PIN, HIGH);
    } else {
      digitalWrite(TASK6_PIN, LOW);
    }
  }

  //used for debugging to see which frame failed
  // if (framecounter % 30 == 0){
  //   for (int i=0; i < 30; i++){
  //     for (int j=0; j < 5; j++){
  //       if (frameSuccess[i][j] != 1){
  //         Serial.print("frame :");
  //         Serial.print(i);
  //         Serial.print(" task");
  //         Serial.println(j);
  //       }
  //     }
  //   }
  // }

  framecounter++;
}

// run job with monitor
void runJob(int i) {
  monitor.jobStarted(i + 1);
  switch (i) {
    case 0:
      task1();
      break;
    case 1:
      task2();
      break;
    case 2:
      task3();
      break;
    case 3:
      task4();
      break;
    case 4:
      task5();
      break;
    default:
      break;
  }
  monitor.jobEnded(i + 1);
}

int testAvgJobTime(int job, int count) {
  int total = 0;
  for (int i = 0; i < count; i++) {
    int t = micros();
    runJob(job);
    total += micros() - t;
  }
  return total / count;
}

void task1() {
  digitalWrite(TASK1_PIN, HIGH);
  delayMicroseconds(250);
  digitalWrite(TASK1_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(TASK1_PIN, HIGH);
  delayMicroseconds(300);
  digitalWrite(TASK1_PIN, LOW);
}

void task2() {
  digitalWrite(TASK2_PIN, HIGH);
  delayMicroseconds(100);
  digitalWrite(TASK2_PIN, LOW);
  delayMicroseconds(50);
  digitalWrite(TASK2_PIN, HIGH);
  delayMicroseconds(200);
  digitalWrite(TASK2_PIN, LOW);
}

void task3() {
  delayMicroseconds(1500);
  //F1 = getFreq(TASK3_PIN);
}

void task4() {
  delayMicroseconds(1200);
  //F2 = getFreq(TASK4_PIN);
}

void task5() {
  monitor.doWork();
  //delayMicroseconds(500);
}

unsigned long getFreq(int PIN) {
  return 500000.0 / pulseIn(PIN, HIGH, 1600);
}
