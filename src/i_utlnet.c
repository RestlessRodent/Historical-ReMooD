// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// Copyright (C) 2011-2013 GhostlyDeath <ghostlydeath@gmail.com>
// ----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// ----------------------------------------------------------------------------
// DESCRIPTION: Networking Support

/***************
*** INCLUDES ***
***************/

//#include "doomtype.h"

#define __REMOOD_SOCKNONE	0					// No Sockets
#define __REMOOD_SOCKBSD	1					// BSD Sockets
#define __REMOOD_SOCKPOSIX	2					// POSIX Sockets
#define __REMOOD_SOCKWIN	3					// WinSock Sockets
#define __REMOOD_SOCKNETLIB	4					// NetLib Sockets 

#ifndef __REMOOD_SOCKLEVEL
	#if defined(__MSDOS__)
		#define __REMOOD_SOCKLEVEL __REMOOD_SOCKNONE
	#elif defined(__palmos__)
		#define __REMOOD_SOCKLEVEL __REMOOD_SOCKNETLIB
	#elif defined(_WIN32)
		#define __REMOOD_SOCKLEVEL __REMOOD_SOCKWIN
	#else
		#define __REMOOD_SOCKLEVEL __REMOOD_SOCKPOSIX
	#endif
#endif

/* IPv6? */
#if !defined(__REMOOD_NOIPV6)
	#define __REMOOD_ENABLEIPV6
#endif

/* Include The Correct Headers */
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <fcntl.h>
	#include <errno.h>
	
	#define __REMOOD_DONTWAITMSG MSG_DONTWAIT

#elif __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <fcntl.h>
	#include <errno.h>
	
	#define __REMOOD_DONTWAITMSG MSG_DONTWAIT

#elif __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	#include <winsock2.h>
	#include <ws2tcpip.h>	// IPv6
	#include <fcntl.h>
	#include <wspiapi.h>
	
	#define __REMOOD_DONTWAITMSG 0

#elif __REMOOD_SOCKLEVEL == __REMOOD_SOCKNETLIB

#else
	// No Sockets
#endif

//#include "i_util.h"
//#include "i_net.h"
//#include "z_zone.h"
//#include "dstrings.h"
//#include "console.h"

#if defined(__REMOOD_ENABLEIPV6)
	#if !defined(IPV6_JOIN_GROUP) && defined(IPV6_ADD_MEMBERSHIP)
		#define IPV6_JOIN_GROUP IPV6_ADD_MEMBERSHIP
	#endif
	
	#if !defined(IPV6_JOIN_GROUP)
		#define __REMOOD_NOIPV6MULTICAST
	#endif
#endif

/*****************
*** STRUCTURES ***
*****************/

/*************
*** LOCALS ***
*************/

/****************
*** FUNCTIONS ***
****************/

/*** COMMUNICATION ***/

/* I_NetHashHost() -- Hashes a host */
uint32_t I_NetHashHost(const I_HostAddress_t* const a_Host)
{
	register int i;
	uint32_t HashVal;
	
	/* Check */
	if (!a_Host)
		return 0;
	
	/* Init */
	HashVal = 0;
	
	/* Hash it */
	// IPv4
	if (a_Host->IPvX == INIPVN_IPV4)
	{
		HashVal = a_Host->Host.v4.u;
		HashVal ^= ((uint32_t)a_Host->Port) << 16U;
	}
	
	// IPv6
	else if (a_Host->IPvX == INIPVN_IPV6)
	{
		for (i = 0; i < 16; i++)
			HashVal ^= ((uint32_t)a_Host->Host.v6.Addr.b[i]) << ((i & 3) * 8);
		HashVal ^= a_Host->Host.v6.Scope;
		HashVal ^= ((uint32_t)a_Host->Port) << 16U;
	}
	
	// Unknown
	else
	{
	}
	
	/* Return hash result */
	return HashVal;
}

