// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@gmail.com>
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Networking Support
//              I already wrote all the code for my other super secret game
//              project, so why don't I just copy and paste!
//              I stripped TCP support since that is not important to ReMooD.
//              I also removed semi-dual stack IPv6 support (since most
//              doomers use only IPv4).

/***************
*** INCLUDES ***
***************/

#if defined(__MSDOS__)
	// DOS Has Nothing

#elif defined(_WIN32)
	// WinSocks
	#include <winsock2.h>
	#include <ws2tcpip.h>	// IPv6
	#include <fcntl.h>
	
	#define __REMOOD_SOCKETCLOSE closesocket
	#define __REMOOD_DONTWAITMSG 0
#else
	// POSIX Sockets
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <fcntl.h>
	#include <errno.h>
	
	#define __REMOOD_SOCKETCLOSE close
	#define __REMOOD_DONTWAITMSG MSG_DONTWAIT
#endif

#include "i_util.h"
#include "i_net.h"
#include "z_zone.h"

/*****************
*** STRUCTURES ***
*****************/

/*****************************************************************************/
/********* BEGIN STOLEN NETWORKING CODE FROM MY SUPER SECRET PROJECT *********/

/* I_NetSocket_s -- Network socket */
struct I_NetSocket_s
{
	bool_t UDP;									// UDP Connection
#if !defined(__MSDOS__)
	I_HostAddress_t BindAddr;					// Address bound to
	I_HostAddress_t RemoteAddr;					// Address communicating to
	uint8_t IPvX;								// Which IP versions?
	bool_t BadOnlySix;							// Failed to make it IPv6 Only
	bool_t TCPConnected;						// Connected with TCP
	bool_t DidAcceptSix;						// Accepted IPv6 connection
	int AcceptFD;								// Accepted FD
	
	/* UNIX Stuff */
	int SockFD[2];								// Socket descriptor (from socket)
	int TCPFD[2];								// TCP descriptor (after accept)
	struct sockaddr_in v4Addr;					// IPv4 Address
	struct sockaddr_in6 v6Addr;					// IPv6 Address
	
	/* Socket Buffers */
	struct
	{
		uint8_t* Buf;							// Actual buffer
		size_t Size;							// Size of contained data
		size_t Max;								// Buffer size (maximum)
	} Buffers[2];
#endif
};

/********** END STOLEN NETWORKING CODE FROM MY SUPER SECRET PROJECT **********/
/*****************************************************************************/

/*************
*** LOCALS ***
*************/

static bool_t l_IPv6 = false;					// using IPv6 Mode

/****************
*** FUNCTIONS ***
****************/

/*****************************************************************************/
/********* BEGIN STOLEN NETWORKING CODE FROM MY SUPER SECRET PROJECT *********/

/*** COMMUNICATION ***/

#if !defined(__MSDOS__)
/* IS_NetAddrWrapToNative() -- Convert wrapped host to native address */
static void IS_NetAddrWrapToNative(const I_HostAddress_t* const a_Host, struct sockaddr_in* const a_V4, struct sockaddr_in6* const a_V6)
{
	size_t i;
	uint32_t T4;
	
	/* Check */
	if (!a_Host || (!a_V4 && !a_V6))
		return;
	
	/* Convert to v4 */
	if (a_V4)
	{
		a_V4->sin_family = AF_INET;
		a_V4->sin_port = htons(a_Host->Port);
		
		// Fill IP information
		for (T4 = 0, i = 0; i < 4; i++)
			T4 |= ((uint32_t)a_Host->Host.v4.b[i]) << ((3-i) * 8U);
		
		// Flip in
		a_V4->sin_addr.s_addr = htonl(T4);
	}
	
	/* Convert to v6 */
	if (a_V6)
	{
		a_V6->sin6_family = AF_INET6;
		a_V6->sin6_port = htons(a_Host->Port);
		
		for (i = 0; i < 16; i++)
			a_V6->sin6_addr.s6_addr[i] = a_Host->Host.v6.b[i];
	}
}
#endif

