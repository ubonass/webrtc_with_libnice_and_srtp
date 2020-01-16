/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       srtp_interface.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#ifndef SRTP_INTERFACE_H
#define SRTP_INTERFACE_H

typedef enum SrtpSsrcType
{
    SRTP_SSRC_UNDEFINED = 0,   /**< Indicates an undefined SSRC type.    */
    SRTP_SSRC_NEED_VALUE = 1,    /**< Indicates a specific SSRC value      */
    SRTP_SSRC_UNPROTECT = 2, /**< Indicates any inbound SSRC value     */
	                          /**< (i.e. a value that is used in the    */
	                          /**< function srtp_unprotect())           */
    SRTP_SSRC_PROTECT = 3 /**< Indicates any outbound SSRC value    */
	                          /**< (i.e. a value that is used in the    */
	                          /**< function srtp_protect())             */
} E_SrtpSsrcType;






/*****************************************************************************
-Fuction        : SrtpInit
-Description    : v
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int SrtpInit();


/*****************************************************************************
-Fuction        : SrtpCreate
-Description    : SrtpCreate
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int SrtpCreate(char *i_acKey,int i_iKeyLen,E_SrtpSsrcType i_eSrtpSsrcType);


/*****************************************************************************
-Fuction        : SrtpProtectRtp
-Description    : SrtpProtectRtp
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int SrtpProtectRtp(char * i_acRtpData,int i_iDataLen);

/*****************************************************************************
-Fuction        : SrtpShutdown
-Description    : SrtpShutdown
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int SrtpShutdown();




#endif