/* I_NetCompareHost() -- Returns true if the hosts are the same */
bool_t I_NetCompareHost(const I_HostAddress_t* const a_A, const I_HostAddress_t* const a_B)
{
	size_t i;
	
	/* Check */
	if (!a_A || !a_B)
		return false;
	
	/* No IP Data at all? */
	if (!a_A->IPvX && !a_B->IPvX)
		return true;	// Set as successful (since nothing matches nothing)
	
	/* Port Mismatch */
	if (a_A->Port != a_B->Port)
		return false;
	
	/* Protocol Mismatch */
	if (a_A->IPvX != a_B->IPvX)
		return false;
	
	/* IPv4 */
	if (a_A->IPvX == INIPVN_IPV4)
	{
		// Check
		if (a_A->Host.v4.u != a_B->Host.v4.u)
			return false;
		return true;
	}
	
	/* IPv6 */
	else if (a_A->IPvX == INIPVN_IPV6)
	{
		// Check
		if (a_A->Host.v6.Scope != a_B->Host.v6.Scope)
			return false;
		for (i = 0; i < 4; i++)
			if (a_A->Host.v6.Addr.u[i] != a_B->Host.v6.Addr.u[i])
				return false;
		return true;
	}
	
	/* Was Not Matched */
	else
		return false;
}

/* I_NetSocket_s -- Socket Data */
struct I_NetSocket_s
{
	uint32_t Flags;								// Socket Flags
	
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	int SockFD;
	socklen_t SockLen;
	struct sockaddr_storage Addr;
#endif
};

/* I_InitNetwork() -- Initializes the network */
bool_t I_InitNetwork(void)
{
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	/* WinSock requires initialization */
	WSADATA wsaData;
	WORD version;
	
	// Get version
	version = MAKEWORD(2, 0);
	
	// Attempt Initialization
	if (WSAStartup(version, &wsaData) != 0)
		return false;
	
	// Bad version?
	if (LOBYTE(wsaData.wVersion) != LOBYTE(version) &&
		HIBYTE(wsaData.wVersion) != HIBYTE(version))
	{
		WSACleanup();
		return false;
	}
#endif

	/* Success! */
	return true;
}

