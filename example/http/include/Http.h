/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       webrtc.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef HTTP_H
#define HTTP_H

#include "TcpSocket.h"


#define HTTP_PACKET_MAX_LEN 2048

#define HTTP_METHOD_GET "GET"
#define HTTP_METHOD_PUT "PUT"
#define HTTP_METHOD_POST "POST"

#define HTTP_VERSION "HTTP/1.0"

#define HTTP_CONTENT_TYPE_TEXT     "text/plain"
/*****************************************************************************
-Class			: HttpClient
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class HttpClient
{
public:
	HttpClient();
	~HttpClient();
	int Init(char * i_strIP,unsigned short i_wPort);
	int Send(const char * i_srtMethod,char * i_srtURL,char * i_acSendBuf,int i_iSendLen,const char * i_srtContentType=NULL);
	int RecvBody(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen);
	int Recv(char *o_acRecvBuf,int *o_piRecvLen,int i_iRecvBufMaxLen);
private:
	TcpClient *m_pTcpClient;
	string m_strServerIp;
	int m_iServerPort;
};





/*****************************************************************************
-Class			: HttpServer
-Description	: 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2019/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
class HttpServer: public TcpServer
{
public:



private:


};









#endif
