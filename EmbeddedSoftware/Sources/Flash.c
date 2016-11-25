/*! @file
 *
 *  @brief Routines for erasing and writing to the Flash.
 *
 *  This contains the functions needed for accessing the internal Flash.
 *
 *  @author ANGZE LI LIN TAO
 *  @date 2016-08-21
 */
/*!
**  @addtogroup Flash_module Flash module documentation
**  @{
*/
/*MODULE Flash*/
// new types
#include "types.h"
#include "Flash.h"
#include "MK70F12.h"

/*! @brief Read out the phrase from flash to RAM
 *
 *  @param buffer[] To store the data get from flash
 *  @return void
 */
static void ReadFlash(uint8_t buffer[])
{
  uint8_t i;  /*!< index value */
  uint8_t *temp;
  temp = (uint8_t *)FLASH_DATA_START;
  for (i = 0; i < 8; i++)
  {
    buffer[i] = *temp;
    temp++;
  }
}

/*! @brief write a phrase into the flash
 *
 *  @param buffer[] To store the data get from flash
 *  @return BOOL - TRUE if the Flash Writing process successfully executed
 */
static BOOL WritePhrase(const uint8_t buffer[])
{
  BOOL eraseSuccess = Flash_Erase();
  if (eraseSuccess)
  {
    for (;;)
    {
      if (FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK)
      {
        //check the old error
        if (FTFE_FSTAT & FTFE_FSTAT_ACCERR_MASK & FTFE_FSTAT_FPVIOL_MASK)
          //clear the old error
          FTFE_FSTAT = 0x30;

        //set up Erase Flash Sector command
        FTFE_FCCOB0 = 0x07;
        FTFE_FCCOB1 = 0x08;
        FTFE_FCCOB2 = 0x00;
        FTFE_FCCOB3 = 0x00;
        FTFE_FCCOB7 = buffer[0];
        FTFE_FCCOB6 = buffer[1];
        FTFE_FCCOB5 = buffer[2];
        FTFE_FCCOB4=  buffer[3];
        FTFE_FCCOBB = buffer[4];
        FTFE_FCCOBA = buffer[5];
        FTFE_FCCOB9 = buffer[6];
        FTFE_FCCOB8 = buffer[7];

        //launch the command
        FTFE_FSTAT |= FTFE_FSTAT_CCIF_MASK;

        //wait until this command completed
        while(! FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK){}

        return bTRUE;
      }
    }
  }
}

BOOL Flash_Init(void)
{
  //enable clock gate
  SIM_SCGC3 = SIM_SCGC3_NFC_MASK;
  //erase the whole sector before first use
  return bTRUE;
}


BOOL Flash_AllocateVar(volatile void** variable, const uint8_t size)
{

  uint32_t index = FLASH_DATA_START;  /*!< index value to indicate the current available address */
  static uint8_t allocationMap[8];    /*!< an array to indicate the allocation state of each memory slot */
  uint8_t i, consistentSpace = 0;     /*!< indicating the number of consistent memory slots*/
  int temp;
  uint8_t j;
  BOOL tempBool, isFull = bFALSE;
  //check if the flash is full
  for (temp = 0; temp < 7; temp++)
  {

    tempBool = (allocationMap[temp] && allocationMap[temp + 1]);
    isFull &= tempBool;
  }
  if (isFull) return bFALSE;

  for (i = 0; i < 8; i++)
  {
    if (allocationMap[i] == 0)
    {
      consistentSpace++;

      if (consistentSpace >= size)
      {
        if (size == 1)
	{
	  *variable = (void *)index;
          //index += size;
	  allocationMap[i] = 1;
	  return bTRUE;
	}

        uint32_t god = index - (uint32_t)size + 1;
        if ((size == 2) && ((god % 2) == 0))
        {
          *variable = (void *)god;
	  for (j = (uint8_t)god; j < (uint8_t)god + size; j++)
	  {
	    allocationMap[j] = 1;
	  }
	  //index += size;
	  return bTRUE;
        }
        if ((size == 4) && (god % 4) == 0)
        {
          *variable = (void *)god;
	  for (j = (uint8_t)god; j < ((uint8_t)god + size); j++)
	  {
	    allocationMap[j] = 1;
	  }
	  //index += size;
	  return bTRUE;
        }

      index++;

     }
     else
       index++;
    }
    else
      index++;
  }

}


BOOL Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{

  uint32union_t word;
  uint16union_t hword0;
  uint16union_t hword1;
  uint32_t temp;
  uint8_t buffer[8];
  temp = (uint32_t)address - FLASH_DATA_START;
  ReadFlash(buffer);

  word.l = data;
  hword0.l = word.s.Lo;
  hword1.l = word.s.Hi;
  buffer[temp] = hword0.s.Lo;
  buffer[temp+1] = hword0.s.Hi;
  buffer[temp+2] = hword1.s.Lo;
  buffer[temp+3] = hword1.s.Hi;

  return WritePhrase(buffer);
}


BOOL Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{
  uint16union_t hword;
  uint32_t temp;

  hword.l = data;
  temp = (uint32_t)address - FLASH_DATA_START;
  uint8_t buffer[8];
  ReadFlash(buffer);

  buffer[temp] = hword.s.Lo;
  buffer[temp+1] = hword.s.Hi;
  return WritePhrase(buffer);
}


BOOL Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{
  uint16union_t hword;
  uint32_t temp;

  temp = (uint32_t)address - FLASH_DATA_START;
  uint8_t buffer[8];
  ReadFlash(buffer);
  buffer[temp] = data;

  return WritePhrase(buffer);
}


BOOL Flash_Erase(void)
{
  for (;;)
  {
    if (FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK)
    {
      //check the old error
      if (FTFE_FSTAT & FTFE_FSTAT_ACCERR_MASK & FTFE_FSTAT_FPVIOL_MASK)
      //clear the old error
        FTFE_FSTAT = 0x30;

      //set up Erase Flash Sector command
      FTFE_FCCOB0 = 0x09;
      FTFE_FCCOB1 = 0x08;
      FTFE_FCCOB2 = 0x00;
      FTFE_FCCOB3 = 0x00;

      //launch the command
      FTFE_FSTAT |= FTFE_FSTAT_CCIF_MASK;

      //wait until this command completed
      while(! FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK){}

      return bTRUE;
    }
  }
}
/* END Flash */
/*!
** @}
*/






