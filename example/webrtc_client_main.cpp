/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       main.cpp
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <sys/prctl.h>

#include "webrtc.h"
#include "webrtc_client.h"
#include "rtp_interface.h"
#include "Base64.h"

#define WEBRTC_RTP_MAX_PACKET_NUM	(300)
#define WEBRTC_RTP_MAX_PACKET_SIZE	((1500-42)/4*4)//MTU

#define WEBRTC_RTP_PAYLOAD_H264 102 //webrtc中是102
#define WEBRTC_VIDEO_ENCODE_FORMAT_NAME "H264"
#define WEBRTC_H264_TIMESTAMP_FREQUENCY 90000

typedef enum WebRtcOfferStatus
{
    WEBRTC_OFFER_INIT,
    WEBRTC_OFFER_SEND_SDP,
    WEBRTC_OFFER_HANDLE_SDP,
    WEBRTC_OFFER_HANDLE_CANDIDATE,
    WEBRTC_OFFER_SEND_READY,//等待通道建立
    WEBRTC_OFFER_SEND_RTP,
    WEBRTC_OFFER_EXIT,
}E_WebRtcOfferStatus;


static void PrintUsage(char *i_strProcName);
static void * WebRtcProc(void *pArg);
static int OfferProc(WebRTC * i_pWebRTC,char * i_strServerIp, int i_iServerPort, char * i_strSelfName,char *i_strVideoPath);

