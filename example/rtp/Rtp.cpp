/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       Rtp.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "Rtp.h"
#include <stdlib.h>//还是需要.h
#include <stdio.h>
#include <string.h>
#include <iostream>//不加.h,c++新的头文件
#include "Definition.h"


using std::cout;//需要<iostream>
using std::endl;

/*****************************************************************************
-Fuction		: Rtp
-Description	: Rtp
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Rtp :: Rtp()
{
    m_pVideoHandle = NULL;
    m_pRtpPacket = NULL;
    m_pRtpPacket = new RtpPacket(RTP_PACKET_H264);

}

/*****************************************************************************
-Fuction		: ~Rtp
-Description	: ~Rtp
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
Rtp :: ~Rtp()
{
    if(NULL !=m_pRtpPacket)
    {

        delete m_pRtpPacket;
    }

    if(NULL !=m_pVideoHandle)
    {

        delete m_pVideoHandle;
    }
}


/*****************************************************************************
-Fuction		: RtpInterface::Init
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::Init(char *i_strPath)
{
    int iRet=FALSE;
    if(NULL == i_strPath)
    {
        cout<<"Init NULL"<<endl;
    }
    else
    {
        m_pVideoHandle=new VideoHandle();
        if(NULL !=m_pVideoHandle)
            iRet=m_pVideoHandle->Init(i_strPath);
    }
    
	return iRet;
}

/*****************************************************************************
-Fuction		: RtpInterface
-Description	: RtpInterface
-Input			: 
-Output 		: 
-Return 		: iPacketNum -1 err,其他表示rtp包个数
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/10/10	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp :: GetRtpData(unsigned char **o_ppbPacketBuf,int *o_aiEveryPacketLen,int i_iEveryRtpBufMaxSize,int i_iPacketBufMaxNum)
{
    int iPacketNum = -1;

    unsigned char *pbNaluStartPos=NULL;
    int iNaluLen=0;
    unsigned char bNaluType=0;
    if(NULL == o_ppbPacketBuf ||NULL == o_aiEveryPacketLen ||NULL == m_pVideoHandle )
    {
        cout<<"GetRtpData NULL"<<endl;
        return iPacketNum;
    }
    
    unsigned char * pbVideoBuf=(unsigned char * )malloc(VIDEO_BUFFER_MAX_SIZE);
    int iVideoBufLen=0;
    memset(pbVideoBuf,0,VIDEO_BUFFER_MAX_SIZE);
    iPacketNum=m_pVideoHandle->GetNextVideoFrame(pbVideoBuf,&iVideoBufLen,VIDEO_BUFFER_MAX_SIZE);
    if(FALSE == iPacketNum)
    {
    }
    else
    {
        iPacketNum=m_pVideoHandle->FindH264Nalu(pbVideoBuf, iVideoBufLen,&pbNaluStartPos, &iNaluLen,&bNaluType);
        if(FALSE== iPacketNum || NULL == pbNaluStartPos ||0==iNaluLen)
        {
            cout<<"FindH264Nalu err:"<<iPacketNum<<" iNaluLen:"<<iNaluLen<<" iVideoBufLen:"<<iVideoBufLen<<endl;
        }
        else
        {
            m_pVideoHandle->TrySetSPS_PPS(pbNaluStartPos,iNaluLen);
            iPacketNum=m_pRtpPacket->Packet(pbNaluStartPos,iNaluLen,o_ppbPacketBuf,o_aiEveryPacketLen);
            if(iPacketNum<=0 || iPacketNum>i_iPacketBufMaxNum)
            {
                cout<<"m_pRtpPacket->Packet err"<<iPacketNum<<endl;
                iPacketNum =-1;
            }
            else
            {
            
            }
        }

    }
    free(pbVideoBuf);

        
    return iPacketNum;
}

/*****************************************************************************
-Fuction		: VideoHandle::RemoveH264EmulationBytes
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::RemoveH264EmulationBytes(unsigned char *o_pbNaluBuf,int i_iMaxNaluBufLen,unsigned char *i_pbNaluBuf,int i_iNaluLen)
{
    int iNaluLen=0;
    if(NULL !=m_pVideoHandle)
    {
        iNaluLen=m_pVideoHandle->RemoveH264EmulationBytes(o_pbNaluBuf,i_iMaxNaluBufLen,i_pbNaluBuf,i_iNaluLen);
    }
	return iNaluLen;
}

/*****************************************************************************
-Fuction		: VideoHandle::GetSPS_PPS
-Description	: 
-Input			: 
-Output 		: 
-Return 		: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int Rtp::GetSPS_PPS(unsigned char *o_pbSpsBuf,int *o_piSpsBufLen,unsigned char *o_pbPpsBuf,int *o_piPpsBufLen)
{
    int iRet=FALSE;
    if(NULL !=m_pVideoHandle)
    {
        iRet=m_pVideoHandle->GetSPS_PPS(o_pbSpsBuf,o_piSpsBufLen,o_pbPpsBuf,o_piPpsBufLen);
    }
	return iRet;
}

