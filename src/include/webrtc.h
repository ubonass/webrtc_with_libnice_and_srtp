/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc.h
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef WEBRTC_H
#define WEBRTC_H

#include "libnice_interface.h"
#include "srtp_interface.h"
#include "dtls_only_handshake.h"
#include "sctp_interface.h"

typedef struct VideoInfo
{
    const char *pstrFormatName;
    unsigned int dwTimestampFrequency;
    //9代表视频使用端口9来传输,但在webrtc中现在一般不使用
	//如果设置为0，代表不传输视频
    unsigned short wPortNumForSDP;//端口,官方demo都是 9
    unsigned char ucRtpPayloadType;
    unsigned char res;
	int iID;
	unsigned int dwProfileLevelId;
	char * strSPS_Base64;
	char * strPPS_Base64;
}T_VideoInfo;

/*****************************************************************************
-Class          : WebRTC
-Description    : WebRTC
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class WebRTC
{
public:
	WebRTC(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling);//:m_Libnice(i_strStunAddr,i_dwStunPort,i_iControlling),m_Srtp()
	virtual ~WebRTC();
    int Proc();
    virtual int HandleMsg(char * i_strMsg,int i_iNotJsonMsgFlag=0)=0;
    int HandleCandidateMsg(char * i_strCandidateMsg,int i_iNotJsonMsgFlag=0);
    int GetSendReadyFlag();
    int SendProtectedRtp(char * i_acRtpBuf,int i_iRtpBufLen);
    virtual int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strMsg,int i_iMaxLen)=0;
    int GenerateLocalCandidateMsg(T_VideoInfo *i_ptVideoInfo,char * o_strCandidateMsg,int i_iCandidateMaxLen);
    virtual int GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen);//webrtc_client使用
    
    static void HandshakeCb(void * pArg);//放到上层的目的是为了底层模块之间不要相互依赖
    static void HandleRecvDataCb(char * i_acData,int i_iLen,void * pArg);//后续可以改为private试试
	static int SendVideoDataOutCb(char * i_acData,int i_iLen,void * pArg);
protected:
    static bool IsDtls(char *buf);
    
    Libnice m_Libnice;
	Srtp m_Srtp;
    DtlsOnlyHandshake * m_pDtlsOnlyHandshake;//只在视频通道这一个通道内协商
    Sctp * m_pSctp;//应用数据和视频数据共用一个通道传输
    
    T_DtlsOnlyHandshakeCb m_tDtlsOnlyHandshakeCb;
    T_LibniceCb m_tLibniceCb;
    T_SctpCb m_tSctpCb;
	int m_iSrtpCreatedFlag;//0未创建,1已经创建
	int m_iSendReadyFlag;//0 no ready,1 ready
    
};

/*****************************************************************************
-Class          : WebRtcOffer
-Description    : WebRtcOffer
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class WebRtcOffer : public WebRTC
{
public:
	WebRtcOffer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling);
	virtual ~WebRtcOffer();
    int HandleMsg(char * i_strAnswerMsg,int i_iNotJsonMsgFlag=0);
    int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strOfferMsg,int i_iOfferMaxLen);
    int GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen);

};


/*****************************************************************************
-Class          : WebRtcAnswer
-Description    : WebRtcAnswer
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/11      V1.0.0              Yu Weifeng       Created
******************************************************************************/
class WebRtcAnswer : public WebRTC
{
public:
	WebRtcAnswer(char * i_strStunAddr,unsigned int i_dwStunPort,E_IceControlRole i_eControlling);
	virtual ~WebRtcAnswer();
	
    int HandleMsg(char * i_strOfferMsg,int i_iNotJsonMsgFlag=0);
    int GenerateLocalMsg(T_VideoInfo *i_ptVideoInfo,char * o_strAnswerMsg,int i_iAnswerMaxLen);

    int GenerateLocalSDP(T_VideoInfo *i_ptVideoInfo,char *o_strSDP,int i_iSdpMaxLen);
};













#endif
