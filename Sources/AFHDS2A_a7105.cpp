 /*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Multiprotocol is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Multiprotocol.  If not, see <http://www.gnu.org/licenses/>.
 */
// Last sync with hexfet new_protocols/flysky_a7105.c dated 2015-09-28
#include "er9x.h"
#include "iface_a7105.h"

#ifdef AFHDS2A_A7105_INO

#define AFHDS2A_TXPACKET_SIZE	37
#define AFHDS2A_RXPACKET_SIZE	37
#define AFHDS2A_HUB_TELEMETRY
//#define AFHDS2A_NUMFREQ			16


//#define GIO1 GIO1_GetVal(GIO1_DeviceData)

 static void AFHDS2A_calc_channels() {
     uint8_t idx = 0;
     uint32_t rnd = ID.MProtocol_id;
     uint8_t i;
     while (idx < AFHDS2A_NUMFREQ)
     {
         uint8_t band_no = ((((idx << 1) | ((idx >> 1) & 0b01)) + ID.rx_tx_addr[3]) & 0b11);
         rnd = rnd * 0x0019660D + 0x3C6EF35F; // Randomization

         uint8_t next_ch = band_no * 41 + 1 + ((rnd >> idx) % 41); // Channel range: 1..164

         for (i = 0; i < idx; i++)
         {
             // Keep the distance 5 between the channels
             uint8_t distance;
             if (next_ch > hopping_frequency[i])
                 distance = next_ch - hopping_frequency[i];
             else
                 distance = hopping_frequency[i] - next_ch;

             if (distance < 5)
                 break;
         }

         if (i != idx)
             continue;

         hopping_frequency[idx++] = next_ch;
     }
 }

// telemetry sensors ID
enum{
	AFHDS2A_SENSOR_RX_VOLTAGE   = 0x00,
	AFHDS2A_SENSOR_RX_TEMP      = 0x01,
	AFHDS2A_SENSOR_RX_RPM       = 0x02,
	AFHDS2A_SENSOR_A3_VOLTAGE   = 0x03,
	AFHDS2A_SENSOR_RX_ERR_RATE  = 0xfe,
	AFHDS2A_SENSOR_RX_RSSI      = 0xfc,
	AFHDS2A_SENSOR_RX_NOISE     = 0xfb,
	AFHDS2A_SENSOR_RX_SNR       = 0xfa,
};

void AFHDS2A_update_telemetry()
{
    if (packet[0] == 0xAA && packet[9] == 0xFD)
        return; // ignore packets which contain the RX configuration: FD FF 32 00 01 00 FF FF FF 05 DC 05 DE FA FF FF FF FF FF FF FF FF FF FF FF FF FF FF
                // Read TX RSSI

    /*
	int16_t temp=256-(A7105_ReadReg(A7105_1D_RSSI_THOLD)*8)/5;		// value from A7105 is between 8 for maximum signal strength to 160 or less
	if(temp<0) temp=0;
	else if(temp>255) temp=255;
	TX_RSSI=temp;
*/

    // AA | TXID | rx_id | sensor id | sensor # | value 16 bit big endian | sensor id ......
    // AC | TXID | rx_id | sensor id | sensor # | length | bytes | sensor id ......
    if (packet[0] == 0xAA)
    { // 0xAA Normal telemetry, 0xAC Extended telemetry not decoded here
        for (uint8_t sensor = 0; sensor < 7; sensor++)
        {
            // Send FrSkyD telemetry to TX
            uint8_t index = 9 + (4 * sensor);
            switch (packet[index])
            {
            case AFHDS2A_SENSOR_RX_VOLTAGE:
                telem_AFHDS2A[RX_IntV] = packet[index + 3] << 8 | packet[index + 2];
                telem_status |= (1 << RX_IntV);
                break;
            case AFHDS2A_SENSOR_A3_VOLTAGE:
                telem_AFHDS2A[RX_ExtV] = (packet[index + 3] << 8) | (packet[index + 2]);
                telem_status |= (1 << RX_ExtV);
                break;
            case AFHDS2A_SENSOR_RX_TEMP:
                telem_AFHDS2A[RX_Temp] = (packet[index + 3] << 8) | (packet[index + 2]);
                telem_status |= (1 << RX_Temp);
                break;
            case AFHDS2A_SENSOR_RX_RPM:
                telem_AFHDS2A[RX_RPM] = (packet[index + 3] << 8) | (packet[index + 2]);
                telem_status |= (1 << RX_RPM);
                break;
            case AFHDS2A_SENSOR_RX_ERR_RATE:
                telem_AFHDS2A[RX_Err] = packet[index + 2];
                telem_status |= (1 << RX_Err);
                break;
            case AFHDS2A_SENSOR_RX_RSSI:
                //RX_RSSI = -packet[index+2];
                break;
            case 0xff: // end of data
                return;
                /*default:
		// unknown sensor ID
		break;*/
            }
        }
    }
}

