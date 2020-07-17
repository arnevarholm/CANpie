//====================================================================================================================//
// File:          qcan_send.hpp                                                                                       //
// Description:   Send CAN messages                                                                                   //
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

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>

#include "qcan_namespace.hpp"

#include <QCanServerSettings>
#include <QCanSocket>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include "VCanMC.h"

class QCanRouteVcan : public QObject
{
  Q_OBJECT

  public:
   QCanRouteVcan(QObject* parent = 0);

   QNetworkDatagram qtMulticastDatagram;
   CpCanLanMsg_s clMulticastLanMessage;
  
   struct
   {
      QHostAddress hostAddress;  // Hostadapter of adapter to send to
      quint16 Port;
      QHostAddress Group;  
   } qtMulticast;           // Route to this Multicast group and port
	
  signals:
   void finished();

  public slots:
   void aboutToQuitApp(void);
   void receiveMulticastDatagrams();
   void runCmdParser(void);

   void socketConnected();
   void socketDisconnected();
   void socketError(QAbstractSocket::SocketError teSocketErrorV);
  
   void  receiveCANpieDatagrams(void);
   void quitApplication();

  private:
   void sendCANpieServerDatagram(QCanFrame& frame);
   QCoreApplication* pclAppP;

   QCommandLineParser clCmdParserP;
   QCanServerSettings clServerP;

   QCanSocket clCanSocketP;
   uint8_t ubChannelP;

   QCanFrame clTransmitFrameP;
   QCanFrame clReceiveFrameP;
   QMutex clReceiveMutexP;
   QQueue<QCanFrame> clReceiveFifoP;
   uint32_t ulFrameIdP;
   // uint32_t             ulFrameGapP;
   uint8_t ubFrameDlcP;
   uint8_t ubFrameFormatP;
   uint8_t aubFrameDataP[QCAN_MSG_DATA_MAX];
   uint32_t ulFrameCountP;
   QUdpSocket clVcanTxSockettP;
   QUdpSocket clVcanRxSockettP;
};