#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
/* IS_ConvertHost() -- Converts hostname */
static bool_t IS_ConvertHost(const bool_t a_ToNative, I_HostAddress_t* const a_Host, struct sockaddr_storage* const a_Native, socklen_t* const a_Len)
{
	size_t i;
	uint32_t T4;
	
	/* Wrapped to Native */
	if (a_ToNative)
	{
		// IPv4
		if (a_Host->IPvX == INIPVN_IPV4)
		{
			// Init
			((struct sockaddr_in*)a_Native)->sin_family = AF_INET;
			((struct sockaddr_in*)a_Native)->sin_port = htons(a_Host->Port);
		
			// Fill IP information
			for (T4 = 0, i = 0; i < 4; i++)
				T4 |= ((uint32_t)a_Host->Host.v4.b[i]) << ((3-i) * 8U);
		
			// Flip in
			((struct sockaddr_in*)a_Native)->sin_addr.s_addr = htonl(T4);
			
			// Set length
			if (a_Len)
				*a_Len = sizeof(struct sockaddr_in);
			
			// Success!
			return true;
		}
		
#if defined(__REMOOD_ENABLEIPV6)
		// IPv6
		else if (a_Host->IPvX == INIPVN_IPV6)
		{
			((struct sockaddr_in6*)a_Native)->sin6_family = AF_INET6;
			((struct sockaddr_in6*)a_Native)->sin6_port = htons(a_Host->Port);
	
			for (i = 0; i < 16; i++)
				((struct sockaddr_in6*)a_Native)->sin6_addr.s6_addr[i] = a_Host->Host.v6.Addr.b[i];
				
			((struct sockaddr_in6*)a_Native)->sin6_scope_id = a_Host->Host.v6.Scope;
			
			// Set length
			if (a_Len)
				*a_Len = sizeof(struct sockaddr_in6);
			
			// Success!
			return true;
		}
#endif
		
		// Missed
		return false;
	}
	
	/* Native to Wrapped */
	else
	{
		// IPv4 Address
		if (((struct sockaddr_in*)a_Native)->sin_family == AF_INET)
		{
			// Fill flag
			a_Host->IPvX = INIPVN_IPV4;
		
			// Convert to host byte order
			T4 = ((struct sockaddr_in*)a_Native)->sin_addr.s_addr;
			T4 = htonl(T4);
		
			// Copy over IP into host
			a_Host->Port = ntohs(((struct sockaddr_in*)a_Native)->sin_port);
			for (i = 0; i < 4; i++)
				a_Host->Host.v4.b[i] = (T4 >> ((3-i) * 8U)) & 0xFFU;
			
			// Success!
			return true;
		}
	
#if defined(__REMOOD_ENABLEIPV6)
		// IPv6 Address
		else if (((struct sockaddr_in6*)a_Native)->sin6_family == AF_INET6)
		{
			// Fill flag
			a_Host->IPvX = INIPVN_IPV6;
		
			// Copy over IP into host
			a_Host->Port = ntohs(((struct sockaddr_in6*)a_Native)->sin6_port);
			for (i = 0; i < 16; i++)
				a_Host->Host.v6.Addr.b[i] = ((struct sockaddr_in6*)a_Native)->sin6_addr.s6_addr[i];
			a_Host->Host.v6.Scope = ((struct sockaddr_in6*)a_Native)->sin6_scope_id;
				
			// Success!
			return true;
		}
		
		// Failed
		return false;
#endif
	}
}
#endif

/* I_NetNameToHost() -- Converts hostname to address */
bool_t I_NetNameToHost(I_NetSocket_t* const a_Socket, I_HostAddress_t* const a_Host, const char* const a_Name)
{
#define BUFSIZE	256
	char Buf[BUFSIZE];
	char* PortP, *Br;
	uint16_t MatchPort;
	
	/* Variables */
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	size_t i;
	struct addrinfo AddrInfo;
	struct addrinfo* Find, *Rover;
	struct sockaddr_in SockInfoFour;
	struct sockaddr_in6 SockInfoSix;
#elif __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	struct hostent* Ent;
#else
#endif
	
	/* Check */
	if (!a_Host || !a_Name)
		return false;
	
	/* Extract port (if any) */
	memset(Buf, 0, sizeof(Buf));
	
	// V6 Addr?
	if (a_Name[0] == '[')
	{
		strncpy(Buf, a_Name + 1, BUFSIZE - 1);
		Br = strchr(Buf, ']');
		
		// Require Matching bracket
		if (!Br)
			return false;
		
		// Port is after the bracket
		PortP = strchr(Br, ':');
		
		// remove bracket
		*Br = 0;
	}
	
	// V4 Addr
	else
	{
		strncpy(Buf, a_Name, BUFSIZE - 1);
		
		PortP = strchr(Buf, ':');
	}
	
	// No :, presume default port
	if (!PortP)
		MatchPort = __REMOOD_BASEPORT;

	// Otherwise, take the name out
	else
	{
		// Erase it
		*PortP = 0;
	
		// Move it up
		PortP++;
	
		// Translate to integer
		MatchPort = C_strtou32(PortP, NULL, 10);
	}

	/* Code */
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
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
	AddrInfo.ai_flags = AI_ADDRCONFIG | AI_ALL;
#endif

	/* Get address info */
	if (getaddrinfo(Buf, NULL, &AddrInfo, &Find) != 0)
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
		if (((a_Socket && !(a_Socket->Flags & INSF_V6)) || a_Host->IPvX == INIPVN_IPV4 || (!a_Socket && !a_Host->IPvX)) && Rover->ai_family == AF_INET)
		{
			// Fill flag
			a_Host->IPvX = INIPVN_IPV4;
			
			// Copy over (in case of unaligned access)
			memmove(&SockInfoFour, Rover->ai_addr, Rover->ai_addrlen);
			
			// Convert to host byte order
			SockInfoFour.sin_addr.s_addr = ntohl(SockInfoFour.sin_addr.s_addr);
			
			// Copy over IP into host
			for (i = 0; i < 4; i++)
				a_Host->Host.v4.b[i] = (SockInfoFour.sin_addr.s_addr >> ((3-i) * 8U)) & 0xFFU;
			a_Host->Port = MatchPort;
			
			// Done
			freeaddrinfo(Find);
			return true;
		}
		
		// Same as above, but for IPv6
		else if (((a_Socket && !(a_Socket->Flags & INSF_V6)) || a_Host->IPvX == INIPVN_IPV6 || (!a_Socket && !a_Host->IPvX)) && Rover->ai_family == AF_INET6)
		{
			// Fill flag
			a_Host->IPvX = INIPVN_IPV6;
			
			// Copy over (in case of unaligned access)
			memmove(&SockInfoSix, Rover->ai_addr, Rover->ai_addrlen);
			
			// Copy over IP into host
			for (i = 0; i < 16; i++)
				a_Host->Host.v6.Addr.b[i] = SockInfoSix.sin6_addr.s6_addr[i];
			a_Host->Host.v6.Scope = SockInfoSix.sin6_scope_id;
			
			a_Host->Port = MatchPort;
			
			// Success!
			freeaddrinfo(Find);
			return true;
		}
	}
	
	/* Free address info */
	freeaddrinfo(Find);
	
	/* No match found */
	return false;
	