#if !defined(__MSDOS__)
/* IS_NetAddrNativeToWrap() -- Convert native address to wrapped address */
static void IS_NetAddrNativeToWrap(I_HostAddress_t* const a_Host, const struct sockaddr* const a_SockAddr)
{
	uint32_t T4;
	size_t i;
	
	/* Check */
	if (!a_Host || !a_SockAddr)
		return;
	
	/* Clear Host */
	memset(a_Host, 0, sizeof(a_Host));
	
	/* IPv4 Address */
	if (((struct sockaddr_in*)a_SockAddr)->sin_family == AF_INET)
	{
		// Fill flag
		a_Host->IPvX |= INIPVN_IPV4;
		
		// Convert to host byte order
		T4 = ((struct sockaddr_in*)a_SockAddr)->sin_addr.s_addr;
		T4 = htonl(T4);
		
		// Copy over IP into host
		a_Host->Port = ntohs(((struct sockaddr_in*)a_SockAddr)->sin_port);
		for (i = 0; i < 4; i++)
			a_Host->Host.v4.b[i] = (T4 >> ((3-i) * 8U)) & 0xFFU;
	}
	
	/* IPv6 Address */
	else if (((struct sockaddr_in6*)a_SockAddr)->sin6_family == AF_INET6)
	{
		// Fill flag
		a_Host->IPvX |= INIPVN_IPV6;
		
		// Copy over IP into host
		a_Host->Port = ntohs(((struct sockaddr_in6*)a_SockAddr)->sin6_port);
		for (i = 0; i < 16; i++)
			a_Host->Host.v6.b[i] = ((struct sockaddr_in6*)a_SockAddr)->sin6_addr.s6_addr[i];
	}
	
	/* Convert v4 address to v6? */
}
#endif

