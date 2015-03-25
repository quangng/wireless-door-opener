/*
University of Turku
Master's degree in Embedded Computing
Cyber Physical Systems course
Project: Wireless door opener
Author: Vu Nguyen <quangngmetro@gmail.com>
License: GNU GPL

Purpose: This code implement the remote control part of the wireless door opener. It receives 
signal from the open/close buttons, then generates a message accordingt to the button pressed.
After that, the message is encrypted using AES-128 and sent to the base station via 
nRF24L01+ transceiver. No handshaking mechanism is implemented since the tranceiver
already supports auto acknowledgement.
*/
#include "TaskScheduler.h"

//#include "TI_aes.h"
#include "AESLib.h"

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
//#include <RF24_config.h>
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

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
// The various roles supported by this sketch
typedef enum { role_remote_control = 1, role_base_station } role_e;
// The debug-friendly names of those roles
const char *role_friendly_name[] = { "invalid", "Remote Control", "Base Station"};
// The role of the current running sketch
role_e role = role_remote_control;


uint8_t key[] = { 0x54, 0x68, 0x69, 0x73, 0x69, 0x73, 0x61, 0x73,
                          0x65, 0x63, 0x72, 0x65, 0x74, 0x6b, 0x65, 0x79};


//Tasks used by the scheduler
void openButtonUpdate(void);
void closeButtonUpdate(void);
void lockButtonUpdate(void);
void unlockButtonUpdate(void);
void sendMessage(void);


void setup() {
  Serial.begin(9600);
  printf_begin();
  printf("\n\rRemote control starting...\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
   
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[0]);
  radio.printDetails();
  
  pinMode(3, OUTPUT); 
  pinMode(BUTTON_LOCK, INPUT);
  pinMode(BUTTON_UNLOCK, INPUT); 
  pinMode(BUTTON_OPEN, INPUT);
  pinMode(BUTTON_CLOSE, INPUT);
  
  Sch.init();  //Initialize task scheduler
  
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

void lockButtonUpdate(void) {
  if(digitalRead(BUTTON_LOCK) == 0)
    lockUnlockState = BUTTON_STATE_LOCK;
}

void unlockButtonUpdate(void) {
  if(digitalRead(BUTTON_UNLOCK) == 0)
    lockUnlockState = BUTTON_STATE_UNLOCK;
}

void sendMessage(void) {
  if(messageFlag) {
    if (lockUnlockState == BUTTON_STATE_UNLOCK) {
      if(openCloseState == BUTTON_STATE_OPEN) {
        char message[] = "0000PES2015_open";
        Serial.print("Message to be sent: ");
        Serial.println(message);
        Serial.print("message size: ");
        Serial.println(sizeof(message));
        
        aes128_enc_single(key, message);
        Serial.print("encrypted: ");
        Serial.println(message);
        printf("\r\nNow sending open message...\r\n");
        bool ok = radio.write(&message, sizeof(message));
        
        if (ok)
          printf("ok\r\n");
        else
          printf("failed\r\n");
          
        digitalWrite(3, HIGH);
      } else {
        char message[] = "000PES2015_close";
        Serial.print("Message to be sent: ");
        Serial.println(message);
        Serial.print("message size: ");
        Serial.println(sizeof(message));
        
        aes128_enc_single(key, message);
        printf("encrypted: ");
        printf(message);          
        printf("Now sending close message...\r\n");
        bool ok = radio.write(message, sizeof(message));
        
        if (ok)
          printf("ok\r\n");
        else
          printf("failed\r\n");          
          
        digitalWrite(3, LOW);
      }
    }  
  }
  messageFlag = false;
}