#elif __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	/* WinXP+ supports getnameinfo() etc. but I want Win98 Support */
	
	/* Obtain host info */
	Ent = gethostbyname(Buf);
	
	// No IP found?
	if (!Ent)
		return false;
	
	/* IPv4/IPv6 Check */
	// TODO FIXME
	//if ((l_IPv6 && Ent->h_addrtype != AF_INET6) || (!l_IPv6 && Ent->h_addrtype != AF_INET))
	//	return false;
	
	/* Convert */
	IS_ConvertHost(false, a_Host, (const struct sockaddr*)Ent->h_addr, NULL);
	a_Host->Port = MatchPort;
	
	/* Success! */
	return true;
#else
	return false;
#endif

#undef BUFSIZE
}

bool_t I_NetHostToName(I_NetSocket_t* const a_Socket, const I_HostAddress_t* const a_Host, char* const a_Out, const size_t a_OutSize)
{
	return false;
}

/* I_NetHostToString() -- Converts hostname to string */
size_t I_NetHostToString(const I_HostAddress_t* const a_Host, char* const a_Out, const size_t a_OutSize)
{
	char* p;
	int32_t l;
	uint32_t s, m, c, i;
	bool_t Zero, DidIt, InRun;
	
	/* Check */
	if (!a_Out || !a_OutSize)
		return 0;
	
	/* NUL at end always */
	a_Out[a_OutSize - 1] = 0;
	
	/* No Host? */
	if (!a_Host)
		return snprintf(a_Out, a_OutSize - 1, "\0\0");
	
	/* IPv4? */
	else if (a_Host->IPvX == INIPVN_IPV4)
		return snprintf(a_Out, a_OutSize - 1, "%i.%i.%i.%i:%u\0\0",
				a_Host->Host.v4.b[0],
				a_Host->Host.v4.b[1],
				a_Host->Host.v4.b[2],
				a_Host->Host.v4.b[3],
				a_Host->Port
			);
	
	/* IPv6? */
	else if (a_Host->IPvX == INIPVN_IPV6)
	{
		p = a_Out;
		l = 0;
		
		// Opening bracket
		if (l < a_OutSize - 1)
			p[l++] = '[';
		
		// Actual Address
		DidIt = InRun = false;
		for (i = 0; i < 8; i++)
		{
			// Get address piece
			s = a_Host->Host.v6.Addr.s[i];
			
			// Print :: in place of zero?
			if (!InRun && !DidIt)
				if (s == 0)
				{
					InRun = true;
					DidIt = true;
					
					// if this is the first, nothing would have been printed
					if (i == 0)
						if (l < a_OutSize - 1)
							p[l++] = ':';
					
					// Print colon
					if (l < a_OutSize - 1)
						p[l++] = ':';
				}
			
			// Skipping?
			if (InRun)
			{
				// No more zero
				if (s != 0)
					InRun = false;
				else
					continue;
			}
			
			// Print Number
			for (Zero = false, m = UINT32_C(0x10000000); m; m /= 16)
			{
				c = (s / m) % 16;
			
				if (c != 0)
					Zero = true;
			
				if (Zero || m == 1)
					if (l < a_OutSize - 1)
						if (c < 10)
							p[l++] = '0' + c;
						else
							p[l++] = 'a' + (c - 10);
			}
			
			// Print colon?
			if (i < 7 && !InRun)
				if (l < a_OutSize - 1)
					p[l++] = ':';
		}
		
		// Scope
		if (l < a_OutSize - 1)
			p[l++] = '%';
		s = a_Host->Host.v6.Scope;
		
		for (Zero = false, m = UINT32_C(1000000000); m; m /= 10)
		{
			c = ((s / m) % 10);
			
			if (c != 0)
				Zero = true;
			
			if (Zero || m == 1)
				if (l < a_OutSize - 1)
					p[l++] = '0' + c;
		}
		
		// Closing bracket
		if (l < a_OutSize - 1)
			p[l++] = ']';
		
		// Port
		if (l < a_OutSize - 1)
			p[l++] = ':';
		s = a_Host->Port;
		
		for (Zero = false, m = UINT32_C(1000000000); m; m /= 10)
		{
			c = ((s / m) % 10);
			
			if (c != 0)
				Zero = true;
			
			if (Zero || m == 1)
				if (l < a_OutSize - 1)
					p[l++] = '0' + c;
		}
		
		// Always have 0 at end
		if (l < a_OutSize - 1)
			p[l++] = 0;
		p[a_OutSize - 1] = 0;
		
		// Return size of string
		return l;
	}
	
	/* Unknown? */
	else
		return snprintf(a_Out, a_OutSize - 1, ":%u\0\0", a_Host->Port);
}