/* I_NetOpenSocket() -- Opens socket on port */
I_NetSocket_t* I_NetOpenSocket(const bool_t a_Server, const I_HostAddress_t* const a_Host, const uint16_t a_Port)
{
#if defined(__MSDOS__)
	return NULL;

#else
	I_NetSocket_t* NewSock;
	I_NetSocket_t TempSock;
	int SockOpt;
	int* pSockFD;
	size_t i;
	struct in6_addr Any6 = IN6ADDR_ANY_INIT;
	struct sockaddr* Addr;
	socklen_t SockLen;
	unsigned long NBVal;
	
	/* Check */
	// Conditions:
		// A server, but no port
	if (a_Server && !a_Port)
		return NULL;
	
	/* Attempt socket creation */
	// Clear
	memset(&TempSock, 0, sizeof(TempSock));
	TempSock.SockFD[0] = TempSock.SockFD[1] = -1;
	
	/* Create IPv4 Socket? */
	if (!M_CheckParm("-ipv6"))
	{
		l_IPv6 = false;
		
		// Create IPv4 Socket
		TempSock.SockFD[0] = -1;
		if (!a_Host || (a_Host && a_Host->IPvX & INIPVN_IPV4))
			TempSock.SockFD[0] = socket(AF_INET, SOCK_DGRAM, 0);
	
		// Worked?
		if (TempSock.SockFD[0] >= 0)
			// Set IPv4 OK
			TempSock.IPvX |= INIPVN_IPV4;
	}
	
	/* Make IPv6 socket */
	else
	{
		l_IPv6 = true;
		
		// Create IPv6 socket (if not binding to address, or if binding to one and there was IPv6 there)
		TempSock.SockFD[1] = -1;
		if (!a_Host || (a_Host && a_Host->IPvX & INIPVN_IPV6))
			TempSock.SockFD[1] = socket(AF_INET6, SOCK_DGRAM, 0);
	
		// Worked?
		if (TempSock.SockFD[1] >= 0)
		{
			// Set IPv6 OK
			TempSock.IPvX |= INIPVN_IPV6;
			
#if !defined(_WIN32)
			// Disable multibinding of IPv6 socket (Only Linux has IPv6 and IPv4 on a single sock)
			SockOpt = 1;
			if (setsockopt(TempSock.SockFD[1], IPPROTO_IPV6, IPV6_V6ONLY, (void*)&SockOpt, sizeof(SockOpt)) <= 0)
#endif
				TempSock.BadOnlySix = true;
		}
	}
	
	if (!(TempSock.IPvX & INIPVN_IPV4) && !(TempSock.IPvX & INIPVN_IPV6))
		return NULL;
	
	// TCP or UDP?
	TempSock.UDP = true;
	
	/* Start opening ports */
	// Open port on v6 side
	if (TempSock.IPvX & INIPVN_IPV6)
	{
		// Convert to native address
		IS_NetAddrWrapToNative(a_Host, NULL, &TempSock.v6Addr);
		
		// Bound to any address?
		if (!a_Host)
			memmove(&TempSock.v6Addr.sin6_addr, &Any6, sizeof(Any6));
		
		// Copy port
		TempSock.v6Addr.sin6_port = htons(a_Port);
	}
	
	// Open port on v4 side
	if (TempSock.IPvX & INIPVN_IPV4)
	{
		// Convert to native address
		IS_NetAddrWrapToNative(a_Host, &TempSock.v4Addr, NULL);
		
		// Bound to any address
		if (!a_Host)
			TempSock.v4Addr.sin_addr.s_addr = INADDR_ANY;
		
		// Copy port
		TempSock.v4Addr.sin_port = htons(a_Port);
	}
	
	// Bind to address specified
	// IPv4?
	if (TempSock.IPvX & INIPVN_IPV4)
	{
		Addr = (struct sockaddr*)&TempSock.v4Addr;
		SockLen = sizeof(TempSock.v4Addr);
		pSockFD = &TempSock.SockFD[0];
	}
	
	// IPv6?
	else
	{
		Addr = (struct sockaddr*)&TempSock.v6Addr;
		SockLen = sizeof(TempSock.v6Addr);
		pSockFD = &TempSock.SockFD[1];
	}
		
	// Set non-block socket
#if !defined(_WIN32)
	SockOpt = fcntl(*pSockFD, F_GETFL, 0);
	SockOpt |= O_NONBLOCK;
	fcntl(*pSockFD, F_SETFL, SockOpt);
#else
	// Win32 uses ioctlsocket()
	NBVal = 1;
	ioctlsocket(*pSockFD, FIONBIO, &NBVal);
#endif
	
	// Bind to address
	if (a_Server)
		if (bind(*pSockFD, Addr, SockLen) < 0)
		{
			// Clear bits and close socket
			__REMOOD_SOCKETCLOSE(*pSockFD);
			*pSockFD = -1;
			return NULL;
		}
	
	// Create new socket and place there
	NewSock = Z_Malloc(sizeof(*NewSock), PU_STATIC, NULL);
	memmove(NewSock, &TempSock, sizeof(TempSock));
	
	// Return the new socket
	return NewSock;
#endif
}

/* I_NetCloseSocket() -- Closes Socket */
void I_NetCloseSocket(I_NetSocket_t* const a_Socket)
{
	size_t i;
	
	/* Check */
	if (!a_Socket)
		return;
	
	/* Close all open files */
	for (i = 0; i < 2; i++)
		if (a_Socket->SockFD[i] >= 0)
			__REMOOD_SOCKETCLOSE(a_Socket->SockFD[i]);
	
	/* Free away */
	Z_Free(a_Socket);
}

