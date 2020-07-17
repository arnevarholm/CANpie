//====================================================================================================================//
// File:          qcan_send.cpp                                                                                       //
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

#include "qcan_routevcan.hpp"

#include "qcan_server_settings.hpp"

#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QNetworkDatagram>
#include <QHostAddress>
#include <QNetworkInterface>

//--------------------------------------------------------------------------------------------------------------------//
// main()                                                                                                             //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
int main(int argc, char* argv[])
{
   QCoreApplication clAppT(argc, argv);
   QCoreApplication::setApplicationName("can-routevcan");

   //---------------------------------------------------------------------------------------------------
   // get application version (defined in .pro file)
   //
   QString clVersionT;
   clVersionT += QString("%1.").arg(VERSION_MAJOR);
   clVersionT += QString("%1.").arg(VERSION_MINOR, 2, 10, QLatin1Char('0'));
   clVersionT += QString("%1,").arg(VERSION_BUILD, 2, 10, QLatin1Char('0'));
   clVersionT += " build on ";
   clVersionT += __DATE__;
   QCoreApplication::setApplicationVersion(clVersionT);

   //---------------------------------------------------------------------------------------------------
   // create the main class
   //
   QCanRouteVcan clMainT;

   //---------------------------------------------------------------------------------------------------
   // connect the signals
   //
   QObject::connect(&clMainT, SIGNAL(finished()), &clAppT, SLOT(quit()));
   QObject::connect(&clAppT, SIGNAL(aboutToQuit()), &clMainT, SLOT(aboutToQuitApp()));

   //---------------------------------------------------------------------------------------------------
   // This code will start the messaging engine in QT and in 10 ms it will start the execution in the
   // clMainT.runCmdParser() routine.
   //
   QTimer::singleShot(10, &clMainT, SLOT(runCmdParser()));

   clAppT.exec();
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanRouteVcan()                                                                                                         //
// constructor                                                                                                        //
//--------------------------------------------------------------------------------------------------------------------//
QCanRouteVcan::QCanRouteVcan(QObject* parent) : QObject(parent)
{
   //---------------------------------------------------------------------------------------------------
   // get the instance of the main application
   //
   pclAppP = QCoreApplication::instance();

   //---------------------------------------------------------------------------------------------------
   // connect signals for socket operations
   //
   QObject::connect(&clCanSocketP, SIGNAL(connected()), this, SLOT(socketConnected()));

   QObject::connect(&clCanSocketP, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

   QObject::connect(&clCanSocketP, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));



   QObject::connect(&clCanSocketP, &QCanSocket::readyRead, this, &QCanRouteVcan::receiveCANpieDatagrams);

   //---------------------------------------------------------------------------------------------------
   // set default values
   //
   ulFrameCountP = 0;
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanRouteVcan::aboutToQuitApp()                                                                                         //
// shortly after quitApplication is called the CoreApplication will signal this routine: delete objects / clean up               //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::aboutToQuitApp()
{
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanRouteVcan::quitApplication()                                                                                                   //
// call this routine to quitApplication the application                                                                          //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::quitApplication()
{
   clCanSocketP.disconnectNetwork();
   emit finished();
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanDump::runCmdParser()                                                                                           //
// 10ms after the application starts this method will parse all commands                                              //
// can-routevcan --seg | -s 5 can1
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::runCmdParser(void)
{
   //---------------------------------------------------------------------------------------------------
   // setup command line parser
   //
   clCmdParserP.setApplicationDescription(tr("Route between CAN interface and VCAN"));
   clCmdParserP.addHelpOption();

   //---------------------------------------------------------------------------------------------------
   // argument <interface> is required
   //
   clCmdParserP.addPositionalArgument("interface", tr("CAN interface, e.g. can1"));
   //---------------------------------------------------------------------------------------------------
   // command line option: -H <host>
   //
   QCommandLineOption clOptHostT("H", tr("Connect to <host>"), tr("host"));
   clCmdParserP.addOption(clOptHostT);

   //---------------------------------------------------------------------------------------------------
   // command line option: -s | --seg <segment>
   //
   QCommandLineOption clOptSegmentT(QStringList() << "s"
                                                  << "seg",
                                    tr("Route segment <seg>"), tr("segment"));
   clCmdParserP.addOption(clOptSegmentT);

   QCommandLineOption clOptVcanPortT(QStringList() << "p"
                                                   << "port",
                                     tr("UDP <port>"), tr("port"));
   clCmdParserP.addOption(clOptVcanPortT);

   //---------------------------------------------------------------------------------------------------
   // command line option: -v, --version
   //
   clCmdParserP.addVersionOption();

   //---------------------------------------------------------------------------------------------------
   // Process the actual command line arguments given by the user
   //
   clCmdParserP.process(*pclAppP);
   const QStringList clArgsT = clCmdParserP.positionalArguments();
   if (clArgsT.size() != 1)
   {
      fprintf(stderr, "%s\n", qPrintable(tr("Error: Must specify CAN interface.\n")));
      clCmdParserP.showHelp(0);
   }

   //---------------------------------------------------------------------------------------------------
   // test format of argument <interface>
   //
   QString clInterfaceT = clArgsT.at(0);
   if (!clInterfaceT.startsWith("can"))
   {
      fprintf(stderr, "%s %s\n", qPrintable(tr("Error: Unknown CAN interface ")), qPrintable(clInterfaceT));
      clCmdParserP.showHelp(0);
   }

   //---------------------------------------------------------------------------------------------------
   // convert CAN channel to uint8_t value
   //
   QString clIfNumT = clInterfaceT.right(clInterfaceT.size() - 3);
   bool btConversionSuccessT;
   uint8_t ubChannelT = clIfNumT.toInt(&btConversionSuccessT, 10);
   if ((btConversionSuccessT == false) || (ubChannelT == 0))
   {
      fprintf(stderr, "%s \n\n", qPrintable(tr("Error: CAN interface out of range")));
      clCmdParserP.showHelp(0);
   }

   //---------------------------------------------------------------------------------------------------
   // store CAN interface channel (CAN_Channel_e)
   //
   ubChannelP = ubChannelT;

   //---------------------------------------------------------------------------------------------------
   // set host address for socket
   //
   if (clCmdParserP.isSet(clOptHostT))
   {
      QHostAddress clAddressT = QHostAddress(clCmdParserP.value(clOptHostT));
      clCanSocketP.setHostAddress(clAddressT);
   }

   //---------------------------------------------------------------------------------------------------
   // store VCANMC_NET network
   //

   //---------------------------------------------------------------------------------------------------
   // stor base VCANMC_DEST Multicast group or Unicast address
   //

   //---------------------------------------------------------------------------------------------------
   // VCANMC_PORT
   if (clCmdParserP.isSet(clOptVcanPortT))
   {
      //---------------------------------------------------------------------------------------------------
      // Must have VCAN segment to route
      //
      if (clCmdParserP.isSet(clOptSegmentT))
      {
         QString seg = clCmdParserP.value(clOptSegmentT);
         /*
          * VCAN TX socket
          */
         const auto ifaces = QNetworkInterface::allInterfaces();
         bool found = false;
         int ifIndex = 0;
         if (!ifaces.isEmpty())
         {
            for (auto i = 0; i < ifaces.size() && !found; i++)
            {
               if (ifaces[i].IsUp && ifaces[i].CanMulticast)
               {
                  const auto aes = ifaces[i].addressEntries();
                  for (auto j = 0; j < aes.size(); ++j)
                  {
                     qtMulticast.hostAddress = aes[j].ip();
                     QHostAddress mask = aes[j].netmask();
                     QHostAddress net(qtMulticast.hostAddress.toIPv4Address() & mask.toIPv4Address());
                     if (net == QHostAddress("192.32.23.0"))
                     {
#if 1
                        // if (clVcanTxSockettP.bind(hostAddress, 0))
                        if (clVcanTxSockettP.bind(QHostAddress(QHostAddress::AnyIPv4), 0))
                        {
                           qDebug() << "QCanRouteVcan::Bind TX to " << qtMulticast.hostAddress;
                        }
                        else
                        {
                           qDebug() << "Bind Failed" << clVcanTxSockettP.errorString();
                        }
#endif

                        // Save interface index
                        ifIndex = ifaces[i].index();
                        clVcanTxSockettP.setMulticastInterface(ifaces[i]);
                        found = true;
                        break;
                     }
                  }
               }
            }
         }

         /*
          * VCAN RX socket
          */
         clVcanRxSockettP.bind(qtMulticast.hostAddress, VCANMC_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
         QString group = QString("239.0.0.%1").arg(seg);
         qtMulticast.Group.setAddress(group);
         qDebug() << "QCanSocket::joining Multicast group" << qtMulticast.Group;
         clVcanRxSockettP.joinMulticastGroup(qtMulticast.Group);

         // Initialise TX datagram
         qtMulticastDatagram.setDestination(qtMulticast.Group, VCANMC_PORT);
         qtMulticastDatagram.setInterfaceIndex(ifIndex);
         clMulticastLanMessage = {0};
         clMulticastLanMessage.cpCanHdr.segment = seg.toUInt();
         //---------------------------------------------------------------------------------------------------
         // connect to CAN interface
         //
         clCanSocketP.connectNetwork((CAN_Channel_e)ubChannelP);
      }
      else
      {
         fprintf(stderr, "%s \n\n", qPrintable(tr("Error: Need VCAN segment to route")));
         exit(-1);
      }
   }
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanDump::socketReceive()                                                                                          //
// Show messages that are available                                                                                   //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::receiveCANpieDatagrams(void)
{	
	qDebug() << "QCanRouteVcan::receiveCANpieDatagrams()";
   int32_t slFrameCountT = clCanSocketP.framesAvailable();   
	clMulticastLanMessage.cpCanHdr.msgCount = slFrameCountT;
	for (int i=0; i < slFrameCountT; i++)
	{
      QCanFrame clCanFrameT;
		if (clCanSocketP.read(clCanFrameT) == true)
		{
			clCanFrameT.toCpCanMsg(&clMulticastLanMessage.cpCanMsg[i]);			
		}		
	}	
	const int length = VCAN_MSGLEN(clMulticastLanMessage.cpCanHdr.msgCount);
   if (clVcanTxSockettP.writeDatagram(QByteArray::fromRawData((char*)&clMulticastLanMessage, length), qtMulticast.Group, VCANMC_PORT) != length)
   {      
      qDebug() << "QCanRouteVcan::Send failed " << clVcanTxSockettP.errorString();     
   }
   else
   {
      qDebug() << "QCanRouteVcan::Send to " << qtMulticast.Group;
   }
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanDump::sendCANpieServerDatagram()                                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::sendCANpieServerDatagram(QCanFrame& clCanFrameP)
{
   clCanSocketP.write(clCanFrameP);
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanRouteVcan::socketConnected()                                                                                        //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::socketConnected()
{
   //---------------------------------------------------------------------------------------------------
   // initial setup of CAN frame
   //

   connect(&clVcanRxSockettP, SIGNAL(readyRead()), this, SLOT(receiveMulticastDatagrams()));

   connect(&clVcanRxSockettP, &QUdpSocket::readyRead, this, &QCanRouteVcan::receiveMulticastDatagrams);
}

void QCanRouteVcan::receiveMulticastDatagrams()
{
   QByteArray datagram;
   while (clVcanRxSockettP.hasPendingDatagrams())
   {
	   int pendingDatagramSize = int(clVcanRxSockettP.pendingDatagramSize());
      datagram.resize(pendingDatagramSize);
      clVcanRxSockettP.readDatagram(datagram.data(), datagram.size());
      CpCanLanMsg_s* canLanMsg = (CpCanLanMsg_s*)datagram.data();
	  if (VCAN_MSGLEN(canLanMsg->cpCanHdr.msgCount) == pendingDatagramSize)
	  {
		  for (int i = 0; i < canLanMsg->cpCanHdr.msgCount; i++)
		  {
			  clReceiveFrameP.fromCpCanMsg(&canLanMsg->cpCanMsg[i]);
			  ulFrameCountP++;
			  sendCANpieServerDatagram(clReceiveFrameP);
		  }
	  }
   }
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanRouteVcan::socketDisconnected()                                                                                     //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//

void QCanRouteVcan::socketDisconnected()
{
   qDebug() << "Disconnected from CAN " << ubChannelP;
}

//--------------------------------------------------------------------------------------------------------------------//
// QCanRouteVcan::socketError()                                                                                            //
// Show error message and quitApplication                                                                                        //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::socketError(QAbstractSocket::SocketError teSocketErrorV)
{
   Q_UNUSED(teSocketErrorV);  // parameter not used
   //----------------------------------------------------------------
   // show error message in case the connection to the network fails
   //
   fprintf(stderr, "%s %s\n", qPrintable(tr("Failed to connect to CAN interface:")), qPrintable(clCanSocketP.errorString()));
   quitApplication();
}