/* I_NetOpenMultiCastSocket() -- Opens multicast socket */
I_NetSocket_t* I_NetOpenMultiCastSocket(const bool_t a_IPv6, const uint16_t a_Port)
{
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	int SockFD, SockOpt, GrpRet;
	struct sockaddr* Addr;
	socklen_t SockLen;
	struct sockaddr_in In4;
	struct ip_mreq MCGroupF;
#if defined(__REMOOD_ENABLEIPV6)
	struct sockaddr_in6 In6;
	struct ipv6_mreq MCGroupS;
#endif
	I_NetSocket_t* RetVal;
	
	/* V6 not supported? */
#if !defined(__REMOOD_ENABLEIPV6) || defined(__REMOOD_NOIPV6MULTICAST)
	if (a_IPv6)
		return NULL;
#endif
	
	/* Attempt socket creation */
#if defined(__REMOOD_ENABLEIPV6)
	if (a_IPv6)
		SockFD = socket(AF_INET6, (SOCK_DGRAM), 0);
	else
#endif
		SockFD = socket(AF_INET, (SOCK_DGRAM), 0);
	
	// Failed
	if (SockFD < 0)
		return NULL;
	
	/* Set IPv6 socket to IPv6 only */
#if defined(__REMOOD_ENABLEIPV6)
	if (a_IPv6)
	{
#if defined(IPV6_V6ONLY)
		SockOpt = 1;
		setsockopt(SockFD, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&SockOpt, sizeof(SockOpt));
#endif
	}
#endif

	/* Re-Use Address */
	SockOpt = 1;
	setsockopt(SockFD, SOL_SOCKET, SO_REUSEADDR, (void*)&SockOpt, sizeof(SockOpt));
	
	/* Bind to any address for a single port */
#if defined(__REMOOD_ENABLEIPV6)
	if (a_IPv6)
	{
		// Set Any (all zeroes)
		memset(&In6, 0, sizeof(In6));
		In6.sin6_family = AF_INET6;
		In6.sin6_port = htons(a_Port);
		Addr = &In6;
		SockLen = sizeof(In6);
	}
	else
#endif
	{
		// Set Any
		In4.sin_addr.s_addr = INADDR_ANY;
		In4.sin_family = AF_INET;
		In4.sin_port = htons(a_Port);
		Addr = &In4;
		SockLen = sizeof(In4);
	}
	
	/* Bind */
	if (bind(SockFD, Addr, SockLen) < 0)
	{
		// Show error
		CONL_OutputUT(CT_NETWORK, DSTR_IUTLNET_BADUNIXBIND, "%i%s\n", errno, strerror(errno));
		
		// Close created socket
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
		closesocket(SockFD);
#else
		close(SockFD);
#endif
		return NULL;
	}
	
	/* Set non-blocking socket */
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	NBVal = 1;
	ioctlsocket(SockFD, FIONBIO, &NBVal);
#else
	SockOpt = fcntl(SockFD, F_GETFL, 0);
	SockOpt |= O_NONBLOCK;
	fcntl(SockFD, F_SETFL, SockOpt);
#endif

	/* Join multi-cast group */
	GrpRet = -1;
#if defined(__REMOOD_ENABLEIPV6)
	if (a_IPv6)
	{
		memset(&MCGroupF, 0, sizeof(MCGroupF));
		
		inet_pton(AF_INET6, "ff02::1337", &MCGroupS.ipv6mr_multiaddr);
		
#if !defined(__REMOOD_NOIPV6MULTICAST)
		GrpRet = setsockopt(SockFD, IPPROTO_IPV6, IPV6_JOIN_GROUP, &MCGroupS, sizeof(MCGroupS));
#endif
	}
	else
#endif
	{
		memset(&MCGroupF, 0, sizeof(MCGroupF));
		
		inet_pton(AF_INET, "224.0.0.167", &MCGroupF.imr_multiaddr);
		MCGroupF.imr_interface.s_addr = INADDR_ANY;
		
		GrpRet = setsockopt(SockFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, &MCGroupF, sizeof(MCGroupF));
	}
	
	/* If it failed, die */
	if (GrpRet < 0)
	{
		// Close created socket
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
		closesocket(SockFD);
#else
		close(SockFD);
#endif
		return NULL;
	}
	
	/* Allocate for socket return */
	RetVal = Z_Malloc(sizeof(*RetVal), PU_STATIC, NULL);
	
	// Set Values
	RetVal->Flags = (a_IPv6 ? INSF_V6 : 0) | INSF_MULTICAST;
	RetVal->SockFD = SockFD;
	memmove(&RetVal->Addr, Addr, SockLen);
	RetVal->SockLen = SockLen;
	
	// Return it
	return RetVal;
#else
	/* Not Implemented */
	return NULL;
#endif
}

