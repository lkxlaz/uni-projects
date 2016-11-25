/*! @file
 *
 *  @brief I/O routines for K70 SPI interface.
 *
 *  This contains the functions for operating the SPI (serial peripheral interface) module.
 *
 *  @author Lin Tao, ANGZE LI
 *
 *  @date 2016-09-24
 */
/*!
**  @addtogroup SPI_module SPI module documentation
**  @{
*/

#include "MK70F12.h"
#include "SPI.h"

#define MAX_BR_SCALER 32768
#define MAX_BR_PRESCALER 7
#define GPIO8 0x00000020
#define GPIO7 0x08000000

static uint32union_t SpiTransmitData;                 /*!< The word that holds the SPI transfer command and data */

/*! @brief calculate the absolute value of a number
 *
 *  @param number the number to be calculated
 *  @return the absolute value
 */
static uint32_t abs(int32_t number)
{
  if (number < 0)
    return (number * (-1));
  else
    return number;
}

/*! @brief Do exhaustive search to the right baud rate
 *
 *  @param moduleClock The module clock in Hz.
 *  @param baudrate The baud rate in bits/sec of the SPI clock.
 */
static void SPI_SetBaudRateDivisors(const uint32_t moduleClock, uint32_t baudRate)
{
  uint32_t scalerCollection[16] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};
  uint32_t prescalerColection[4] = {2, 3, 5, 7};                                                                                 
  // Final value for scaler and prescaler
  uint8_t scaler, prescaler;                                                 
  // Difference between desiredBaudRate and scaledValue            
  uint32_t closestDifference = 0xFFFFFFFF;                                
  uint32_t temp; 
  uint32_t test;

  uint8_t i, j;
  // Iterate through prescaler values for each scaler value
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 16; j++)
    {
      temp = moduleClock / (scalerCollection[j] * prescalerColection[i]);
      if (abs(temp - baudRate) < closestDifference)
      {
	// update  closestDifference
	closestDifference = abs(temp - baudRate);
	// assign the value of prescaler and scaler
	scaler = j;
	prescaler = i;
      }
    }

  }

  SPI2_CTAR0 |= SPI_CTAR_PBR(prescaler);
  SPI2_CTAR0 |= SPI_CTAR_BR(scaler);
}

BOOL SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock)
{
  // Enables SPI clock
  SIM_SCGC3 |= SIM_SCGC3_DSPI2_MASK;       
  
  // Enable clock gate to PORTD and PORTE
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;                         
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;                         

  // set PORTD 11-15 as SPI2
  PORTD_PCR11 = PORT_PCR_MUX(2);                             
  PORTD_PCR12 = PORT_PCR_MUX(2);                            
  PORTD_PCR13 = PORT_PCR_MUX(2);                            
  PORTD_PCR14 = PORT_PCR_MUX(2);                             
  PORTD_PCR15 = PORT_PCR_MUX(2);                        

  // Configure GPIO7 and GPIO8
  PORTE_PCR27 = PORT_PCR_MUX(1);                           
  PORTE_PCR5  = PORT_PCR_MUX(1);           
  
  // Initially clear output  
  GPIOE_PCOR  = (GPIO8 | GPIO7);  
  // set pin direction as output  
  GPIOE_PDDR |= (GPIO8 | GPIO7);                             

  SPI_SetBaudRateDivisors(moduleClock, aSPIModule->baudRate);

  // Enable module clocks
  SPI2_MCR &= ~SPI_MCR_MDIS_MASK;          
  // Halt serial transfers in debug mode  
  SPI2_MCR |= SPI_MCR_FRZ_MASK;      
  // Set Chip select to inactive high  
  SPI2_MCR |= SPI_MCR_PCSIS(1);    
  // Disable transmit FIFO and receive FIFO  
  SPI2_MCR |= SPI_MCR_DIS_TXF_MASK;                          
  SPI2_MCR |= SPI_MCR_DIS_RXF_MASK;                          

  // Set 16 bit frame size
  SPI2_CTAR0 |= SPI_CTAR_FMSZ(15);                           

  // Sets the SPI to Master Mode
  if (aSPIModule->isMaster)
    SPI2_MCR |= SPI_MCR_MSTR_MASK;                           

  // Sets the SPI to Master Mode
  if (aSPIModule->continuousClock)
    SPI2_MCR |= SPI_MCR_CONT_SCKE_MASK;                      

  // Enables the SPI clock
  if (aSPIModule->inactiveHighClock)
    SPI2_CTAR0 |= SPI_CTAR_CPOL_MASK;                        

  if (aSPIModule->changedOnLeadingClockEdge)                 
    SPI2_CTAR0 |= SPI_CTAR_CPHA_MASK;

  // Sets the LSB first
  if (aSPIModule->LSBFirst)
    SPI2_CTAR0 |= SPI_CTAR_LSBFE_MASK;                       

  // Delay to allow capacitors to charge before transfer
  SPI2_CTAR0 |= SPI_CTAR_CSSCK(3); 
  SPI2_CTAR0 |= SPI_CTAR_PCSSCK(3);

  SpiTransmitData.s.Hi = 1;

  return bTRUE;

}



void SPI_SelectSlaveDevice(const uint8_t slaveAddress)
{
  
  switch (slaveAddress)
  {
    // LTC2704
    case 4:
      GPIOE_PCOR  = (GPIO8 | GPIO7);
      break;

    // LTC2600
    case 5:
      GPIOE_PCOR  = GPIO8;
      GPIOE_PSOR  = GPIO7;
      break;

    // LTC2498
    case 6:
      GPIOE_PSOR  = GPIO8;
      GPIOE_PCOR  = GPIO7;
      break;
     // LTC1859
    case 7:
      GPIOE_PSOR  = (GPIO8 | GPIO7);
      break;
  }
}

void SPI_ExchangeChar(const uint16_t dataTx, uint16_t* const dataRx)
{
  SpiTransmitData.s.Lo = dataTx;
  // Wait until the FIFO is not full
  while (!(SPI2_SR & SPI_SR_TFFF_MASK));                    
  // Push data and commands byte
  SPI2_PUSHR = SpiTransmitData.l;                           
  // clear the flag by writing a 1
  SPI2_SR |= SPI_SR_TFFF_MASK;                              
  // Start transfer
  SPI2_MCR &= ~SPI_MCR_HALT_MASK;                           

  // Wait until SPI transaction is complete
  while (!(SPI2_SR & SPI_SR_RFDF_MASK));  
  // Stop transfer  
  SPI2_MCR |= SPI_MCR_HALT_MASK;                            
  *dataRx = (uint16_t)SPI2_POPR;                            
  // clear the flag by writing a 1
  SPI2_SR |= SPI_SR_RFDF_MASK;                              

}

/*!
** @}
*/
