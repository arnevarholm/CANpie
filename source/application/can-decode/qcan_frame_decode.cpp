//====================================================================================================================//
// File:          qcan_frame_decode.cpp                                                                                      //
// Description:   QCAN classes - CAN frame                                                                            //
//                                                                                                                    //
// Copyright (C) MicroControl GmbH & Co. KG                                                                           //
// 53844 Troisdorf - Germany                                                                                          //
// www.microcontrol.net                                                                                               //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the   //
// following conditions are met:                                                                                      //
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions, the following   //
//    disclaimer and the referenced file 'LICENSE'.                                                                   //
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       //
//    following disclaimer in the documentation and/or other materials provided with the distribution.                //
// 3. Neither the name of MicroControl nor the names of its contributors may be used to endorse or promote products   //
//    derived from this software without specific prior written permission.                                           //
//                                                                                                                    //
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance     //
// with the License. You may obtain a copy of the License at                                                          //
//                                                                                                                    //
//    http://www.apache.org/licenses/LICENSE-2.0                                                                      //
//                                                                                                                    //
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed   //
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for  //
// the specific language governing permissions and limitations under the License.                                     //
//                                                                                                                    //
//====================================================================================================================//

/*--------------------------------------------------------------------------------------------------------------------*\
** Include files                                                                                                      **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/
#include <QCanFrame>
#include "qcan_frame_decode.hpp"

//----------------------------------------------------------------------------------------------------------------
// These three files are included with an absolute file path to ensure that the correct platform definition is
// passed to the CANpie structures and settings. The definitions for the CANpie message structure are done
// inside the file "cp_platform.h" located inside the "device/qcan" directory.
//
#include "../device/qcan/cp_platform.h"
#include "../misc/mc_compiler.h"
#include "../canpie-fd/canpie.h"

/*--------------------------------------------------------------------------------------------------------------------*\
** Definitions                                                                                                        **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/


QString QCanFrameDecode::decode_can_std() const
{
	QString clStringT;
	struct CpCanMsg_s tsCanMsgV;

	if (toCpCanMsg(&tsCanMsgV))
	{
		switch(tsCanMsgV.ulIdentifier)
		{
		case 0x00:
			clStringT += QString("NMT N:%2 Goto:").arg(tsCanMsgV.tuMsgData.aubByte[1]);
			switch (tsCanMsgV.tuMsgData.aubByte[0])
			{
			case 0x01:
				clStringT += QString("Operational");
				break;
			case 0x02:
				clStringT += QString("Stopped");
				break;
			case 0x80:
				clStringT += QString("Pre-operational");
				break;
			case 0x81:
				clStringT += QString("Reset node");
				break;
			case 0x82:
				clStringT += QString("Reset communication");
				break;
			default:
				clStringT += QString("??");
				break;
			}
			break;
		case 0x80:
			clStringT += QString("SYNC");
			break;
		case 0x100:
			clStringT += QString("TIME");
			break;
		case 0x200:
			clStringT += QString("200?");
			break;
		case 0x280:
			clStringT += QString("280?");
			break;
		case 0x300:
			clStringT += QString("300?");
			break;
		case 0x380:
			clStringT += QString("380?");
			break;
		case 0x400:
			clStringT += QString("400?");
			break;
		case 0x480:
			clStringT += QString("480?");
			break;

		case 0x500:
			clStringT += QString("500?");
			break;
		case 0x580:
			clStringT += QString("580?");
			break;
		case 0x600:
			clStringT += QString("600?");
			break;
		case 0x680:
			clStringT += QString("680?");
			break;
		default:
			if(tsCanMsgV.ulIdentifier > 0x80 && tsCanMsgV.ulIdentifier < 0x100)
			{
				clStringT += QString("EMCY N:%1").arg(tsCanMsgV.ulIdentifier & ~0x80);
			}
			else if(tsCanMsgV.ulIdentifier > 0x180 && tsCanMsgV.ulIdentifier < 0x200)
			{
				clStringT += QString("PDO1 TX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x180);				
			}
			else if (tsCanMsgV.ulIdentifier > 0x200 && tsCanMsgV.ulIdentifier < 0x280)
			{
				clStringT += QString("PDO1 RX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x200);
			}


			else if (tsCanMsgV.ulIdentifier > 0x280 && tsCanMsgV.ulIdentifier < 0x300)
			{
				clStringT += QString("PDO2 TX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x280);
			}
			else if (tsCanMsgV.ulIdentifier > 0x300 && tsCanMsgV.ulIdentifier < 0x380)
			{
				clStringT += QString("PDO2 RX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x300);
			}



			else if (tsCanMsgV.ulIdentifier > 0x380 && tsCanMsgV.ulIdentifier < 0x400)
			{
				clStringT += QString("PDO3 TX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x380);
			}
			else if (tsCanMsgV.ulIdentifier > 0x400 && tsCanMsgV.ulIdentifier < 0x480)
			{
				clStringT += QString("PDO3 RX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x400);
			}

			else if (tsCanMsgV.ulIdentifier > 0x480 && tsCanMsgV.ulIdentifier < 0x500)
			{
				clStringT += QString("PDO4 TX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x480);
			}
			else if (tsCanMsgV.ulIdentifier > 0x500 && tsCanMsgV.ulIdentifier < 0x580)
			{
				clStringT += QString("PDO4 RX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x500);
			}



			
			else if (tsCanMsgV.ulIdentifier > 0x580 && tsCanMsgV.ulIdentifier < 0x600)
			{
				clStringT += QString("SDO TX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x580);
				uint8_t ccs = (tsCanMsgV.tuMsgData.aubByte[0] >> 5);
				uint8_t n = (tsCanMsgV.tuMsgData.aubByte[0] >> 2) & 0x3;
				uint8_t e = (tsCanMsgV.tuMsgData.aubByte[0] >> 1) & 0x1;
				uint8_t s = (tsCanMsgV.tuMsgData.aubByte[0] >> 0) & 0x1;
				clStringT += QString(" ccs:%1 n:%2 e:%3 s:%4").arg(ccs).arg(n).arg(e).arg(s);
				uint16_t index = tsCanMsgV.tuMsgData.aubByte[1] & (tsCanMsgV.tuMsgData.aubByte[1] << 8);
				clStringT += QString("Ix:%1 Six:%2").arg(index).arg(tsCanMsgV.tuMsgData.aubByte[2]);
				
				



				
			}

			else if (tsCanMsgV.ulIdentifier > 0x600 && tsCanMsgV.ulIdentifier < 0x680)
			{
				clStringT += QString("SDO RX N:%1").arg(tsCanMsgV.ulIdentifier & ~0x600);
			}


			




			else if (tsCanMsgV.ulIdentifier > 0x700 && tsCanMsgV.ulIdentifier < 0x800)
			{				
				clStringT += QString("HB N:%1 S:").arg((tsCanMsgV.ulIdentifier & ~0x700));
				switch(tsCanMsgV.tuMsgData.aubByte[0])
				{
				case 0x00:
					clStringT += QString("BootUp");
					break;
				case 0x04:
					clStringT += QString("Stopped");
					break;
				case 0x05:
					clStringT += QString("Operational");
					break;
				case 0x7F:
					clStringT += QString("Pre-operational");
					break;
				default:
					clStringT += QString("??");
					break;
				}				
				
			}
			else if (tsCanMsgV.ulIdentifier > 0x000 && tsCanMsgV.ulIdentifier < 0x080)
			{
				clStringT += QString("Unused1");
			}
			else if (tsCanMsgV.ulIdentifier > 0x100 && tsCanMsgV.ulIdentifier < 0x180)
			{
				clStringT += QString("Unused2");
			}
			else if (tsCanMsgV.ulIdentifier > 0x100 && tsCanMsgV.ulIdentifier < 0x180)
			{
				clStringT += QString("0x101-0x17FF");
			}
			else if (tsCanMsgV.ulIdentifier > 0x77F && tsCanMsgV.ulIdentifier < 0x7E4)
			{
				clStringT += QString("0x780-0x7E3");
			}
			else if (tsCanMsgV.ulIdentifier > 0x7E5 && tsCanMsgV.ulIdentifier < 0x780)
			{
				clStringT += QString("0x7E6-0x7FF");
			}
			else
			{
				clStringT += QString("%1??").arg(tsCanMsgV.ulIdentifier);
				
			}
			break;
		}
		
		
	}
	return clStringT;
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanFrameDecode::toString()                                                                                        //
// Decode CAN data																									  //
//--------------------------------------------------------------------------------------------------------------------//
QString QCanFrameDecode::toString(const bool &btShowTimeR, const bool &btShowDataSizeR) const
{
   //---------------------------------------------------------------------------------------------------
   // setup a string object
   //
   QString clStringT;
   uint32_t ulSecondsT = 0;
   uint32_t ulNanoSecT = 0;

   if (btShowTimeR == true)
   {
      ulSecondsT = this->timeStamp().seconds();
      ulNanoSecT = this->timeStamp().nanoSeconds();
      ulNanoSecT = ulNanoSecT / 10000;
      clStringT = QString("%1.%2 ").arg(ulSecondsT, 5, 10).arg(ulNanoSecT, 5, 10, QChar('0'));
   }

   //---------------------------------------------------------------------------------------------------
   // check frame type: CAN data frame
   //
   if (frameType() == eFRAME_TYPE_DATA)
   {
      //-------------------------------------------------------------------------------------------
      // print identifier
      //
      clStringT += QString("%1  ").arg(identifier(), 8, 16).toUpper();

      //-------------------------------------------------------------------------------------------
      // print frame format
      //
      switch (frameFormat())
      {
      case eFORMAT_CAN_STD:
		  clStringT += decode_can_std();
         break;

      case eFORMAT_CAN_EXT:
         clStringT += "CEFF ";
         break;

      case eFORMAT_FD_STD:
         clStringT += "FBFF ";
         break;

      case eFORMAT_FD_EXT:
         clStringT += "FEFF ";
         break;

      default:

         break;
      }

      //-------------------------------------------------------------------------------------------
      // print frame flag for classic CAN : RTR           -> ubFrameFlagsT = 4
      // print frame flag for CAN FD      : BRS and ESI   -> ubFrameFlagsT = 1, 2, 3
      //
      uint8_t ubFrameFlagsT = 0;
      if (bitrateSwitch())
      {
         ubFrameFlagsT += 1;
      }

      if (errorStateIndicator())
      {
         ubFrameFlagsT += 2;
      }

      if (isRemote())
      {
         ubFrameFlagsT += 4;
      }

      switch (ubFrameFlagsT)
      {

      case 1:
         clStringT += "BRS     ";
         break;

      case 2:
         clStringT += "ESI     ";
         break;

      case 3:
         clStringT += "BRS,ESI ";
         break;

      case 4:
         clStringT += "RTR     ";
         break;

      default:
         clStringT += "        ";
         break;
      }

      //-------------------------------------------------------------------------------------------
      // print DLC or data size
      //
      if (btShowDataSizeR)
      {
      }
      else
      {
         clStringT += QString("%1  ").arg(dlc(), 2, 10);
      }

      //-------------------------------------------------------------------------------------------
      // print data
      //
      uint8_t ubDataSizeT = dataSize();
      if (isRemote())
      {
         ubDataSizeT = 0;
      }
      for (uint8_t ubCntT = 0; ubCntT < ubDataSizeT; ubCntT++)
      {
         //-----------------------------------------------------------------------------------
         // print a newline and 19/31 spaces after 32 data bytes
         //
         if ((ubCntT > 0) && ((ubCntT % 32) == 0))
         {
            if (btShowTimeR == true)
            {
               clStringT += "\n                               ";
            }
            else
            {
               clStringT += "\n                   ";
            }
         }

         //-----------------------------------------------------------------------------------
         // print data with 2 digits and in hex format
         //
         clStringT += QString("%1 ").arg(data(ubCntT), 2, 16, QLatin1Char('0')).toUpper();
      }
   }

   //---------------------------------------------------------------------------------------------------
   // check frame type: CAN error frame
   //
   if (frameType() == eFRAME_TYPE_ERROR)
   {
      //-------------------------------------------------------------------------------------------
      // print error frame, show current state first
      //
      clStringT += "   Error frame ----------  ";

      switch (this->errorState())
      {
      case eCAN_STATE_STOPPED:
         clStringT += "CAN controller stopped";
         break;

      case eCAN_STATE_SLEEPING:
         clStringT += "CAN controller sleeping";
         break;

      case eCAN_STATE_BUS_ACTIVE:
         clStringT += "Error active";
         break;

      case eCAN_STATE_BUS_WARN:
         clStringT += "Warning level reached";
         break;

      case eCAN_STATE_BUS_PASSIVE:
         clStringT += "Error passive";
         break;

      case eCAN_STATE_BUS_OFF:
         clStringT += "Bus off";
         break;

      default:
         clStringT += "Unknown state";
         break;
      }

      //-------------------------------------------------------------------------------------------
      // print current values of error counters
      //
      clStringT += QString(", Error counter: Rcv=%1, Trm=%2 ").arg(errorCounterReceive()).arg(errorCounterTransmit());

      //-------------------------------------------------------------------------------------------
      // print current error type
      //
      switch (this->errorType())
      {
      case eERROR_TYPE_NONE:

         break;

      case eERROR_TYPE_BIT0:
      case eERROR_TYPE_BIT1:
         clStringT += ", bit error";
         break;

      case eERROR_TYPE_STUFF:
         clStringT += ", stuff error";
         break;

      case eERROR_TYPE_FORM:
         clStringT += ", form error";
         break;

      case eERROR_TYPE_CRC:
         clStringT += ", CRC error";
         break;

      case eERROR_TYPE_ACK:
         clStringT += ", acknowledgement error";
         break;
      }
   }

   return (clStringT);
}