/* I_NetOpenSocket() -- Opens a socket on the specified port */
I_NetSocket_t* I_NetOpenSocket(const uint32_t a_Flags, const I_HostAddress_t* const a_Host, const uint16_t a_Port)
{
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	int SockFD, SockOpt, Err;
	unsigned long NBVal;
	struct sockaddr* Addr;
	socklen_t SockLen;
	struct sockaddr_in In4;
#if defined(__REMOOD_ENABLEIPV6)
	struct sockaddr_in6 In6;
#endif
	I_NetSocket_t* RetVal;
	
	/* V6 not supported? */
#if !defined(__REMOOD_ENABLEIPV6)
	if (a_Flags & INSF_V6)
		return NULL;
#endif
	
	/* Attempt socket creation */
#if defined(__REMOOD_ENABLEIPV6)
	if (a_Flags & INSF_V6)
		SockFD = socket(AF_INET6, ((a_Flags & INSF_TCP) ? SOCK_STREAM : SOCK_DGRAM), 0);
	else
#endif
		SockFD = socket(AF_INET, ((a_Flags & INSF_TCP) ? SOCK_STREAM : SOCK_DGRAM), 0);
	
	// Creation failed
	if (SockFD < 0)
		return NULL;
	
	/* Bind to local address and port */
#if defined(__REMOOD_ENABLEIPV6)
	if (a_Flags & INSF_V6)
	{
		// Disable multi-binding on IPv6 socket
#if defined(IPV6_V6ONLY)
		SockOpt = 1;
		setsockopt(SockFD, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&SockOpt, sizeof(SockOpt));
#endif
		
		// Set Any (all zeroes)
		memset(&In6, 0, sizeof(In6));
		
		// Lookup Host?
		if (a_Host && a_Host->IPvX == INIPVN_IPV6)
			IS_ConvertHost(true, a_Host, &In6, NULL);
			
		// Set Port
		In6.sin6_family = AF_INET6;
		In6.sin6_port = htons(a_Port);
		Addr = &In6;
		SockLen = sizeof(In6);
	}
	else
#endif
	{
		// Set Any
		In4.sin_addr.s_addr = INADDR_ANY;
		
		// Lookup Host?
		if (a_Host && a_Host->IPvX == INIPVN_IPV4)
			IS_ConvertHost(true, a_Host, &In4, NULL);
		
		// Set Port
		In4.sin_family = AF_INET;
		In4.sin_port = htons(a_Port);
		Addr = &In4;
		SockLen = sizeof(In4);
	}
	
	/* Bind */
	if (bind(SockFD, Addr, SockLen) < 0)
	{
		// Show error
		CONL_OutputUT(CT_NETWORK, DSTR_IUTLNET_BADUNIXBIND, "%i%s\n", errno, strerror(errno));
		
		// Close created socket
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
		closesocket(SockFD);
#else
		close(SockFD);
#endif
		return NULL;
	}
	
	/* Set non-blocking socket */
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	NBVal = 1;
	ioctlsocket(SockFD, FIONBIO, &NBVal);
#else
	SockOpt = fcntl(SockFD, F_GETFL, 0);
	SockOpt |= O_NONBLOCK;
	fcntl(SockFD, F_SETFL, SockOpt);
#endif
	
	/* Allocate for socket return */
	RetVal = Z_Malloc(sizeof(*RetVal), PU_STATIC, NULL);
	
	// Set Values
	RetVal->Flags = a_Flags;
	RetVal->SockFD = SockFD;
	memmove(&RetVal->Addr, Addr, SockLen);
	RetVal->SockLen = SockLen;
	
	// Return it
	return RetVal;
#else
	/* Not Implemented */
	return NULL;
#endif
}

