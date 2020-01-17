/*****************************************************************************
* Copyright (C) 2020-2025 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module           :       libnice_interface.c
* Description           : 	
* Created               :       2020.01.13.
* Author                :       Yu Weifeng
* Function List         : 	
* Last Modified         : 	
* History               : 	
******************************************************************************/
#include "libnice_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <agent.h>

#include <gio/gnetworking.h>

static void LibniceCandidateGatheringDone(NiceAgent *i_ptAgent, guint i_dwStreamID,gpointer pData);
static void LibniceNewSelectPair(NiceAgent *agent, guint _stream_id,guint component_id, gchar *lfoundation,gchar *rfoundation, gpointer data);
static void LibniceComponentStateChanged(NiceAgent *agent, guint _stream_id,guint component_id, guint state,gpointer data);
static void LibniceRecv(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data);

static T_LocalCandidate g_tLocalCandidate;
static const char *g_astrCandidateTypeName[] = {"host", "srflx", "prflx", "relay"};
static NiceAgent *g_ptAgent=NULL;
static GSList *g_pRemoteCandidatesList = NULL;
static unsigned int g_dwStreamID=0;//
static int g_iLibniceSendReadyFlag =0;//0不可发送,1准备好通道可以发送
static T_LibniceDepData g_tLibniceDepData = {0};


