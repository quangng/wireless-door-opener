/*
* ----------------------------------------------------------------------------
* “THE COFFEEWARE LICENSE” (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
* This library is based on this library: 
*   https://github.com/aaronds/arduino-nrf24l01
* Which is based on this library: 
*   http://www.tinkerer.eu/AVRLib/nRF24L01
* -----------------------------------------------------------------------------
*/
#ifndef _NRF24_H
#define _NRF24_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "nRF24L01.h"
#include <SPI.h>

#define LOW 0
#define HIGH 1

#define nrf24_ADDR_LEN 5
#define nrf24_CONFIG ((1<<EN_CRC)|(0<<CRCO))

#define NRF24_TRANSMISSON_OK 0
#define NRF24_MESSAGE_LOST   1

/* adjustment functions */
void nrf24_init(uint8_t ce_pin, uint8_t cs_pin);
void nrf24_rx_address(uint8_t* adr);
void nrf24_tx_address(uint8_t* adr);
void nrf24_config(uint8_t channel, uint8_t pay_length);

/* state check functions */
uint8_t nrf24_dataReady();
uint8_t nrf24_isSending();
uint8_t nrf24_getStatus();
uint8_t nrf24_rxFifoEmpty();

/* core TX / RX functions */
void nrf24_send(uint8_t* value);
void nrf24_getData(uint8_t* data);

/* use in dynamic length mode */
uint8_t nrf24_payloadLength();

/* post transmission analysis */
uint8_t nrf24_lastMessageStatus();
uint8_t nrf24_retransmissionCount();

/* Returns the payload length */
uint8_t nrf24_payload_length();

/* power management */
void nrf24_powerUpRx();
void nrf24_powerUpTx();
void nrf24_powerDown();

/* low level interface ... */
uint8_t spi_transfer(uint8_t tx);
void nrf24_transmitSync(uint8_t* dataout,uint8_t len);
void nrf24_transferSync(uint8_t* dataout,uint8_t* datain,uint8_t len);
void nrf24_configRegister(uint8_t reg, uint8_t value);
void nrf24_readRegister(uint8_t reg, uint8_t* value, uint8_t len);
void nrf24_writeRegister(uint8_t reg, uint8_t* value, uint8_t len);
//Low-level platform dependent functions, implemented for Arduino Uno
//If ported to other platform, the following two functions need to be modified
//to suit the target platform
void nrf24_ce_digitalWrite(uint8_t state);
void nrf24_csn_digitalWrite(uint8_t state);

#endif