/* I_NetCloseSocket() -- Closes the specified socket */
void I_NetCloseSocket(I_NetSocket_t* const a_Socket)
{
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	/* Check */
	if (!a_Socket)
		return;
	
	/* Close */
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	closesocket(a_Socket->SockFD);
#else
	close(a_Socket->SockFD);
#endif
	
	/* Free it */
	Z_Free(a_Socket);
#else
	return;
#endif
}

/* IS_NetRecvWrap() -- Recieve data from remote end */
static size_t IS_NetRecvWrap(I_NetSocket_t* const a_Socket, I_HostAddress_t* const a_Host, void* const a_OutData, const size_t a_Len, const bool_t a_Peek, const bool_t a_CheckConn)
{
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	struct sockaddr_storage Addr;
	socklen_t SockLen;
	bool_t DisconSock;
	int32_t RetVal;
	size_t i;
	
	/* Check */
	if (!a_Socket || !a_OutData || !a_Len)
		return 0;
	
	/* Receive from socket */
	SockLen = sizeof(Addr);
	memset(&Addr, 0, sizeof(Addr));
	
	RetVal = recvfrom(a_Socket->SockFD, a_OutData, a_Len, __REMOOD_DONTWAITMSG | (a_Peek ? MSG_PEEK : 0), (struct sockaddr*)&Addr, &SockLen);
	
	// Error?
	if (RetVal < 0)
		return 0;
	
	// Convert address to host
	IS_ConvertHost(false, a_Host, &Addr, NULL);
	
	/* Return written bytes */
	return RetVal;
	
#else
	return 0;
#endif
}