/*****************************************************************************
-Fuction        : LibniceInit
-Description    : LibniceInit
-Input          : i_iControlling 感觉不必要
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceInit(char * i_strStunAddr,unsigned int i_dwStunPort,int i_iControlling)
{
	int iRet = -1;
    if i_strStunAddr == NULL) 
    {
		printf("LibniceInit i_strStunAddr null \r\n");
		return iRet;
    }
	memset(&g_tLibniceDepData,0,sizeof(T_LibniceDepData));
	snprintf(g_tLibniceDepData.strStunAddr,sizeof(g_tLibniceDepData.strStunAddr),"%s",i_strStunAddr);
	g_tLibniceDepData.dwStunPort = i_dwStunPort;
	g_tLibniceDepData.iControlling = i_iControlling;

	iRet = 0;
	return iRet;
}


/*****************************************************************************
-Fuction        : LibniceProc
-Description    : 会阻塞,线程函数
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceProc()
{
	int iRet = -1;
    static GMainLoop *ptLoop=NULL;//
    static GIOChannel* ptStdinIO=NULL;//

    g_iLibniceSendReadyFlag = 0;
    memset(&g_tLocalCandidate,0,sizeof(T_LocalCandidate));
	g_networking_init();//
	ptLoop = g_main_loop_new(NULL, FALSE);//
	ptStdinIO = g_io_channel_unix_new(fileno(stdin));//

	// Create the nice agent
	g_ptAgent = nice_agent_new(g_main_loop_get_context (ptLoop),NICE_COMPATIBILITY_RFC5245);
	if (g_ptAgent == NULL)
		g_error("Failed to create agent");

	// Set the STUN settings and controlling mode
	if (strlen(g_tLibniceDepData.strStunAddr)>0) 
	{
		g_object_set(g_ptAgent, "stun-server", g_tLibniceDepData.strStunAddr, NULL);
		g_object_set(g_ptAgent, "stun-server-port", g_tLibniceDepData.dwStunPort, NULL);
	}
	g_object_set(g_ptAgent, "controlling-mode", g_tLibniceDepData.iControlling, NULL);

	// Connect to the signals
	g_signal_connect(g_ptAgent, "candidate-gathering-done",G_CALLBACK(LibniceCandidateGatheringDone), NULL);
	g_signal_connect(g_ptAgent, "new-selected-pair",G_CALLBACK(LibniceNewSelectPair), NULL);//
	g_signal_connect(g_ptAgent, "component-state-changed",G_CALLBACK(LibniceComponentStateChanged), NULL);//

	// Create a new stream with one component
	g_dwStreamID = nice_agent_add_stream(g_ptAgent, 1);//
	if (g_dwStreamID == 0)
		g_error("Failed to add stream");

	// Attach to the component to receive the data
	// Without this call, candidates cannot be gathered
	nice_agent_attach_recv(g_ptAgent, g_dwStreamID, 1,g_main_loop_get_context (ptLoop), LibniceRecv, NULL);//

	// Start gathering local candidates
	if (!nice_agent_gather_candidates(g_ptAgent, g_dwStreamID))
		g_error("Failed to start candidate gathering");

	g_debug("waiting for candidate-gathering-done signal...");

	// Run the mainloop. Everything else will happen asynchronously
	// when the candidates are done gathering.
	g_main_loop_run (ptLoop);////g_main_loop_quit (gloop);//
    

	g_main_loop_unref(ptLoop);//
	g_object_unref(g_ptAgent);//
	g_io_channel_unref (ptStdinIO);//
}
/*****************************************************************************
-Fuction        : LibniceGetLocalCandidate
-Description    : LibniceGetLocalCandidate
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceGetLocalCandidate(T_LocalCandidate * i_ptLocalCandidate)
{
	int iRet=-1;
	while(g_tLocalCandidate.iGatheringDoneFlag == 0)
	{

	}
	memcpy(i_ptLocalCandidate,&g_tLocalCandidate,sizeof(T_LocalCandidate));
	return 0;
}
/*****************************************************************************
-Fuction        : LibniceGetLocalSDP
-Description    : LibniceGetLocalSDP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceGetLocalSDP(char * i_strSDP,int i_iSdpLen)
{
	int iRet=-1;
	char *strSDP = NULL;
    if (i_strSDP == NULL || g_ptAgent == NULL || i_iSdpLen <= 0) 
    {
		printf("LibniceGetLocalCandidate NULL\r\n");
		return iRet;
    }
	while(g_tLocalCandidate.iGatheringDoneFlag == 0)
	{

	}
    // Candidate gathering is done. Send our local candidates on stdout
    strSDP = nice_agent_generate_local_sdp (g_ptAgent);
    if(NULL!=strSDP)
    {
		snprintf(i_strSDP,i_iSdpLen,"%s",strSDP);
	    g_free (strSDP);
	    iRet = 0;
    }
	return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSetRemoteCredentials
-Description    : LibniceSetRemoteCredentials
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceSetRemoteCredentials(char * i_strUfrag,char * i_strPasswd)
{
	int iRet = -1;
    if (i_strUfrag == NULL || i_strPasswd == NULL || g_ptAgent == NULL || g_dwStreamID == 0) 
    {
		g_message("line must have at least ufrag, password, and one candidate");
		return iRet;
    }
    
    if (!nice_agent_set_remote_credentials(g_ptAgent, g_dwStreamID, i_strUfrag, i_strPasswd)) 
    {
		g_message("failed to set remote credentials");
		return iRet;
    }
    iRet = 0;
    return iRet;
}
/*****************************************************************************
-Fuction        : LibniceSetRemoteCandidateToGlist
-Description    : LibniceSetRemoteCandidateToGlist
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceSetRemoteCandidateToGlist(char * i_strCandidate)
{
	int iRet = -1;
    NiceCandidate *ptCand = NULL;
    NiceCandidateType ntype;
    char **tokens = NULL;
    unsigned int i=0;
	if(NULL == i_strCandidate || 0==g_dwStreamID)
	{
        g_message("line must have at least ufrag, password, and one candidate");
        return iRet;
	}
	// Remaining args are serialized canidates (at least one is required)
    tokens = g_strsplit (i_strCandidate, " ", 5);
    for (i = 0; tokens[i]; i++);
	if (i != 5)
	{
        g_strfreev(tokens);
        return iRet;
	}
	for (i = 0; i < G_N_ELEMENTS (g_astrCandidateTypeName); i++) 
	{
		if (strcmp(tokens[4], g_astrCandidateTypeName[i]) == 0)
		{
			ntype = i;
			break;
		}
	}
	if (i == G_N_ELEMENTS (g_astrCandidateTypeName))
    {
        g_strfreev(tokens);
        return iRet;
    }    
	ptCand = nice_candidate_new(ntype);
	ptCand->component_id = 1;
	ptCand->stream_id = g_dwStreamID;
	ptCand->transport = NICE_CANDIDATE_TRANSPORT_UDP;
	strncpy(ptCand->foundation, tokens[0], NICE_CANDIDATE_MAX_FOUNDATION);
	ptCand->foundation[NICE_CANDIDATE_MAX_FOUNDATION - 1] = 0;
	ptCand->priority = atoi (tokens[1]);
	if (!nice_address_set_from_string(&ptCand->addr, tokens[2])) 
	{
		g_message("failed to parse addr: %s", tokens[2]);
		nice_candidate_free(ptCand);
		ptCand = NULL;
        g_strfreev(tokens);
        return iRet;
	}
    nice_address_set_port(&ptCand->addr, atoi (tokens[3]));
    
	if (ptCand == NULL) 
	{
		g_message("failed to parse candidate: %s", i_strCandidate);
        g_strfreev(tokens);
	    return iRet;
	}
	g_pRemoteCandidatesList = g_slist_prepend(g_pRemoteCandidatesList, ptCand);
    g_strfreev(tokens); 
    iRet=0;
    return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSetRemoteCandidates
-Description    : LibniceSetRemoteCandidates
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceSetRemoteCandidates()
{
	int iRet = -1;
    if (g_pRemoteCandidatesList == NULL) 
    {
		printf("g_pRemoteCandidatesList null \r\n");
	    return iRet;
    }
    if (g_ptAgent == NULL || g_dwStreamID == 0) 
    {
		printf("g_ptAgent null \r\n");
		return iRet;
    }
	// Note: this will trigger the start of negotiation.
	if (nice_agent_set_remote_candidates(g_ptAgent, g_dwStreamID, 1,g_pRemoteCandidatesList) < 1) 
	{
		g_message("failed to set remote candidates");
	}
	else
	{
		iRet=0;
	}
	if (g_pRemoteCandidatesList != NULL)
	{
		g_slist_free_full(g_pRemoteCandidatesList, (GDestroyNotify)&nice_candidate_free);
		g_pRemoteCandidatesList =NULL;
	}
	return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSetRemoteSDP
-Description    : LibniceSetRemoteSDP
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceSetRemoteSDP(char * i_strSDP)
{
	int iRet = -1;
    if (g_ptAgent == NULL || i_strSDP == NULL) 
    {
		printf("LibniceSetRemoteSDP g_ptAgent null \r\n");
		return iRet;
    }

	// Parse remote candidate list and set it on the agent
	if (nice_agent_parse_remote_sdp (g_ptAgent, i_strSDP) > 0) 
	{
		iRet = 0;
	} 
	else 
	{
		printf("LibniceSetRemoteSDP nice_agent_parse_remote_sdp fail \r\n");
	}
	return iRet;
}

/*****************************************************************************
-Fuction        : LibniceSendData
-Description    : LibniceSendData
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceSendData(char * i_acBuf,int i_iBufLen)
{
	int iRet = -1;
    if g_ptAgent == NULL || g_dwStreamID == 0 || i_acBuf == NULL) 
    {
		printf("LibniceSendData g_ptAgent null \r\n");
		return iRet;
    }
    if g_iLibniceSendReadyFlag == 0) 
    {
		printf("LibniceSendReady err \r\n");
		return iRet;
    }
    iRet = nice_agent_send(g_ptAgent, g_dwStreamID, 1, i_iBufLen, i_acBuf);
    return iRet;
}

/*****************************************************************************
-Fuction        : LibniceGetSendReadyFlag
-Description    : LibniceGetSendReadyFlag
-Input          : 
-Output         : 
-Return         : 
* Modify Date     Version             Author           Modification
* -----------------------------------------------
* 2020/01/13      V1.0.0              Yu Weifeng       Created
******************************************************************************/
int LibniceGetSendReadyFlag()
{
    return g_iLibniceSendReadyFlag;
}

