#include <TI_aes.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <nrf24.h>

#include <Servo.h>

#define SERVO_PIN 4
#define LED_PIN 7

uint8_t message[17];
uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};

uint8_t key[] = { 0x54, 0x68, 0x69, 0x73, 0x69, 0x73, 0x61, 0x73,
                            0x65, 0x63, 0x72, 0x65, 0x74, 0x6b, 0x65, 0x79};
     
Servo myservo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Testing base-station...");
  
  nrf24_init(9,10);       //Set ce pin and csn pin
  nrf24_config(2,17);  //Channel #2, payload length: 17
  nrf24_rx_address(rx_address);
  
  pinMode(LED_PIN, OUTPUT);
  
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(nrf24_dataReady()){      
    nrf24_getData(message);
    aes_decrypt(message, key);
    char tmp[17];
    Serial.print("Received: ");
    for (int i = 0; i < sizeof(message); i++) {
      char c = message[i];
      tmp[i] = c;
      Serial.print(c);
    }
    Serial.println();
    
    if (strcmp(tmp, "0000PES2015_open") == 0) {
      digitalWrite(LED_PIN, HIGH);
      //myservo.write(180);
    } else if (strcmp(tmp, "000PES2015_close") == 0) {
      digitalWrite(LED_PIN, LOW);
      //myservo.write(0);
    }
    
  }
}