/* I_NetReadyBytes() -- Determines amount of ready bytes */
size_t I_NetReadyBytes(I_NetSocket_t* const a_Socket, const size_t a_Bytes)
{
	int8_t* JunkBuf;
	
	/* Check */
	if (!a_Socket || !a_Bytes)
		return 0;
	
	/* Peek the message */
	JunkBuf = (int8_t*)alloca(a_Bytes);
	return IS_NetRecvWrap(a_Socket, NULL, JunkBuf, a_Bytes, true, true);
}

/* I_NetSend() -- Send data to remote host */
size_t I_NetSend(I_NetSocket_t* const a_Socket, const I_HostAddress_t* const a_Host, const void* const a_InData, const size_t a_Len)
{
#if __REMOOD_SOCKLEVEL == __REMOOD_SOCKPOSIX || __REMOOD_SOCKLEVEL == __REMOOD_SOCKBSD || __REMOOD_SOCKLEVEL == __REMOOD_SOCKWIN
	struct sockaddr_storage Addr;
	socklen_t SockLen;
	bool_t DisconSock;
	int32_t RetVal;
	size_t i;
	
	/* Check */
	if (!a_Socket || !a_InData || !a_Len)
		return 0;
	
	/* Wrong Host? */
	if (a_Host)
		if ((a_Host->IPvX == INIPVN_IPV6 && !(a_Socket->Flags & INSF_V6)) ||
			(a_Host->IPvX == INIPVN_IPV4 && (a_Socket->Flags & INSF_V6)) ||
			(a_Host->IPvX != INIPVN_IPV4 && a_Host->IPvX != INIPVN_IPV6)
			)
			return 0;
	
	/* Recieve from which socket? */
	// Convert address to host
	memset(&Addr, 0, sizeof(Addr));
	IS_ConvertHost(true, a_Host, &Addr, &SockLen);
	
	// On BSD systems, use convert hosts' SockLen because on Mac OS X
	// if SockLen != sizeof(struct sockaddr_in(6)) sendto will EINVAL
	// It seems Linux does not give a damn about the length however.
	//SockLen = sizeof(Addr);	// so just ignore it really then
	
	// Send data
	RetVal = sendto(a_Socket->SockFD, a_InData, a_Len, __REMOOD_DONTWAITMSG, (struct sockaddr*)&Addr, SockLen);
	
	// Error?
	if (RetVal < 0)
		return 0;
	
	/* Return written bytes */
	return RetVal;
#else
	return 0;
#endif
}

/* I_NetRecv() -- Sends data to remote host */
size_t I_NetRecv(I_NetSocket_t* const a_Socket, I_HostAddress_t* const a_Host, void* const a_OutData, const size_t a_Len)
{
	return IS_NetRecvWrap(a_Socket, a_Host, a_OutData, a_Len, false, true);
}