static void LibniceCandidateGatheringDone(NiceAgent *i_ptAgent, guint i_dwStreamID,gpointer pData)
{
	g_debug("SIGNAL candidate gathering done\n");

	// Candidate gathering is done. Send our local candidates on stdout
	int iRet = -1;
	gchar *strLocalUfrag = NULL;
	gchar *strLocalPassword = NULL;
	gchar ipaddr[INET6_ADDRSTRLEN];
	GSList *cands = NULL, *item = NULL;
	static const gchar *transport_name[] = {"udp", "tcp_active", "tcp_passive", "tcp_so"};
	
	if (!nice_agent_get_local_credentials(i_ptAgent, i_dwStreamID,&strLocalUfrag, &strLocalPassword))
		goto end;
	cands = nice_agent_get_local_candidates(i_ptAgent, i_dwStreamID, 1);
	if (cands == NULL)
		goto end;

	for (item = cands; item; item = item->next) 
	{
		NiceCandidate *c = (NiceCandidate *)item->data;

		nice_address_to_string(&c->addr, ipaddr);

		// RFC 5245
		// a=candidate:<foundation> <component-id> <transport> <priority>
		// <connection-address> <port> typ <candidate-types>
		// [raddr <connection-address>] [rport <port>]
		// *(SP extension-att-name SP extension-att-value)
		//"candidate:3442447574 1 udp 2122260223 192.168.0.170 54653 typ host generation 0 ufrag gX6M network-id 1"
		snprintf(g_tLocalCandidate.strCandidateData,sizeof(g_tLocalCandidate.strCandidateData),
		"candidate:%s %u %s %u %s %u typ %s",
		c->foundation,c->component_id,transport_name[c->transport],c->priority,
		ipaddr,nice_address_get_port(&c->addr),g_astrCandidateTypeName[c->type]);
	}
	printf("%s ,%s %s", g_tLocalCandidate.strCandidateData,strLocalUfrag, strLocalPassword);
	g_tLocalCandidate.iGatheringDoneFlag = 1;
	iRet = 0;

	end:
		if (strLocalUfrag)
			g_free(strLocalUfrag);
		if (strLocalPassword)
			g_free(strLocalPassword);
		if (cands)
			g_slist_free_full(cands, (GDestroyNotify)&nice_candidate_free);
	return iRet;
}

 static void LibniceNewSelectPair(NiceAgent *agent, guint _stream_id,guint component_id, gchar *lfoundation,gchar *rfoundation, gpointer data)
{//此处开始dtls握手
	g_debug("SIGNAL: selected pair %s %s", lfoundation, rfoundation);
}

static void LibniceComponentStateChanged(NiceAgent *agent, guint _stream_id,guint component_id, guint state,gpointer data)
{
	static const gchar *state_name[] = {"disconnected", "gathering", "connecting","connected", "ready", "failed"};
    
	g_debug("SIGNAL: state changed %d %d %s[%d]\n",_stream_id, component_id, state_name[state], state);

	if (state == NICE_COMPONENT_STATE_READY) 
	{//协商成功
        g_iLibniceSendReadyFlag = 1;
	}
}

static void LibniceRecv(NiceAgent *agent, guint _stream_id, guint component_id,guint len, gchar *buf, gpointer data)
{
	if (len == 1 && buf[0] == '\0')
	{//这里接收浏览器发出的报文(包括dtls协商报文)

	}
	printf("%.*s", len, buf);
	//fflush(stdout);
}







