/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "hal.h"
#include "er9x.h"
//#include "lcd.h"
/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/

////------temp---------
////uint16_t s_anaFilt[8];
//uint8_t pinb,pind,pine,ping;
//uint16_t tim;
//Si2c_buffer i2c_buffer;
//uint16_t i = 0;
////-------------------
#define PINB PINB()
#define PIND PIND()
#define PINE PINE()
#define PING PING()

///*---------------For test A7105 radio------------------------------*/
//#define A7105_01_MODE_CONTROL  0x01

//void A7105_WriteReg(uint8_t address, uint8_t data) {
//  uint8_t out[2];
//  out[0] = address;
//  out[1] = data;
//  A7105_CSN_OFF;
//  SPI_RADIO_SendBlock(out, 2);
//  A7105_CSN_ON;
//}

//uint8_t A7105_ReadReg(uint8_t address) {
//  uint8_t out;
//  uint8_t in;
//  A7105_CSN_OFF;
//  out = address | 0x40;
//  SPI_RADIO_SendBlock(&out, 1);
//  SPI_RADIO_ReceiveBlock(&in, 1);
//  A7105_CSN_ON;
//  return in;
//}
///*------------------for test EEPROM-----------------------------------*/

//void eeprom_read_block(void *i_pointer_ram, const void *i_pointer_eeprom,
//		size_t size) {
//	//while (tmrEEPROM < EE_TIME_WR) {
//	//}   // make sure EEPROM is ready
//	uint16_t pointer_eeprom = (unsigned) (uint16_t*) i_pointer_eeprom;
//	uint8_t buff[2];
//	buff[0] = (uint8_t) (pointer_eeprom >> 8);
//	buff[1] = (uint8_t) (pointer_eeprom & 0xff);
//	i2c_buffer.length = 2;
//	i2c_buffer.buf = buff;
//	i2c_master(I2C_TX, 0x50);
//	i2c_buffer.length = size;
//	i2c_buffer.buf = (uint8_t*) i_pointer_ram;
//	i2c_master(I2C_RX, 0x50);
//}


int main(void) {
  HW_Init();
  mainER();

  // g_eeGeneral.contrast = 25;
  // lcd_init();
  //BACKLIGHT_ON;
//  RF0_ClrVal();
//  RF1_ClrVal();
//  TX_RX_PutVal(0);
//  while (1) {
//    getADC_osmp();
//    pinb = PINB;
//    pind = PIND;
//    pine = PINE;
//    ping = PING;
//    lcd_clear();
////    // Analog
//    lcd_putsAtt(0, 0, "AIL-", 0);
//    lcd_putsAtt(0, FH * 1, "THR-", 0);
//    lcd_putsAtt(0, FH * 2, "ELE-", 0);
//    lcd_putsAtt(0, FH * 3, "RUD-", 0);
//    lcd_putsAtt(0, FH * 4, "VrA-", 0);
//    lcd_putsAtt(0, FH * 5, "VrB-", 0);
//    lcd_putsAtt(0, FH * 6, "SwC-", 0);
//    lcd_putsAtt(0, FH * 7, "BAT-", 0);
//    for (uint8_t i = 0; i < 8; i++)
//    lcd_outdez(FW * 7, FH * i, s_anaFilt[i]);

////    //Buttons
//    lcd_putsAtt(FW * 8, 0, "OCDULR", 0);
//    lcd_outdez(FW * 9-1, FH * 1, ((pinb & (1 << INP_B_KEY_MEN)) ? 1 : 0));
//    lcd_outdez(FW * 10-1, FH * 1, ((pinb & (1 << INP_B_KEY_EXT)) ? 1 : 0));
//    lcd_outdez(FW * 11-1, FH * 1, ((pinb & (1 << INP_B_KEY_DWN)) ? 1 : 0));
//    lcd_outdez(FW * 12-1, FH * 1, ((pinb & (1 << INP_B_KEY_UP)) ? 1 : 0));
//    lcd_outdez(FW * 13-1, FH * 1, ((pinb & (1 << INP_B_KEY_LFT)) ? 1 : 0));
//    lcd_outdez(FW * 14-1, FH * 1, ((pinb & (1 << INP_B_KEY_RGT)) ? 1 : 0));
////   //Trimmers
//    lcd_putsAtt(FW * 15, 0, "RH_U", 0);
//    lcd_putsAtt(FW * 15, FH*1, "RH_D", 0);
//    lcd_putsAtt(FW * 15, FH*2, "LV_U", 0);
//    lcd_putsAtt(FW * 15, FH*3, "LV_D", 0);
//    lcd_putsAtt(FW * 15, FH*4, "RV_U", 0);
//    lcd_putsAtt(FW * 15, FH*5, "RV_D", 0);
//    lcd_putsAtt(FW * 15, FH*6, "LH_D", 0);
//    lcd_putsAtt(FW * 15, FH*7, "LH_U", 0);
 
//    lcd_outdez(FW * 20, FH * 0, ((pind & (1 << INP_D_TRM_RH_UP )) ? 1 : 0));
//    lcd_outdez(FW * 20, FH * 1, ((pind & (1 << INP_D_TRM_RH_DWN)) ? 1 : 0));
//    lcd_outdez(FW * 20, FH * 2, ((pind & (1 << INP_D_TRM_LV_UP )) ? 1 : 0));
//    lcd_outdez(FW * 20, FH * 3, ((pind & (1 << INP_D_TRM_LV_DWN)) ? 1 : 0));
//    lcd_outdez(FW * 20, FH * 4, ((pind & (1 << INP_D_TRM_RV_UP )) ? 1 : 0));
//    lcd_outdez(FW * 20, FH * 5, ((pind & (1 << INP_D_TRM_RV_DWN )) ? 1 : 0));
//    lcd_outdez(FW * 20, FH * 6, ((pind & (1 << INP_D_TRM_LH_DWN )) ? 1 : 0));
//    lcd_outdez(FW * 20, FH * 7, ((pind & (1 << INP_D_TRM_LH_UP )) ? 1 : 0));
////    // switches
//    lcd_putsAtt(FW * 8, FH * 3, "SwA-", 0);
//    lcd_putsAtt(FW * 8, FH * 4, "SwB-", 0);
//    lcd_putsAtt(FW * 8, FH * 5, "SwD-", 0);
 
//    lcd_outdez(FW * 13, FH * 3, ((pine & (1 << INP_E_ThrCt)) ? 1 : 0));
//    lcd_outdez(FW * 13, FH * 4, ((ping & (1 << INP_G_RuddDR)) ? 1 : 0));
//    lcd_outdez(FW * 13, FH * 5, ((pine & (1 << INP_E_ElevDR)) ? 1 : 0));

////    A7105_WriteReg(0x00, 0x00); //reset
////    pinb = 0;
////    while (A7105_ReadReg(0x10) != 0x9E) {
////    };

////    eeprom_read_block(&pinb, (const void*)(size_t)i, 1);
////    i++;
////    lcd_outhex4((FW * 10) - 2, FH * 6, pinb);

////    lcd_outdez(FW * 13, FH * 7, tim);
    
//    refreshDiplay();
//  }
}

/*************************** End of file ****************************/