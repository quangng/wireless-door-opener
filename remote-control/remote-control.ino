/*
University of Turku
Master's degree in Embedded Computing
Cyber Physical Systems course
Project: Wireless door opener
Author: Vu Nguyen <quangngmetro@gmail.com>
License: GPL v2

Purpose: This code implement the remote control part of the wireless door opener. It receives 
signal from the open/close buttons, then generates a message accordingt to the button pressed.
After that, the message is encrypted using AES-128 and sent to the base station via 
nRF24L01+ transceiver. No handshaking mechanism is implemented since the tranceiver
already supports auto acknowledgement.
*/
#include "TaskScheduler.h"

#include "pes_aes128.h"

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//Buttons - open, close, lock, unlock 
#define BUTTON_OPEN 4
#define BUTTON_CLOSE 5
#define BUTTON_LOCK 6
#define BUTTON_UNLOCK 7

#define BUTTON_STATE_OPEN true
#define BUTTON_STATE_CLOSE false
#define BUTTON_STATE_LOCK true
#define BUTTON_STATE_UNLOCK false

//Set up buttons
bool lockUnlockState = BUTTON_STATE_LOCK;
bool openCloseState = BUTTON_STATE_CLOSE;
bool openButtonState;
bool lastOpenButtonState = HIGH;
bool closeButtonState;
bool lastCloseButtonState = HIGH;

bool messageFlag = false;

RF24 radio(9,10);  // Set up nRF24L01 radio on SPI bus plus pins 9 & 10
const uint64_t pipe = 0xF0F0F0F0E1LL;  // Radio pipe addresses for the 2 nodes to communicate.

uint8_t key[] = {  0x54, 0x68, 0x69, 0x73, 0x69, 0x73, 0x61, 0x73,    //Secret key for remote-control
                            0x65, 0x63, 0x72, 0x65, 0x74, 0x6b, 0x65, 0x79};  //and base station

//Tasks used by the scheduler
void openButtonUpdate(void);
void closeButtonUpdate(void);
void lockButtonUpdate(void);
void unlockButtonUpdate(void);
void sendMessage(void);


void setup() {
  //Set up nRF24L01+ radio transceiver
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipe);
  
  //Set up button pins 
  pinMode(BUTTON_LOCK, INPUT);
  pinMode(BUTTON_UNLOCK, INPUT); 
  pinMode(BUTTON_OPEN, INPUT);
  pinMode(BUTTON_CLOSE, INPUT);
  
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
      openCloseState = BUTTON_STATE_OPEN;
      messageFlag = true;   
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
      openCloseState = BUTTON_STATE_CLOSE;
      messageFlag = true;
    }
  }
  lastCloseButtonState = reading;  
}


//lock button task. Set lockUnlockState variable to state lock when "lock" button is pressed
void lockButtonUpdate(void) {
  if(digitalRead(BUTTON_LOCK) == 0)
    lockUnlockState = BUTTON_STATE_LOCK;
}


//unlock button task. Set lockUnlockState variable to state lock when "unlock" button is pressed
void unlockButtonUpdate(void) {
  if(digitalRead(BUTTON_UNLOCK) == 0)
    lockUnlockState = BUTTON_STATE_UNLOCK;
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
        radio.write(&message, sizeof(message));
      } else {
        uint8_t message[] = "000PES2015_close";
        aes_encrypt(message,key);
        radio.write(message, sizeof(message));
      }
    }  
  }
  messageFlag = false;
}

