/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios. 
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two 
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting 
 * with the serial monitor and sending a 'T'.  The ping node sends the current 
 * time to the pong node, which responds by sending the value back.  The ping 
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#include "AESLib.h"

#include <Servo.h>

#define SERVO_PIN 4

uint8_t key[] = { 0x54, 0x68, 0x69, 0x73, 0x69, 0x73, 0x61, 0x73,
                          0x65, 0x63, 0x72, 0x65, 0x74, 0x6b, 0x65, 0x79};


//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
//

// The various roles supported by this sketch
typedef enum { role_remote_control = 1, role_base_station } role_e;

// The debug-friendly names of those roles
const char *role_friendly_name[] = { "invalid", "Remote Control", "Base Station"};

// The role of the current running sketch
role_e role = role_base_station;



Servo myservo;



void setup(void)
{
  //
  // Print preamble
  //
  pinMode(7, OUTPUT);
  
  
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  
  
  Serial.begin(9600);
  printf_begin();
  printf("\n\rBase station starting...\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  //if ( role == role_ping_out )
  {
    //radio.openWritingPipe(pipes[0]);
    //radio.openReadingPipe(1,pipes[1]);
  }
  //else
  {
    //radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}

void loop(void)
{

  //
  // Base station - receives encrypted messages, decrypt them, and control servo
  //
  // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      //unsigned long got_time;
      char message[17];
      printf("Size of receive buffer: %d\n", sizeof(message));
      
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &message, sizeof(message) );

        // Spew it
        printf("Got message %s\r\n",message);
 
        
        //Decrypting the messsage
        aes128_dec_single(key, message);
        message[16] = '\0';
        printf("decrypted: ");
        printf(message);
       
        
        if (strcmp(message, "0000PES2015_open") == 0) {
            digitalWrite(7, HIGH);
            myservo.write(180);
            delay(15);
        }
        
        if (strcmp(message, "000PES2015_close") == 0) {
          digitalWrite(7, LOW);
          myservo.write(0);
          delay(15);
        }

	// Delay just a little bit to let the other unit
	// make the transition to receiver
	delay(20);
      }

      // First, stop listening so we can talk
      //radio.stopListening();

      // Send the final one back.
      //radio.write( &got_time, sizeof(unsigned long) );
      //printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