static void AFHDS2A_build_bind_packet(void)
{
	uint8_t ch;
    uint8_t phase = RadioState & 0x0F;
	memcpy( &packet[1], ID.rx_tx_addr, 4);
	memset( &packet[5], 0xff, 4);
	packet[10]= 0x00;
	for(ch=0; ch<AFHDS2A_NUMFREQ; ch++)
		packet[11+ch] = hopping_frequency[ch];
	memset( &packet[27], 0xff, 10);
	packet[37] = 0x00;
	switch(phase)
	{
		case AFHDS2A_BIND1:
			packet[0] = 0xbb;
			packet[9] = 0x01;
			break;
		case AFHDS2A_BIND2:
		case AFHDS2A_BIND3:
		case AFHDS2A_BIND4:
			packet[0] = 0xbc;
			if(phase == AFHDS2A_BIND4)
			{
				memcpy( &packet[5], &g_model.rxID, 4);
				memset( &packet[11], 0xff, 16);
			}
			packet[9] = phase-1;
			if(packet[9] > 0x02)
				packet[9] = 0x02;
			packet[27]= 0x01;
			packet[28]= 0x80;
			break;
	}
}

void AFHDS2A_build_packet(uint8_t type)
{
//	uint16_t val;
	memcpy( &packet[1], ID.rx_tx_addr, 4);
	memcpy( &packet[5], g_model.rxID, 4);
	switch(type)
	{
	case AFHDS2A_PACKET_STICKS:
		packet[0] = 0x58;
		for (uint8_t ch = 0; ch < 14; ch++) {
			uint16_t channelMicros;
			if (g_model.failsafeRepeat)
				channelMicros = convert_failsafe_ppm(ch);
			else
			channelMicros = g_chans512[ch] / 2 + RADIO_PPM_CENTER;
			packet[9 + ch * 2] = channelMicros & 0xFF;
			packet[10 + ch * 2] = (channelMicros >> 8) & 0xFF;
		}
			#ifdef AFHDS2A_LQI_CH
				// override channel with LQI
				val = 2000 - 10*RX_LQI;
				packet[9+((AFHDS2A_LQI_CH-1)*2)] = val & 0xff;
				packet[10+((AFHDS2A_LQI_CH-1)*2)] = (val >> 8) & 0xff;
			#endif
			break;
		case AFHDS2A_PACKET_FAILSAFE:
			packet[0] = 0x56;
			for(uint8_t ch=0; ch<14; ch++)
			{
					if(g_model.failsafeMode)
					{ // Failsafe values
						uint16_t failsafeMicros = convert_failsafe_ppm(ch);
						packet[9 + ch*2] =  failsafeMicros & 0xff;
						packet[10+ ch*2] = ( failsafeMicros >> 8) & 0xff;
					}
					else
					{ // no values
						packet[9 + ch*2] = 0xff;
						packet[10+ ch*2] = 0xff;
					}
			}
			break;
		case AFHDS2A_PACKET_SETTINGS:
			packet[0] = 0xaa;
			packet[9] = 0xfd;
			packet[10]= 0xff;
			if(g_model.ServoFreq < 50 || g_model.ServoFreq>400) g_model.ServoFreq = 50;	// default is 50Hz
			packet[11]= g_model.ServoFreq;
			packet[12]= g_model.ServoFreq >> 8;
			if(g_model.PPMOut)
				packet[13] = 0x01;	// PPM output enabled
			else
				packet[13] = 0x00;
			packet[14]= 0x00;
			for(uint8_t i=15; i<37; i++)
				packet[i] = 0xff;
			packet[18] = 0x05;		// ?
			packet[19] = 0xdc;		// ?
			packet[20] = 0x05;		// ?
			if(g_model.IS_BUS)
				packet[21] = 0xdd;	// SBUS output enabled
			else
				packet[21] = 0xde;	// IBUS
			break;
	}
	if (hopping_frequency_no >= AFHDS2A_NUMFREQ)
		packet[37] = 0x00;
	else
		packet[37] = 0;//hopping_frequency_no+2;
}

