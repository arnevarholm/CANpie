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


//--------------------------------------------------------------------------------------------------------------------//
// main()                                                                                                             //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
int main(int argc, char *argv[])
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
   QObject::connect(&clMainT, SIGNAL(finished()),
                    &clAppT, SLOT(quit()));
   
   QObject::connect(&clAppT, SIGNAL(aboutToQuit()),
                    &clMainT, SLOT(aboutToQuitApp()));

   
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
QCanRouteVcan::QCanRouteVcan(QObject *parent) :
    QObject(parent)
{
   //---------------------------------------------------------------------------------------------------
   // get the instance of the main application
   //
   pclAppP = QCoreApplication::instance();

   
   //---------------------------------------------------------------------------------------------------
   // connect signals for socket operations
   //
   QObject::connect(&clCanSocketP, SIGNAL(connected()),
                    this, SLOT(socketConnected()));

   QObject::connect(&clCanSocketP, SIGNAL(disconnected()),
                    this, SLOT(socketDisconnected()));
   
   QObject::connect(&clCanSocketP, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(socketError(QAbstractSocket::SocketError)));
   
   //---------------------------------------------------------------------------------------------------
   // set default values
   //
   ubChannelP     = eCAN_CHANNEL_NONE;
   ulFrameIdP     = 0;
   ulFrameGapP    = 0;
   ubFrameDlcP    = 0;
   ubFrameFormatP = 0;
   btIncIdP       = false;
   btIncDlcP      = false;
   btIncDataP     = false;
   ulFrameCountP  = 0;

}


//--------------------------------------------------------------------------------------------------------------------//
// QCanRouteVcan::aboutToQuitApp()                                                                                         //
// shortly after quit is called the CoreApplication will signal this routine: delete objects / clean up               //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::aboutToQuitApp()
{

}


//--------------------------------------------------------------------------------------------------------------------//
// QCanRouteVcan::quit()                                                                                                   //
// call this routine to quit the application                                                                          //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::quit()
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
   clCmdParserP.addPositionalArgument("interface", 
                                      tr("CAN interface, e.g. can1"));   
   //---------------------------------------------------------------------------------------------------
   // command line option: -H <host>
   //
   QCommandLineOption clOptHostT("H", 
         tr("Connect to <host>"),
         tr("host"));
   clCmdParserP.addOption(clOptHostT);


   //---------------------------------------------------------------------------------------------------
   // command line option: -s | --seg <segment>
   //
   QCommandLineOption clOptSegmentT(QStringList() << "s" << "seg",
	   tr("Route segment <seg>"),
	   tr("segment"));
   clCmdParserP.addOption(clOptSegmentT);

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
      fprintf(stderr, "%s\n", 
              qPrintable(tr("Error: Must specify CAN interface.\n")));
      clCmdParserP.showHelp(0);
   }

   
   //---------------------------------------------------------------------------------------------------
   // test format of argument <interface>
   //
   QString clInterfaceT = clArgsT.at(0);
   if(!clInterfaceT.startsWith("can"))
   {
      fprintf(stderr, "%s %s\n", 
              qPrintable(tr("Error: Unknown CAN interface ")),
              qPrintable(clInterfaceT));
      clCmdParserP.showHelp(0);
   }
   
   //---------------------------------------------------------------------------------------------------
   // convert CAN channel to uint8_t value
   //
   QString clIfNumT = clInterfaceT.right(clInterfaceT.size() - 3);
   bool btConversionSuccessT;
   int32_t slChannelT = clIfNumT.toInt(&btConversionSuccessT, 10);
   if((btConversionSuccessT == false) ||
      (slChannelT == 0) )
   {
      fprintf(stderr, "%s \n\n", 
              qPrintable(tr("Error: CAN interface out of range")));
      clCmdParserP.showHelp(0);
   }
   
   //---------------------------------------------------------------------------------------------------
   // store CAN interface channel (CAN_Channel_e)
   //
   ubChannelP = (uint8_t) (slChannelT);
	
   //---------------------------------------------------------------------------------------------------
   // set host address for socket
   //
   if (clCmdParserP.isSet(clOptHostT))
   {
      QHostAddress clAddressT = QHostAddress(clCmdParserP.value(clOptHostT));
      clCanSocketP.setHostAddress(clAddressT);
   }
   else
   {
      /*
      //-------------------------------------------------------------------------------------------
      // check to local server state and print error if it is not active
      //
      if (pclServerP->state() < QCanServerSettings::eSTATE_ACTIVE)
      {
         fprintf(stdout, "CANpie FD server %s \n", qPrintable(pclServerP->stateString()));
         exit(0);
      }
      */
   }

   if (clCmdParserP.isSet(clOptSegmentT))
   {
	   QString s = clCmdParserP.value(clOptSegmentT);
	   uint32_t seg = s.toUInt();

   	
   	
	   
   }
   else
   {
	   fprintf(stderr, "%s \n\n",
		   qPrintable(tr("Error: Need VCAN segment to route")));
	   exit(-1);

	   
   }
   

	

   //---------------------------------------------------------------------------------------------------
   // connect to CAN interface
   //
   clCanSocketP.connectNetwork((CAN_Channel_e) ubChannelP);

}


