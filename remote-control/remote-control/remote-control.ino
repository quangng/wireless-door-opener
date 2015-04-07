#include <TaskScheduler.h>
#include <cps_aes128.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <nrf24.h>

//Buttons - open, close, lock, unlock 
#define BUTTON_OPEN 4
#define BUTTON_CLOSE 5
#define BUTTON_LOCK 6
#define BUTTON_UNLOCK 7

//Button states
#define BUTTON_STATE_OPEN true
#define BUTTON_STATE_CLOSE false
#define BUTTON_STATE_LOCK true
#define BUTTON_STATE_UNLOCK false

#define LED_PIN 3

//Set up buttons
bool lockUnlockState = BUTTON_STATE_LOCK;
bool openCloseState = BUTTON_STATE_CLOSE;
bool lastOpenButtonState = HIGH;
bool lastCloseButtonState = HIGH;
bool lastLockButtonState = HIGH;
bool lastUnlockButtonState = HIGH;

bool messageFlag = false;

uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};

uint8_t key[] = { 0x54, 0x68, 0x69, 0x73, 0x69, 0x73, 0x61, 0x73,   //Secret key for remote-control
                  0x65, 0x63, 0x72, 0x65, 0x74, 0x6b, 0x65, 0x79};  //and base station

//Tasks used by the scheduler
void openButtonUpdate(void);
void closeButtonUpdate(void);
void lockButtonUpdate(void);
void unlockButtonUpdate(void);
void sendMessage(void);


void setup() {
  //Set up nRF24L01+ radio transceiver
  nrf24_init(9,10);   //set ce pin and csn pin
  nrf24_config(2,17); //channel: #2, payload: 17 including Null character
  nrf24_tx_address(tx_address);
  
  //Set up button pins 
  pinMode(BUTTON_LOCK, INPUT);
  pinMode(BUTTON_UNLOCK, INPUT); 
  pinMode(BUTTON_OPEN, INPUT);
  pinMode(BUTTON_CLOSE, INPUT);
  
  pinMode(LED_PIN, OUTPUT);
  
  Sch.init();  //Initialize the hybrid scheduler
  
  /*
   * use Sch.addTask(task, start_time, period, priority) to add tasks
   * task - tasks to be scheduled
   * start_time - when the task starts (ms)
   * period - repeat period of the task (ms)
   * priority - 1: mormal priority, 0: high priority
   */
  Sch.addTask(openButtonUpdate, 0, 100, 0);
  Sch.addTask(closeButtonUpdate, 20, 100, 0);
  Sch.addTask(lockButtonUpdate, 40, 100, 0);
  Sch.addTask(unlockButtonUpdate, 60, 100, 0);
  Sch.addTask(sendMessage, 80, 100, 1);
 
  Sch.start();  //Start task scheduler
}

void loop() {
  Sch.dispatchTasks();
}

//Tasks to be scheduled
//open button task. Set messageFlag whenever the "open" button is pressed and set
//the openCloseState variable to state open
void openButtonUpdate(void) {
  int reading = digitalRead(BUTTON_OPEN);
  
  if (reading != lastOpenButtonState) {
    if (reading == LOW) {
      if (lockUnlockState == BUTTON_STATE_UNLOCK) {
        openCloseState = BUTTON_STATE_OPEN;
        messageFlag = true;
        digitalWrite(3, HIGH);
      }
    }
  }
  lastOpenButtonState = reading; 
}


//close button task. Set messageFlag whenever the "close" button is presed and set
//the openCloseState variable to state close
void closeButtonUpdate(void) {
  int reading = digitalRead(BUTTON_CLOSE);
  
  if (reading != lastCloseButtonState) {
    if (reading == LOW) {
      if (lockUnlockState == BUTTON_STATE_UNLOCK) {
        openCloseState = BUTTON_STATE_CLOSE;
        messageFlag = true;
        digitalWrite(3, LOW);      
      }
    }
  }
  lastCloseButtonState = reading;  
}


//lock button task. Set lockUnlockState variable to state lock when "lock" button is pressed
void lockButtonUpdate(void) {
  int reading = digitalRead(BUTTON_LOCK);
  
  if (reading != lastLockButtonState) {
    if (reading == LOW) {
      lockUnlockState = BUTTON_STATE_LOCK;
    }
  }
  lastLockButtonState = reading;
}



//unlock button task. Set lockUnlockState variable to state lock when "unlock" button is pressed
void unlockButtonUpdate(void) {
  int reading = digitalRead(BUTTON_UNLOCK);
  
  if (reading != lastUnlockButtonState) {
    if (reading == LOW) {
      lockUnlockState = BUTTON_STATE_UNLOCK;
    }
  }
  lastUnlockButtonState = reading;
}


//send message task. This task checks whether the remote-control is lock or unlocked
//If it is unlocked and the "open" button is pressed. It will encrypt a message for opening 
//and send the encrypted message via nRF24L01+ radio transceiver to the base station.
//The same procedure applies to "close" button. If remote-control is locked, this task does nothing
void sendMessage(void) {
  if(messageFlag) {
    if (lockUnlockState == BUTTON_STATE_UNLOCK) {
      if(openCloseState == BUTTON_STATE_OPEN) {
        uint8_t message[] = "0000PES2015_open";
        aes_encrypt(message, key);
        nrf24_send(message);
        while(nrf24_isSending());
      } else {
        uint8_t message[] = "000PES2015_close";       
        aes_encrypt(message, key);
        nrf24_send(message);
        while(nrf24_isSending());      
      }
    }
    messageFlag = false;
  }
}
