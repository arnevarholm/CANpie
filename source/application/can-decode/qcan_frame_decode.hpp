//====================================================================================================================//
// File:          qcan_frame_decode.hpp                                                                                      //
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


#ifndef QCAN_FRAME_DECODE_HPP_
#define QCAN_FRAME_DECODE_HPP_


/*--------------------------------------------------------------------------------------------------------------------*\
** Include files                                                                                                      **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/
#include "qcan_frame.hpp"

using namespace QCan;


/*--------------------------------------------------------------------------------------------------------------------*\
** Definitions                                                                                                        **
**                                                                                                                    **
\*--------------------------------------------------------------------------------------------------------------------*/


class QCanFrameDecode :public QCanFrame
{
public:
	//---------------------------------------------------------------------------------------------------
	/*!
	** Constructs an empty classic standard CAN frame (frame format QCanFrame::eFORMAT_CAN_STD) with a
	** DLC value of 0.
	*/
	QCanFrameDecode() : QCanFrame() {};


   //---------------------------------------------------------------------------------------------------
   /*!
   ** \param[in]  ubTypeR  Frame type
   **
   ** Constructs an empty data structure with a frame type of QCanFrame::FrameType_e.
   */
	QCanFrameDecode(const FrameType_e & ubTypeR) : QCanFrame(ubTypeR) {};


   //---------------------------------------------------------------------------------------------------
   /*!
   ** \param[in]  ubFormatR      CAN frame format
   ** \param[in]  ulIdentifierR  CAN frame identifier
   ** \param[in]  ubDlcR         CAN frame data length code
   **
   ** Constructs a CAN frame of type \c ubTypeR with an identifier value of \c ulIdentifierR and a DLC
   ** value of \c ubDlcR.
   */
	QCanFrameDecode(const FrameFormat_e & ubFormatR, const uint32_t & ulIdentifierR = 0, const uint8_t & ubDlcR = 0) : QCanFrame(ubFormatR, ulIdentifierR, ubDlcR) {};
	virtual ~QCanFrameDecode() {};
   
   virtual QString    toString(const bool & btShowTimeR = false, const bool & btShowDataSizeR = false) const;

private:
	QString decode_can_std() const;
};





#endif