//--------------------------------------------------------------------------------------------------------------------//
// QCanDump::sendFrame()                                                                                              //
//                                                                                                                    //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::sendFrame(void)
{
   QTime          clSystemTimeT;
   QCanTimeStamp  clCanTimeT;
   
   clSystemTimeT = QTime::currentTime();
   clCanTimeT.fromMilliSeconds(clSystemTimeT.msec());
   clCanFrameP.setTimeStamp(clCanTimeT);
   
   clCanSocketP.write(clCanFrameP);
   
   if (ulFrameCountP > 1)
   {
      ulFrameCountP--;
      
      //-------------------------------------------------------------------------------------------
      // test if identifier value must be incremented
      //
      if (btIncIdP)
      {
         ulFrameIdP++;
         
         //-----------------------------------------------------------------------------------
         // test for wrap-around
         //
         if (clCanFrameP.isExtended())
         {
            if (ulFrameIdP > QCAN_FRAME_ID_MASK_EXT)
            {
               ulFrameIdP = 0;
            }
         }
         else
         {
            if (ulFrameIdP > QCAN_FRAME_ID_MASK_STD)
            {
               ulFrameIdP = 0;
            }
         }
         
         //-----------------------------------------------------------------------------------
         // set new identifier value
         //
         clCanFrameP.setIdentifier(ulFrameIdP);
      }
      
      //-------------------------------------------------------------------------------------------
      // test if DLC value must be incremented
      //
      if (btIncDlcP)
      {
         ubFrameDlcP++;
         
         //-----------------------------------------------------------------------------------
         // test for wrap-around
         //
         if (clCanFrameP.frameFormat() > QCanFrame::eFORMAT_CAN_EXT)
         {
            if (ubFrameDlcP > 15)
            {
               ubFrameDlcP = 0;
            }
         }
         else
         {
            if (ubFrameDlcP > 8)
            {
               ubFrameDlcP = 0;
            }
         }

         //-----------------------------------------------------------------------------------
         // set new DLC value
         //
         clCanFrameP.setDlc(ubFrameDlcP);
      }  
      
      //-------------------------------------------------------------------------------------------
      // test if data value must be incremented
      //
      if (btIncDataP)
      {

         if (clCanFrameP.dataSize() == 1)
         {
            clCanFrameP.setData(0, clCanFrameP.data(0) + 1);
         }

         if (clCanFrameP.dataSize() == 2)
         {
            clCanFrameP.setDataUInt16(0, clCanFrameP.dataUInt16(0) + 1);
         }

         if (clCanFrameP.dataSize() >= 4)
         {
            clCanFrameP.setDataUInt32(0, clCanFrameP.dataUInt32(0) + 1);
         }
      }
      
      QTimer::singleShot(ulFrameGapP, this, SLOT(sendFrame()));
   }
   else
   {
      QTimer::singleShot(50, this, SLOT(quit()));
   }

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
   clCanFrameP.setFrameFormat((QCanFrame::FrameFormat_e) ubFrameFormatP);
   clCanFrameP.setIdentifier(ulFrameIdP);
   clCanFrameP.setDlc(ubFrameDlcP);
   for(uint8_t ubCntT = 0; ubCntT < clCanFrameP.dataSize(); ubCntT++)
   {
      clCanFrameP.setData(ubCntT, aubFrameDataP[ubCntT]);
   }
    
   QTimer::singleShot(10, this, SLOT(sendFrame()));
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
// Show error message and quit                                                                                        //
//--------------------------------------------------------------------------------------------------------------------//
void QCanRouteVcan::socketError(QAbstractSocket::SocketError teSocketErrorV)
{
   Q_UNUSED(teSocketErrorV);  // parameter not used 
   
   //----------------------------------------------------------------
   // show error message in case the connection to the network fails
   //
   fprintf(stderr, "%s %s\n", 
           qPrintable(tr("Failed to connect to CAN interface:")),
           qPrintable(clCanSocketP.errorString()));
   quit();
}