/* IS_NetRecvWrap() -- Recieve data from remote end */
static size_t IS_NetRecvWrap(I_NetSocket_t* const a_Socket, I_HostAddress_t* const a_Host, void* const a_OutData, const size_t a_Len, const bool_t a_Peek, const bool_t a_CheckConn)
{
#if defined(__MSDOS__)
	return 0;
#else
	struct sockaddr_storage Addr;
	socklen_t SockLen;
	bool_t DisconSock;
	ssize_t RetVal;
	size_t i;
	
	/* Check */
	if (!a_Socket || !a_OutData || !a_Len)
		return 0;
	
	/* Recieve from which socket? */
	if (a_Socket->IPvX & INIPVN_IPV6)
		i = 1;
	else
		i = 0;
	
	// Receive from it
	SockLen = sizeof(Addr);
	
	if (a_Host)
		RetVal = recvfrom(a_Socket->SockFD[i], a_OutData, a_Len, __REMOOD_DONTWAITMSG | (a_Peek ? MSG_PEEK : 0), (struct sockaddr*)&Addr, &SockLen);
	else
		RetVal = recv(a_Socket->SockFD[i], a_OutData, a_Len, __REMOOD_DONTWAITMSG | (a_Peek ? MSG_PEEK : 0));
	
	// Error?
	if (RetVal < 0)
		return 0;
	
	// Convert address to host
	if (a_Host)
		IS_NetAddrNativeToWrap(a_Host, (struct sockaddr*)&Addr);
	
	/* Return written bytes */
	return RetVal;
#endif
}

/* I_NetReadyBytes() -- Determine bytes in net buffer */
size_t I_NetReadyBytes(I_NetSocket_t* const a_Socket, const size_t a_Bytes)
{
#if defined(__MSDOS__)
	return 0;
#else
	int8_t* JunkBuf;
	
	/* Check */
	if (!a_Socket || !a_Bytes)
		return 0;
	
	/* Peek the message */
	JunkBuf = alloca(a_Bytes);
	return IS_NetRecvWrap(a_Socket, NULL, JunkBuf, a_Bytes, true, true);
#endif
}

/* I_NetSend() -- Send Data */
size_t I_NetSend(I_NetSocket_t* const a_Socket, const I_HostAddress_t* const a_Host, const void* const a_InData, const size_t a_Len)
{
#if defined(__MSDOS__)
	return 0;
#else
	struct sockaddr_storage Addr;
	socklen_t SockLen;
	bool_t DisconSock;
	ssize_t RetVal;
	size_t i;
	
	/* Check */
	if (!a_Socket || !a_InData || !a_Len)
		return 0;
	
	/* Recieve from which socket? */
	if (a_Socket->IPvX & INIPVN_IPV6)
		i = 1;
	else
		i = 0;
		
	// Convert address to host
	if (a_Host)
		IS_NetAddrWrapToNative(a_Host, (!i ? &Addr : NULL), (i ? &Addr : NULL));
	
	// Receive from it
	SockLen = sizeof(Addr);

	if (a_Host)
		RetVal = sendto(a_Socket->SockFD[i], a_InData, a_Len, __REMOOD_DONTWAITMSG, (struct sockaddr*)&Addr, SockLen);
	else
		RetVal = send(a_Socket->SockFD[i], a_InData, a_Len, __REMOOD_DONTWAITMSG);
	
	// Error?
	if (RetVal < 0)
		return 0;
	
	/* Return written bytes */
	return RetVal;
#endif
}

/* I_NetRecv() -- Receive Data */
size_t I_NetRecv(I_NetSocket_t* const a_Socket, I_HostAddress_t* const a_Host, void* const a_OutData, const size_t a_Len)
{
	return IS_NetRecvWrap(a_Socket, a_Host, a_OutData, a_Len, false, true);
}

/*** NAME RESOLUTION ***/

