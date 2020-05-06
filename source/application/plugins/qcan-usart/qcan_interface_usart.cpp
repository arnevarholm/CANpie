//====================================================================================================================//
// File:          qcan_interface_usart.cpp                                                                            //
// Description:   QCan Inteface for USART                                                                             //
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

#include "qcan_interface_usart.hpp"

/*--------------------------------------------------------------------------------------------------------------------*\
** Definitions                                                                                                        **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/


bool btWrtieIsPendingG;
QVector<CpCanMsg_ts> atsReadMessageListG;
QVector<QCanFrame> atsWriteMessageListG;
CpPort_ts tsPortP;
quint32 ulTrmCountG;

//--------------------------------------------------------------------------------------------------------------------//
// AppCanErrHandler()                                                                                                 //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
uint8_t AppCanErrHandler(CpState_ts  *ptsErrV)
{
   Q_UNUSED(ptsErrV);

   return eCP_ERR_NONE;
}

//--------------------------------------------------------------------------------------------------------------------//
// AppCanRcvHandler()                                                                                                 //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
uint8_t AppCanRcvHandler(CpCanMsg_ts *ptsMsgV, uint8_t ubBufferV)
{
   Q_UNUSED(ubBufferV);

   //---------------------------------------------------------------------------------------------------
   // append new CAN message
   //
   atsReadMessageListG.append(*ptsMsgV);

   return eCP_ERR_NONE;
}

//--------------------------------------------------------------------------------------------------------------------//
// AppCanTrmHandler()                                                                                                 //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
uint8_t AppCanTrmHandler(CpCanMsg_ts *ptsMsgV, uint8_t ubBufferV)
{
   //---------------------------------------------------------------------------------------------------
   // handle transmit event from corresponding buffer
   //
   switch (ubBufferV)
   {
      case eCP_BUFFER_1:

         //-----------------------------------------------------------------------------------
         // clear write pending flag, so next message can be written
         //
         btWrtieIsPendingG = false;

         //-----------------------------------------------------------------------------------
         // Only Buffer 1 will be used for sending all CANpie mesages via USART
         // Trigger next one after the transmission has been finished
         //
         transmitFrame();

         break;
      default:
         break;
   }

   return eCP_ERR_NONE;
}

//--------------------------------------------------------------------------------------------------------------------//
// transmitFrame()                                                                                                    //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void transmitFrame(void)
{
   QCanUsart &clCpUsartT = QCanUsart::getInstance();
   CpStatus_tv tvStatusT = eCP_ERR_NONE;
   QCanFrame clFrameT;

   //---------------------------------------------------------------------------------------------------
   // check for actually running transmission and quit if one is pending
   //
   if (btWrtieIsPendingG == true)
   {
      return;
   }
   btWrtieIsPendingG = true;

   //---------------------------------------------------------------------------------------------------
   // check for pending missages for transmission
   //
   if (atsWriteMessageListG.isEmpty() == true)
   {
      btWrtieIsPendingG = false;
      return;
   }

   //---------------------------------------------------------------------------------------------------
   // get the next CANpie message from the list
   //
   clFrameT = atsWriteMessageListG.first();
   atsWriteMessageListG.removeFirst();

   //---------------------------------------------------------------------------------------------------
   // setup buffer for transmission and send message
   //
   if (clFrameT.isExtended())
   {
      tvStatusT += clCpUsartT.CpUsartBufferConfig(&tsPortP, eCP_BUFFER_1,
                                                   (uint32_t) clFrameT.identifier(),
                                                   CP_MASK_EXT_FRAME,
                                                   CP_MSG_FORMAT_CEFF,
                                                   eCP_BUFFER_DIR_TRM);
   }
   else
   {
      tvStatusT += clCpUsartT.CpUsartBufferConfig(&tsPortP, eCP_BUFFER_1,
                                                   (uint32_t) clFrameT.identifier(),
                                                   CP_MASK_EXT_FRAME,
                                                   CP_MSG_FORMAT_CBFF,
                                                   eCP_BUFFER_DIR_TRM);
   }

   tvStatusT += clCpUsartT.CpUsartBufferSetDlc(&tsPortP, eCP_BUFFER_1,
                                                clFrameT.dlc());

   tvStatusT += clCpUsartT.CpUsartBufferSetData(&tsPortP, eCP_BUFFER_1,
                                                 (uint8_t *)clFrameT.data().data(),
                                                 0,
                                                 clFrameT.dlc());

   tvStatusT += clCpUsartT.CpUsartBufferSend(&tsPortP, eCP_BUFFER_1);

   //---------------------------------------------------------------------------------------------------
   // check transmission was succesfull
   //
   if (tvStatusT == eCP_ERR_NONE)
   {
      ulTrmCountG++;
   } else
   {
      qCritical() << "FAIL to send a CANpie Message via USART buffer with error 0x" + QString::number(tvStatusT,16);
   }
}

/*--------------------------------------------------------------------------------------------------------------------*\
** Class methods                                                                                                      **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart()                                                                                               //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QCanInterfaceUsart::QCanInterfaceUsart(uint16_t uwDeviceNrV, QString clNameV)
{
   if (!clCpUsartP.isAvailable())
   {
      qCritical() << "QCanInterfaceUsart(): USART interface is available!";
   }

   clUsartNameP = clNameV;
   uwUsartNumberP = uwDeviceNrV;

   //---------------------------------------------------------------------------------------------------
   // setup interface variables
   //
   teConnectedP = UnconnectedState;

   //---------------------------------------------------------------------------------------------------
   // clear error state
   //
   teErrorStateP = eCAN_STATE_BUS_ACTIVE;

   teCanModeP = eCAN_MODE_STOP;

   atsReadMessageListG.clear();
   atsWriteMessageListG.clear();
}


//----------------------------------------------------------------------------//
// ~QCanInterfaceUsart()                                                      //
//                                                                            //
//----------------------------------------------------------------------------//
QCanInterfaceUsart::~QCanInterfaceUsart()
{
   qDebug() << "QCanInterfaceUsart::~QCanInterfaceUsart()";

   if (clCpUsartP.isAvailable())
   {
//      pclEventTimerP->stop();
//      pclEventTimerP->disconnect();
//      pclEventTimerP->deleteLater();

      if (teConnectedP == ConnectedState)
      {
         teConnectedP = UnconnectedState;
         clCpUsartP.CpUsartDriverRelease(&tsPortP);
      }

   }


}

//----------------------------------------------------------------------------//
// connect()                                                                  //
//                                                                            //
//----------------------------------------------------------------------------//
QCanInterface::InterfaceError_e QCanInterfaceUsart::connect(void)
{
   InterfaceError_e teReturnT = eERROR_LIBRARY;
   CpStatus_tv      tvStatusT;

   qDebug() << QString("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++QCanInterfaceUsart::connect()...");

    emit addLogMessage(tr("Connect ") + clUsartNameP + tr(" from 'QCan USART' plugin"), eLOG_LEVEL_INFO);

   //----------------------------------------------------------------
   //
   //
   if (clCpUsartP.isAvailable())
   {
      clCpUsartP.setDeviceName(clUsartNameP);
      tvStatusT = clCpUsartP.CpUsartDriverInit(uwUsartNumberP+1,&tsPortP,0);
      if (tvStatusT == eCP_ERR_NONE)
      {
         tvStatusT += clCpUsartP.CpUsartIntFunctions(&tsPortP,
                                                      AppCanRcvHandler,
                                                      AppCanTrmHandler,
                                                      AppCanErrHandler);

         //----------------------------------------------------------------
         // configure reception of global buffers for
         // standard and extended identifiers
         //
         tvStatusT += clCpUsartP.CpUsartBufferConfig(&tsPortP, eCP_BUFFER_3,
                                                     (uint32_t) 0x71,
                                                     0x00,
                                                    CP_MSG_FORMAT_CBFF,
                                                eCP_BUFFER_DIR_RCV);
         tvStatusT += clCpUsartP.CpUsartBufferSetDlc(&tsPortP, eCP_BUFFER_3, 8);

         tvStatusT += clCpUsartP.CpUsartBufferConfig(&tsPortP, eCP_BUFFER_4,
                                                (uint32_t) 0x71,
                                                0x00,
                                                CP_MSG_FORMAT_CEFF,
                                                eCP_BUFFER_DIR_RCV);
         tvStatusT += clCpUsartP.CpUsartBufferSetDlc(&tsPortP, eCP_BUFFER_4, 8);


         if (tvStatusT != eCP_ERR_NONE)
         {
            emit addLogMessage(tr("Something went wrong with buffer initialisation"), eLOG_LEVEL_WARN);
         }

         teConnectedP = ConnectedState;
         atsReadMessageListG.clear();
         atsWriteMessageListG.clear();

         emit connectionChanged(ConnectedState);

//         pclEventTimerP->start(1);
         teReturnT = eERROR_NONE;

         //---------------------------------------------------------------------------------------------------
         // configure the refresh timer which updates all statistic information and sends some signals
         //
         QTimer::singleShot(10, this, SLOT(onTimerEvent()));
      }
      else
      {
         qWarning() << QString("QCanInterfaceUsart::connect(0x" +QString::number(tsPortP.ubPhyIf,16)+", "+clUsartNameP+")") << "fail with error:" << clCpUsartP.formatedError(tvStatusT);
      }
   }

   return teReturnT;
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::connectionState()                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QCanInterface::ConnectionState_e QCanInterfaceUsart::connectionState(void)
{
   return teConnectedP;
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::disableFeatures()                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void QCanInterfaceUsart::disableFeatures(uint32_t ulFeatureMaskV)
{
   Q_UNUSED(ulFeatureMaskV)
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::disconnect()                                                                                   //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QCanInterface::InterfaceError_e QCanInterfaceUsart::disconnect()
{
   CpStatus_tv tvStatusT;
   InterfaceError_e teReturnT = eERROR_LIBRARY;

   if (clCpUsartP.isAvailable())
   {
      tvStatusT = clCpUsartP.CpUsartDriverRelease(&tsPortP);
      if (tvStatusT == eCP_ERR_NONE)
      {
//         pclEventTimerP->stop();

         atsReadMessageListG.clear();
         atsWriteMessageListG.clear();
         teConnectedP = UnconnectedState;
         emit connectionChanged(UnconnectedState);
//         emit disconnected();
         teReturnT = eERROR_NONE;
      }
      else
      {
         qWarning() << QString("QCanInterfaceUsart::disconnect(0x" +QString::number(tsPortP.ubPhyIf,16)+", "+clUsartNameP+")") << "fail with error:" << clCpUsartP.formatedError(tvStatusT);
      }
   }

   return teReturnT;
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::disableFeatures()                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void QCanInterfaceUsart::enableFeatures(uint32_t ulFeatureMaskV)
{
   Q_UNUSED(ulFeatureMaskV)
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::icon()                                                                                         //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QIcon QCanInterfaceUsart::icon(void)
{
   if (clCpUsartP.isAvailable())
   {
      return QIcon(":/images/usart_plugin_256.png");
   }

   return QIcon(":/images/usart_plugin_fail_256.png");
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::name()                                                                                         //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QString QCanInterfaceUsart::name()
{
   qDebug() << "QCanInterfaceUsart::name()";
   if (clCpUsartP.isAvailable())
   {
      return clUsartNameP;
   }

   return QString("PCAN Basic library is not available");
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceTemplate::onTimerEven()                                                                               //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void QCanInterfaceUsart::onTimerEvent(void)
{
//   if (connectionState() == ConnectedState)
//   {
//      if (btHasReceivedFrameP == true)
//      {
//         btHasReceivedFrameP = false;
//         emit readyRead();
//      }

//      if (teErrorStateP != clErrFrameP.errorState())
//      {
//         teErrorStateP = clErrFrameP.errorState();
//         emit stateChanged(teErrorStateP);
//      }
//      QTimer::singleShot(50, this, SLOT(onTimerEvent()));
//   }

//   if (read(clRcvFrameP) == eERROR_NONE)
//   {
//      btHasReceivedFrameP = true;
//      emit readyRead();
//   }

//   qDebug() << "onTimerEvent";
   if (atsReadMessageListG.isEmpty() == false)
   {
//      qDebug() << "atsReadMessageListG is NOT empty!";
      emit readyRead();
   } else {
      QTimer::singleShot(50, this, SLOT(onTimerEvent()));
   }
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::read()                                                                                         //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QCanInterface::InterfaceError_e  QCanInterfaceUsart::read( QCanFrame &clFrameR)
{
   InterfaceError_e  clRetValueT = eERROR_NONE;
   uint8_t           ubCntT;
   QCanFrame         clCanFrameT;
   CpCanMsg_ts       tsCanMessageT;

   qDebug() << "read( QCanFrame &clFrameR)";

   //----------------------------------------------------------------
   // check channel is available
   //
   if (!clCpUsartP.isAvailable())
   {
      return eERROR_LIBRARY;
   }

   //----------------------------------------------------------------
   // check channel is connected
   //
   if (teConnectedP != ConnectedState)
   {
      return eERROR_DEVICE;
   }

   //----------------------------------------------------------------
   // check fifo contains some messages
   //
   if (atsReadMessageListG.isEmpty())
   {
      clRetValueT = eERROR_FIFO_RCV_EMPTY;
      QTimer::singleShot(50, this, SLOT(onTimerEvent()));
   }

   //----------------------------------------------------------------
   // get next message from FIFO
   //
   else
   {
      // get message
      tsCanMessageT = atsReadMessageListG.at(0);
      atsReadMessageListG.removeAt(0);

      //------------------------------------------------
      // Classical CAN frame with standard or
      // extended identifier
      //
      if (CpMsgIsExtended(&tsCanMessageT))
      {
         clCanFrameT.setFrameFormat(QCanFrame::eFORMAT_CAN_EXT);
      }
      else
      {
         clCanFrameT.setFrameFormat(QCanFrame::eFORMAT_CAN_STD);
      }

      //------------------------------------------------
      // Classical CAN remote frame
      //
      if (CpMsgIsRemote(&tsCanMessageT))
      {
         clCanFrameT.setRemote(true);
      }

      //------------------------------------------------
      // copy the identifier
      //
      clCanFrameT.setIdentifier(CpMsgGetIdentifier(&tsCanMessageT));

      //------------------------------------------------
      // copy the DLC
      //
      clCanFrameT.setDlc(CpMsgGetDlc(&tsCanMessageT));

      //------------------------------------------------
      // copy the data
      //
      for (ubCntT = 0; ubCntT < clCanFrameT.dataSize(); ubCntT++)
      {
         clCanFrameT.setData(ubCntT, CpMsgGetData(&tsCanMessageT,ubCntT));
      }

      // \todo no timestamp is supproted
      clCanFrameT.setTimeStamp(0);

      qDebug() << "get new CAN (USART) message...";
      //------------------------------------------------
      // increase statistic counter
      //
      tsStatisticP.ulRcvCount++;

      //------------------------------------------------
      // copy the CAN frame
      //
      clFrameR = clCanFrameT;
   }

   return (clRetValueT);
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::reset()                                                                                        //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QCanInterface::InterfaceError_e  QCanInterfaceUsart::reset()
{
   qDebug() << "QCanInterfaceUsart::reset()";

   //----------------------------------------------------------------
   // check lib is available
   //
   if (!clCpUsartP.isAvailable())
   {
      return eERROR_LIBRARY;
   }

   //----------------------------------------------------------------
   // reset statistic values
   //
   tsStatisticP.ulErrCount = 0;
   tsStatisticP.ulRcvCount = 0;
   ulTrmCountG = 0;

   //----------------------------------------------------------------
   // perform a hardware reset only if
   // it has been initialised before
   //
   if (teConnectedP == ConnectedState)
   {
      disconnect();
      connect();
      setBitrate(ulUsartBitrateP,eCP_BITRATE_NONE);
   }

   return setMode(teCanModeP);
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::setBitrate()                                                                                   //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QCanInterface::InterfaceError_e QCanInterfaceUsart::setBitrate(int32_t slNomBitRateV, int32_t slDatBitRateV)
{
   InterfaceError_e  teReturnT = eERROR_LIBRARY;
   CpStatus_tv       tvStatusT;

   qDebug() << "QCanInterfaceUsart::setBitrate()";

   if (clCpUsartP.isAvailable())
   {
      slNomBitRateV = eCP_BITRATE_125K;
      slDatBitRateV = eCP_BITRATE_NONE;

      ulUsartBitrateP = slNomBitRateV;
      tvStatusT = clCpUsartP.CpUsartBitrate(&tsPortP, slNomBitRateV, slDatBitRateV);
      if (tvStatusT == eCP_ERR_NONE)
      {
         teReturnT = eERROR_NONE;
      }
      else
      {
         qWarning() << QString("QCanInterfaceUsart::disconnect(0x" +QString::number(tsPortP.ubPhyIf,16)+", "+clUsartNameP+")") << "fail with error:" << clCpUsartP.formatedError(tvStatusT);
      }
   }

   return teReturnT;
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::setMode()                                                                                      //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QCanInterface::InterfaceError_e	QCanInterfaceUsart::setMode(const CAN_Mode_e teModeV)
{
   CpStatus_tv tvStatusT;
   //CpMode_e    teCanModeP

   qDebug() << "QCanInterfaceUsart::setMode()";

   //----------------------------------------------------------------
   // check lib is available
   //
   if (!clCpUsartP.isAvailable())
   {
      return eERROR_LIBRARY;
   }

   //----------------------------------------------------------------
   // select mode
   //
   switch (teModeV)
   {
      case eCAN_MODE_START :

         //---------------------------------------------------
         // reset statistic values
         //
         tsStatisticP.ulErrCount = 0;
         tsStatisticP.ulRcvCount = 0;
         ulTrmCountG = 0;

         tvStatusT = clCpUsartP.CpUsartCanMode(&tsPortP, eCAN_MODE_START);
         if (tvStatusT == eCP_ERR_NONE)
         {
            teCanModeP = eCAN_MODE_START;
         } else
         {
            qWarning() << QString("QCanInterfaceUsart::setMode(0x" +QString::number(tsPortP.ubPhyIf,16)+", "+clUsartNameP+")") << "fail with error:" << clCpUsartP.formatedError(tvStatusT);
         }
         break;

      case eCAN_MODE_STOP :
         teCanModeP = eCAN_MODE_STOP;
         break;

      default :
         return eERROR_MODE;
         break;
   }

   return eERROR_NONE;
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::state()                                                                                        //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
CAN_State_e QCanInterfaceUsart::state(void)
{
   return (teErrorStateP);
}


//--------------------------------------------------------------------------------------------------------------------//
// QCanInterfaceUsart::statistic()                                                                                    //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QCanInterface::InterfaceError_e	QCanInterfaceUsart::statistic(QCanStatistic_ts &clStatisticR)
{
   if(clCpUsartP.isAvailable())
   {
      clStatisticR.ulErrCount = tsStatisticP.ulErrCount;
      clStatisticR.ulRcvCount = tsStatisticP.ulRcvCount;
      clStatisticR.ulTrmCount = ulTrmCountG;
   }
   else
   {
      return(eERROR_LIBRARY);
   }


   return(eERROR_NONE);
}


//----------------------------------------------------------------------------//
// supportedFeatures()                                                        //
//                                                                            //
//----------------------------------------------------------------------------//
uint32_t QCanInterfaceUsart::supportedFeatures()
{
   uint32_t ulFeaturesT = 0;

   qDebug() << "QCanInterfaceUsart::supportedFeatures()";

   if(clCpUsartP.isAvailable())
   {
      ulFeaturesT = QCAN_IF_SUPPORT_SPECIFIC_CONFIG;
   }

   return(ulFeaturesT);
}

//------------------------------------------------------------------------------------------------------
// nice macros to make a string from a definition, used in version()
//
#define STRINGIFY(x)    #x
#define TOSTRING(x)     STRINGIFY(x)

//--------------------------------------------------------------------------------------------------------------------//
// version()                                                                                                          //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
QString QCanInterfaceUsart::version(void)
{
   QString clVersionT;
   const char szPluginC[]   = TOSTRING(TARGET_NAME);

   clVersionT  = QString("Plugin ");
   clVersionT.append(szPluginC);
   clVersionT += QString(" version %1.%2,").arg(VERSION_MAJOR).arg(VERSION_MINOR);
   clVersionT += QString(" build %1,").arg(VERSION_BUILD);

   return (clVersionT);
}


//----------------------------------------------------------------------------//
// write()                                                                    //
//                                                                            //
//----------------------------------------------------------------------------//
QCanInterface::InterfaceError_e QCanInterfaceUsart::write(const QCanFrame &clFrameR)
{
//   qDebug() << "QCanInterfaceUsart::write()";

   //----------------------------------------------------------------
   //
   //
   if (!clCpUsartP.isAvailable())
   {
      qDebug() << "QCanInterfaceUsart::write() LIB";
      return eERROR_LIBRARY;
   }

   //----------------------------------------------------------------
   // support only classic can messages
   //
   if (clFrameR.frameFormat() > QCanFrame::eFORMAT_CAN_EXT)
   {
      qDebug() << "QCanInterfaceUsart::write() EXT";
      return eERROR_MODE;
   }


   //----------------------------------------------------------------
   // check no write is pending
   //
  if (atsWriteMessageListG.size() < 32)
  {
//      qDebug() << "QCanInterfaceUsart::write() OK: ID" << QString::number(clFrameR.identifier(),16) << "DLC" << QString::number(clFrameR.dlc(),16);
      QCanFrame clPendingFrameT = clFrameR;
      atsWriteMessageListG.append(clPendingFrameT);

      // trigger transmission
      transmitFrame();

      return eERROR_NONE;
   }

  qDebug() << "QCanInterfaceUsart::write() FIFO FULL";

   return eERROR_FIFO_TRM_FULL;

}

//----------------------------------------------------------------------------//
// configureDevice()                                              //
//                                                                            //
//----------------------------------------------------------------------------//
bool QCanInterfaceUsart::configureDevice(void)
{
   bool btStateT = false; // if true than new initialisation shoud be done
   qDebug() << "QCanInterfaceUsart::configureDevice";

   //----------------------------------------------------------------
   // create configuration GUI
   //
   qDebug() << "value from current connfig, name: "<<clCpUsartP.currentConfig().clName;
   qDebug() << "value from current connfig, dir: "<<clCpUsartP.currentConfig().ubDirection;
   qDebug() << "value from current connfig, mode: "<<clCpUsartP.currentConfig().ubMode;
   qDebug() << "value from current connfig, bitrate: "<<clCpUsartP.currentConfig().ulBaud;

   QCanConfig *pclConfigGuiT = new QCanConfig(clCpUsartP.currentConfig());
   pclConfigGuiT->exec();

   if (pclConfigGuiT->result() == QDialog::Accepted)
   {
      /*! Put here Code to reinitialise
       *  the Hardware or other changed parameters
       *
       */
      clCpUsartP.setConfig(pclConfigGuiT->currentConfig());

      btStateT = true;
   }
   pclConfigGuiT->deleteLater();

   return btStateT;
}

