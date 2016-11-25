/*! @file
 *
 *  @brief I/O routines for K70 SPI interface.
 *
 *  This contains the functions for operating the SPI (serial peripheral interface) module.
 *
 *  @author PMcL
 *  @date 2016-08-22
 */

#ifndef SPI_H
#define SPI_H

// new types
#include "types.h"

typedef struct
{
  bool isMaster;
  bool continuousClock;
  bool inactiveHighClock;
  bool changedOnLeadingClockEdge;
  bool LSBFirst;
  uint32_t baudRate;
} TSPIModule;

// ----------------------------------------
// SPI_Init
// 
// Sets up the Serial Peripheral Interface
// Input:
//   aSPISetup is a structure containing the parameters to 
//     be used in setting up the SPI:
//       isMaster is a Boolean value indicating whether the SPI is master or slave
//       activeLowClocks is a Boolean value indicating whether the clock is active
//         low or active high
//       evenEdgeClockPhase is a Boolean value indicating whether the data is clocked
//         on even or odd edges
//       LSBFirst is a Boolean value indicating whether the data is transferred LSB
//         first or MSB first
//       baudRate is the baud rate in bits/sec of the SPI clock
//     moduleClock is the bus clock rate in Hz
// Output:
//   none
// Conditions:
//   none
// ----------------------------------------

BOOL SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock);
 
void SPI_SelectSlaveDevice(const uint8_t slaveAddress);

// ----------------------------------------
// SPI_ExchangeChar
//
// Transmits a byte and retrieves a received byte from the SPI
// Input:
//   dataTx is a byte to transmit
//   dataRx is a pointer to a byte to receive
// Output:
//   none
// Conditions:
//   Assumes SPI has been set up

void SPI_ExchangeChar(const uint16_t dataTx, uint16_t* const dataRx);

#endif