/* I_NetNameToHost() -- Convert hostname to an IP Address */
// I was only able to test IPv6 with localhost, that is all but it should
// work hopefully!
bool_t I_NetNameToHost(I_HostAddress_t* const a_Host, const char* const a_Name)
{
#if defined(__MSDOS__)
	/* DOS Does not support networking */
	return false;
	
#elif defined(_WIN32)
	/* WinXP+ supports getnameinfo() etc. but I want Win98 Support */
	struct hostent* Ent;
	
	/* Check */
	if (!a_Host || !a_Name)
		return false;
	
	/* Obtain host info */
	Ent = gethostbyname(a_Name);
	
	// No IP found?
	if (!Ent)
		return false;
	
	/* IPv4/IPv6 Check */
	if ((l_IPv6 && Ent->h_addrtype != AF_INET6) || (!l_IPv6 && Ent->h_addrtype != AF_INET))
		return false;
	
	/* Convert */
	IS_NetAddrNativeToWrap(a_Host, Ent->h_addr);
	
	/* Success! */
	return true;
	
#elif !defined(_WIN32) || (defined(_WIN32) && _WIN32_WINNT >= 0x0501)
	/* Otherwise, use the good stuff */
	size_t i;
	struct addrinfo AddrInfo;
	struct addrinfo* Find, *Rover;
	struct sockaddr_in6 SockInfoSix;
	struct sockaddr_in SockInfoFour;
	
	/* Check */
	if (!a_Host || !a_Name)
		return false;
	
	/* Clear all */
	memset(a_Host, 0, sizeof(*a_Host));
	memset(&AddrInfo, 0, sizeof(AddrInfo));
	memset(&SockInfoSix, 0, sizeof(SockInfoSix));
	memset(&SockInfoFour, 0, sizeof(SockInfoFour));
	
	/* Prepare for getaddrinfo */
	// I want both IPv4 and IPv6 addresses!
	AddrInfo.ai_family = AF_UNSPEC;
	AddrInfo.ai_socktype = SOCK_STREAM;
#if !defined(_WIN32)
	AddrInfo.ai_flags = AI_ADDRCONFIG | AI_ALL | AI_V4MAPPED;
#endif
	
	/* Get address info */
	if (getaddrinfo(a_Name, NULL, &AddrInfo, &Find) != 0)
		return false;
	
	/* Rove through addresses */
	for (Rover = Find; Rover; Rover = Rover->ai_next)
	{
		// Not IPv4 or IPv6? (UNIX sockets!?)
		if (Rover->ai_family != AF_INET && Rover->ai_family != AF_INET6)
			continue;
		
		// Not TCP or UDP (other kinds of net sockets)
		if (Rover->ai_socktype != SOCK_STREAM && Rover->ai_socktype != SOCK_DGRAM)
			continue;
		
		// If this is an IPv4 Address and host has none, use it!
		if (!(a_Host->IPvX & INIPVN_IPV4) && Rover->ai_family == AF_INET)
		{
			// Fill flag
			a_Host->IPvX |= INIPVN_IPV4;
			
			// Copy over (in case of unaligned access)
			memmove(&SockInfoFour, Rover->ai_addr, Rover->ai_addrlen);
			
			// Convert to host byte order
			SockInfoFour.sin_addr.s_addr = ntohl(SockInfoFour.sin_addr.s_addr);
			
			// Copy over IP into host
			for (i = 0; i < 4; i++)
				a_Host->Host.v4.b[i] = (SockInfoFour.sin_addr.s_addr >> ((3-i) * 8U)) & 0xFFU;
		}
		
		// Same as above, but for IPv6
		else if (!(a_Host->IPvX & INIPVN_IPV6) && Rover->ai_family == AF_INET6)
		{
			// Fill flag
			a_Host->IPvX |= INIPVN_IPV6;
			
			// Copy over (in case of unaligned access)
			memmove(&SockInfoSix, Rover->ai_addr, Rover->ai_addrlen);
			
			// Copy over IP into host
			for (i = 0; i < 16; i++)
				a_Host->Host.v6.b[i] = SockInfoSix.sin6_addr.s6_addr[i];
		}
	}
	
	/* If v4 && !v6, then convert v4 to v6 */
	if ((a_Host->IPvX & INIPVN_IPV4) && !(a_Host->IPvX & INIPVN_IPV6))
	{
		// Set six
		a_Host->IPvX |= INIPVN_IPV6;
		
		// FF for the almost last set
		for (i = 10; i < 12; i++)
			a_Host->Host.v6.b[i] = 0xFF;
		
		// Add remaining IP to end
		for (i = 0; i < 4; i++)
			a_Host->Host.v6.b[i + 12] = a_Host->Host.v4.b[i];
	}
	
	/* Free address info */
	freeaddrinfo(Find);
	
	/* Return true ONLY if an IP was found */
	if (a_Host->IPvX)
		return true;
	return false;
#endif
}