void ActionAFHDS2A(void) {
  uint8_t Channel;
  static uint8_t packet_type;
  static uint16_t telem_counter;
  static uint16_t packet_counter = 0;
  A7105_AdjustLOBaseFreq();

  //----------------------------------------------------------------------------
  if (IS_BIND_DONE)
    RadioState = (RadioState & 0xF0) | AFHDS2A_DATA;
  switch (RadioState) {
  case ((TIM_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_BIND1)):
  case ((TIM_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_BIND2)):
  case ((TIM_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_BIND3)):
  case ((TIM_CALL << CALLER) | (RES << SEND_RES) | (AFHDS2A_BIND1)):
  case ((TIM_CALL << CALLER) | (RES << SEND_RES) | (AFHDS2A_BIND2)):
  case ((TIM_CALL << CALLER) | (RES << SEND_RES) | (AFHDS2A_BIND3)):
    goto SendBIND_;

  case ((TIM_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_BIND4)):
    goto SendBIND4_;

  case ((GPIO_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_BIND1)):
  case ((GPIO_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_BIND2)):
  case ((GPIO_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_BIND3)):
    goto EndSendBIND123_;

  case ((GPIO_CALL << CALLER) | (RES << SEND_RES) | (AFHDS2A_BIND1)):
  case ((GPIO_CALL << CALLER) | (RES << SEND_RES) | (AFHDS2A_BIND2)):
  case ((GPIO_CALL << CALLER) | (RES << SEND_RES) | (AFHDS2A_BIND3)):
    goto ResBIND123_;

  case ((TIM_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_DATA)):
  case ((TIM_CALL << CALLER) | (RES << SEND_RES) | (AFHDS2A_DATA)):
    goto SendData_;

  case ((GPIO_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_DATA)):
    goto EndSendData_;

  case ((GPIO_CALL << CALLER) | (RES << SEND_RES) | (AFHDS2A_DATA)):
    goto ResData_;

  default:
    return;
  }
//--------------------------------------------------------------------------
SendBIND4_: //--------------------------------------------------------------
  bind_phase++;
  if (bind_phase >= 4) {
    hopping_frequency_no = 1;
    RadioState = (RadioState & 0xF0) | AFHDS2A_DATA;
    SETBIT(RadioState, SEND_RES, SEND);
    BIND_DONE;
    return;
  }
SendBIND_: //--------------------------------------------------------------
  AFHDS2A_build_bind_packet();
  Channel = (packet_count % 2 ? 0x0d : 0x8c);
  SETBIT(RadioState, SEND_RES, SEND);
  goto Send_;
EndSendBIND123_: //-----------------------------------------------------------
  A7105_SetPower();
  A7105_SetTxRxMode(TXRX_OFF); // Turn LNA off since we are in near range and we want to prevent swamping
  A7105_Strobe(A7105_RX);
  EnableGIO();
  RadioState++;
  if ((RadioState & 0x0F) > AFHDS2A_BIND3)
    RadioState = (RadioState & 0xF0) | AFHDS2A_BIND1;
  SETBIT(RadioState, SEND_RES, RES);
  return;
ResBIND123_: //-----------------------------------------------------------
  A7105_ReadData(AFHDS2A_RXPACKET_SIZE);
  if ((packet[0] == 0xbc) & (packet[9] == 0x01)) {
    for (uint8_t i = 0; i < 4; i++) {
      g_model.rxID[i] = packet[5 + i];
    }
    RadioState = (RadioState & 0xF0) | AFHDS2A_BIND4;
    bind_phase = 0;
    SETBIT(RadioState, SEND_RES, SEND);
  }
  return;
SendData_: //--------------------------------------------------------------
  Channel = hopping_frequency[hopping_frequency_no++];
  AFHDS2A_build_packet(packet_type);
  SETBIT(RadioState, SEND_RES, SEND);
  if (hopping_frequency_no >= AFHDS2A_NUMFREQ) {
    hopping_frequency_no = 0;
    goto SendNoAntSwitch_;
  }
  goto Send_;
EndSendData_: //-----------------------------------------------------------
  A7105_SetPower();
  A7105_SetTxRxMode(RX_EN);
  A7105_Strobe(A7105_RX);
  if (!(packet_counter % 1569))
    packet_type = AFHDS2A_PACKET_FAILSAFE;
  else
    packet_type = AFHDS2A_PACKET_STICKS;
  SETBIT(RadioState, SEND_RES, RES);
  EnableGIO();
  if (telem_counter < 100)
    telem_counter++;
  else
    telem_status = 0;
  SETBIT(RadioState, SEND_RES, RES);
  return;
ResData_: //-----------------------------------------------------------
  A7105_ReadData(AFHDS2A_RXPACKET_SIZE);
  if (packet[0] == 0xAA && packet[9] == 0xFC) // RX is asking for settings
    packet_type = AFHDS2A_PACKET_SETTINGS;       
  if (packet[0] == 0xAA && packet[9] == 0xFD) // RX is asking for FailSafe
    packet_type = AFHDS2A_PACKET_FAILSAFE;       
  if (packet[0] == 0xAA || packet[0] == 0xAC) {
    if (!memcmp(&packet[1], ID.rx_tx_addr, 4)) { // Validate TX address
      AFHDS2A_update_telemetry();
      telem_counter = 0;
    }
  }
  SETBIT(RadioState, SEND_RES, SEND);
  return;
Send_: //---------------------------------------------------------------
  A7105_AntSwitch();
SendNoAntSwitch_:
  A7105_WriteData(AFHDS2A_TXPACKET_SIZE, Channel);
  EnableGIO();
  packet_count++;
  packet_counter++;
  return;
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
void initAFHDS2A() {
  protocol        = PROTO_AFHDS2A;
  RadioState      = ((TIM_CALL << CALLER) | (SEND << SEND_RES) | (AFHDS2A_DATA));
  ID.MProtocol_id = GetChipID();
  AFHDS2A_calc_channels();
  A7105_Init();
  packet_count         = 0;
  hopping_frequency_no = 0;
}
#endif