/*****************************************************************************
-Fuction        : main
-Description    : main
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int main(int argc, char* argv[]) 
{
    char strSeverIp[20];
    int dwPort;
    WebRTC * pWebRTC=NULL;
    pthread_t tWebRtcID;

    int iRet = -1;
    
    if(argc !=6)
    {
        PrintUsage(argv[0]);
        return -1;
    }
    memset(strSeverIp,0,sizeof(strSeverIp));
    strncpy(strSeverIp,argv[1],sizeof(strSeverIp));
    dwPort=atoi(argv[2]);
    
    //简单点，又因为没有ui，所以offer 和answer不能共存
    if(0 == strcmp(argv[5],"offer"))
    {
        pWebRTC = new WebRtcOffer(strSeverIp,dwPort,ICE_CONTROLLING_ROLE);
    }
    else
    {
        PrintUsage(argv[0]);
        return -1;
    }
    if( pthread_create(&tWebRtcID,NULL,WebRtcProc,(void *)pWebRTC) != 0 )
    {
        printf("WebRtcProc pthread_create err\r\n");
    }
    else
    {
        if(0 == strcmp(argv[5],"offer"))
        {
            iRet =OfferProc(pWebRTC,strSeverIp,dwPort,argv[3],argv[4]);
        }
        else
        {
        }
    }
    delete pWebRTC;
    return iRet;
}
/*****************************************************************************
-Fuction        : PrintUsage
-Description    : PrintUsage
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static void PrintUsage(char *i_strProcName)
{
    printf("Usage: %s StunIP StunPort SelfName VideoFile offer/answer\r\n",i_strProcName);
    printf("eg: %s 192.168.7.199 9898 ywf sintel.h264 offer\r\n",i_strProcName);
}

/*****************************************************************************
-Fuction        : WebRtcProc
-Description    : WebRtcProc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static void * WebRtcProc(void *pArg)
{
    //设置线程名字,以便让系统知道,
    prctl(15,(unsigned long)"WebRtcProc");
    WebRTC * pWebRTC=NULL;
    
    if(NULL != pArg)
    {
        pWebRTC = (WebRTC *)pArg;
        pWebRTC->Proc();
    }
    return NULL;
}

/*****************************************************************************
-Fuction        : AnswerProc
-Description    : SendAnswerProc
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/01      V1.0.0              Yu Weifeng       Created
******************************************************************************/
static int OfferProc(WebRTC * i_pWebRTC,char * i_strServerIp, int i_iServerPort, char * i_strSelfName,char *i_strVideoPath)
{
    T_VideoInfo tVideoInfo;
    char acGetMsg[6*1024];
    RtpInterface *pRtpInterface=NULL;
    WebRtcClientOffer *pWebRtcClientOffer=NULL;
    int iRecvLen=0;
    int iRet=0;

    pWebRtcClientOffer = new WebRtcClientOffer();
    pRtpInterface = new RtpInterface(i_strVideoPath);
    int iPacketNum=0;
    unsigned char *ppbPacketBuf[WEBRTC_RTP_MAX_PACKET_NUM]={0};
    int aiEveryPacketLen[WEBRTC_RTP_MAX_PACKET_NUM]={0};
    int i=0;
    for(i=0;i<WEBRTC_RTP_MAX_PACKET_NUM;i++)
    {
        ppbPacketBuf[i]=(unsigned char *)malloc(WEBRTC_RTP_MAX_PACKET_SIZE);//
        memset(ppbPacketBuf[i],0,WEBRTC_RTP_MAX_PACKET_SIZE);
    }
    E_WebRtcOfferStatus eWebRtcStatus=WEBRTC_OFFER_INIT;
    while(1)
    {
        switch(eWebRtcStatus)
        {
            case WEBRTC_OFFER_INIT:
            {
                iPacketNum = pRtpInterface->GetRtpData(ppbPacketBuf,aiEveryPacketLen,WEBRTC_RTP_MAX_PACKET_SIZE,WEBRTC_RTP_MAX_PACKET_NUM);//第一次会失败
                iRet = pWebRtcClientOffer->Login(i_strServerIp,i_iServerPort);
                if(iRet >= 0)
                {
                    eWebRtcStatus=WEBRTC_OFFER_SEND_SDP;
                }
                else
                {
                    printf("pWebRtcClientOffer->Login err\r\n");
                }
                break;
            }
            case WEBRTC_OFFER_SEND_SDP:
            {
                memset(&tVideoInfo,0,sizeof(tVideoInfo));
                tVideoInfo.pstrFormatName=WEBRTC_VIDEO_ENCODE_FORMAT_NAME;
                tVideoInfo.dwTimestampFrequency=WEBRTC_H264_TIMESTAMP_FREQUENCY;  
                tVideoInfo.ucRtpPayloadType=WEBRTC_RTP_PAYLOAD_H264;
                tVideoInfo.wPortNumForSDP=9;
                tVideoInfo.iID=0;
                unsigned char           abSPS[64]={0};
                unsigned char           abPPS[64]={0};
                int                     iSPS_Len=0;
                int                     iPPS_Len=0;
                pRtpInterface->GetSPS_PPS(abSPS, &iSPS_Len, abPPS, &iPPS_Len);
                unsigned char abSPS_WEB[SPS_PPS_BUF_MAX_LEN]={0};// "WEB" means "Without Emulation Bytes"
                int iSPS_WEB_Len= pRtpInterface->RemoveH264EmulationBytes(abSPS_WEB, iSPS_Len, abSPS, iSPS_Len);
                if (iSPS_WEB_Len < 4) 
                { // Bad SPS size => assume our source isn't ready
                    printf("Bad SPS size:%d \r\n",iSPS_WEB_Len);
                }
                unsigned int dwProfileLevelId = (abSPS_WEB[1]<<16) | (abSPS_WEB[2]<<8) | abSPS_WEB[3];
                char * strSPS_Base64 = base64Encode((char*)abSPS, iSPS_Len);//可以考虑放到代码内部
                char * strPPS_Base64 = base64Encode((char*)abPPS, iPPS_Len);//但会多依赖base64
                tVideoInfo.dwProfileLevelId = dwProfileLevelId;
                tVideoInfo.strSPS_Base64 = strSPS_Base64;
                tVideoInfo.strPPS_Base64= strPPS_Base64;
                memset(acGetMsg,0,sizeof(acGetMsg));
                if(i_pWebRTC->GenerateLocalSDP(&tVideoInfo,acGetMsg,sizeof(acGetMsg))>=0)
                {
                    pWebRtcClientOffer->PostSdpMsg(acGetMsg);
                    eWebRtcStatus=WEBRTC_OFFER_HANDLE_SDP;
                    delete[] strSPS_Base64;
                    delete[] strPPS_Base64;
                }
                else
                {
                    printf("pSignalingInf->GenerateLocalSDP err\r\n");
                    delete[] strSPS_Base64;
                    delete[] strPPS_Base64;
                    sleep(2);
                }
                break;
            }
            
            case WEBRTC_OFFER_HANDLE_SDP:
            {
                if(0==pWebRtcClientOffer->GetMsg())
                {
                    memset(acGetMsg,0,sizeof(acGetMsg));
                    pWebRtcClientOffer->GetSdpMsg(acGetMsg,&iRecvLen,sizeof(acGetMsg));
                    if(0==i_pWebRTC->HandleMsg(acGetMsg,1))
                    {
                        eWebRtcStatus=WEBRTC_OFFER_HANDLE_CANDIDATE;
                    }
                    else
                    {
                        printf("pSignalingInf->HandleMsg err\r\n");
                    }
                }
                else
                {
                    printf("pWebRtcClientOffer->GetMsg GetMsg err\r\n");
                    sleep(1);
                }
                break;
            }
            
            case WEBRTC_OFFER_HANDLE_CANDIDATE:
            {
                memset(acGetMsg,0,sizeof(acGetMsg));
                pWebRtcClientOffer->GetCandidateMsg(acGetMsg,&iRecvLen,sizeof(acGetMsg));
                if(0==i_pWebRTC->HandleCandidateMsg(acGetMsg,1))
                {
                    eWebRtcStatus=WEBRTC_OFFER_SEND_READY;
                }
                else
                {
                    printf("i_pWebRTC->HandleCandidateMsg err\r\n");
                    sleep(2);
                }
                break;
            }
            case WEBRTC_OFFER_SEND_READY:
            {
                if(i_pWebRTC->GetSendReadyFlag() == 0)
                {
                    eWebRtcStatus=WEBRTC_OFFER_SEND_RTP;
                    printf("WEBRTC_SEND_RTP......\r\n");
                }
                else
                {
                    printf("WEBRTC_SEND_READY......\r\n");
                    sleep(1);
                }
                break;
            }
            
            case WEBRTC_OFFER_SEND_RTP:
            {
                if(iPacketNum > 0)
                {
                    for(i=0;i<iPacketNum;i++)
                    {
                        i_pWebRTC->SendProtectedRtp((char *)ppbPacketBuf[i], aiEveryPacketLen[i]);
                    }
                    iPacketNum = -1;
                }
                else
                {
                    printf("pRtpInterface->GetRtpData err\r\n");
                    eWebRtcStatus=WEBRTC_OFFER_SEND_RTP;
                }
                iPacketNum = pRtpInterface->GetRtpData(ppbPacketBuf,aiEveryPacketLen,WEBRTC_RTP_MAX_PACKET_SIZE,WEBRTC_RTP_MAX_PACKET_NUM);
                break;
            }
            
            case WEBRTC_OFFER_EXIT:
            {
                printf("#####################WEBRTC_EXIT:%d\r\n",eWebRtcStatus);
                break;
            }
            
            default:
            {
                printf("unkown eWebRtcStatus:%d\r\n",eWebRtcStatus);
                break;
            }
            usleep(40*1000);//25帧率
        }
        if(WEBRTC_OFFER_EXIT == eWebRtcStatus)
        {
            break;
        }
    }
    for(i=0;i<WEBRTC_RTP_MAX_PACKET_NUM;i++)
        free(ppbPacketBuf[i]);

    return iRet;
}