/* I_NetHostToName() -- Converts a host to a named address */
bool_t I_NetHostToName(const I_HostAddress_t* const a_Host, char* const a_Out, const size_t a_OutSize)
{
#if defined(__MSDOS__)
	/* DOS Does not support networking */
	return false;
	
#elif defined(_WIN32)
	/* WinXP+ supports getnameinfo() etc. but I want Win98 Support */
	struct hostent* Ent;
	struct sockaddr_in6 SockInfoSix;
	struct sockaddr_in SockInfoFour;
	
	/* Check */
	if (!a_Host || !a_Out || !a_OutSize)
		return false;
		
	/* Convert */
	// IPv6
	if (l_IPv6)
		IS_NetAddrWrapToNative(a_Host, NULL, &SockInfoSix);
	
	// IPv4
	else
		IS_NetAddrWrapToNative(a_Host, &SockInfoFour, NULL);
	
	/* Obtain host info */
	Ent = gethostbyaddr(
			(l_IPv6 ? &SockInfoSix : &SockInfoFour),
			(l_IPv6 ? sizeof(SockInfoSix) : sizeof(SockInfoFour)),
			(l_IPv6 ? AF_INET6 : AF_INET)
			);
	
	// No Name found?
	if (!Ent)
		return false;
	
	/* Copy string */
	strncpy(a_Out, Ent->h_name, a_OutSize - 1);
	
	/* Success! */
	return true;
	
#elif !defined(_WIN32) || (defined(_WIN32) && _WIN32_WINNT >= 0x0501)
	/* Otherwise, use the good stuff */
	bool_t RetVal;
	size_t i;
	uint32_t T4;
	struct sockaddr_in6 SockInfoSix;
	struct sockaddr_in SockInfoFour;
	
	/* Check */
	if (!a_Host || !a_Out || !a_OutSize)
		return false;
	
	/* Clear */
	RetVal = false;
	memset(&SockInfoSix, 0, sizeof(SockInfoSix));
	memset(&SockInfoFour, 0, sizeof(SockInfoFour));
	
	/* First try IPv6 */
	if (a_Host->IPvX & INIPVN_IPV6)
	{
		// Fill IP information
		for (i = 0; i < 16; i++)
			SockInfoSix.sin6_addr.s6_addr[i] = a_Host->Host.v6.b[i];
		
		// Fill remaining info
		SockInfoSix.sin6_family = AF_INET6;
		
		// Get name of host
		if (getnameinfo((struct sockaddr*)&SockInfoSix, sizeof(SockInfoSix), a_Out, a_OutSize, NULL, 0, 0) == 0)
			if (a_Out[0] != '\0')
				RetVal = true;	// It worked!
	}
	
	/* Second, try IPv4 */
	if (!RetVal && (a_Host->IPvX & INIPVN_IPV4))
	{
		// Fill IP information
		for (T4 = 0, i = 0; i < 4; i++)
			T4 |= ((uint32_t)a_Host->Host.v4.b[i]) << ((3-i) * 8U);
		
		// Flip in
		SockInfoFour.sin_addr.s_addr = htonl(T4);
		
		// Fill remaining info
		SockInfoFour.sin_family = AF_INET;
		
		// Get name of host
		if (getnameinfo((struct sockaddr*)&SockInfoFour, sizeof(SockInfoFour), a_Out, a_OutSize, NULL, 0, 0) == 0)
			if (a_Out[0] != '\0')
				RetVal = true;	// It worked!
	}
	
	/* Return */
	return RetVal;
#endif
}

/********** END STOLEN NETWORKING CODE FROM MY SUPER SECRET PROJECT **********/
/*****************************************************************************/


/* I_InitNetwork() -- Initializes the network */
bool_t I_InitNetwork(void)
{
	/* Success! */
	return true;
}

