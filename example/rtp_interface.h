/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       rtp_interface.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef RTP_INTERFACE_H
#define RTP_INTERFACE_H

#include "Rtp.h"

/*****************************************************************************
-Class			: RtpInterface
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class RtpInterface
{
public:
	RtpInterface(char *i_strPath);
	~RtpInterface();
    int GetSPS_PPS(unsigned char *o_pbSpsBuf,int *o_piSpsBufLen,unsigned char *o_pbPpsBuf,int *o_piPpsBufLen);
    int RemoveH264EmulationBytes(unsigned char *o_pbNaluBuf,int i_iMaxNaluBufLen,unsigned char *i_pbNaluBuf,int i_iNaluLen);
    int GetRtpData(unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iEveryRtpBufMaxSize,int i_iPacketBufMaxNum);


private:
	Rtp               *m_pRtp;

    
};


#endif
