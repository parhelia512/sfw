//Based on:

// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "inet_address.h"

#include <cstdio>
#include <cstring>

#ifdef _WIN32

#ifdef __GNUC__
#define GCCWIN
// Mingw / gcc on windows
//   #define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>

extern "C" {
WINSOCK_API_LINKAGE INT WSAAPI inet_pton(INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
WINSOCK_API_LINKAGE PCSTR WSAAPI inet_ntop(INT Family, PVOID pAddr, PSTR pStringBuf, size_t StringBufSize);
}
#else
// Windows...
#include <in6addr.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

struct in6__addruint {
	union {
		u_char Byte[16];
		u_short Word[8];
		uint32_t __s6_addr32[4];
	} uext;
};
#else
#include <netdb.h>
#include <netinet/tcp.h>
#include <strings.h> // memset
#endif

// INADDR_ANY use (type)value casting.
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_ANY;

//     /* Structure describing an Internet socket address.  */
//     struct sock_addrin {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

/*
#ifdef __linux__
#if !(__GNUC_PREREQ(4, 6))
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
#endif
*/

String InetAddress::to_ip_port() const {
	char buf[64] = "";
	uint16_t port = ntohs(_addr.sin_port);
	snprintf(buf, sizeof(buf), ":%u", port);

	return to_ip() + String(buf);
}

bool InetAddress::is_intranet_ip() const {
	if (_addr.sin_family == AF_INET) {
		uint32_t ip_addr = ntohl(_addr.sin_addr.s_addr);
		if ((ip_addr >= 0x0A000000 && ip_addr <= 0x0AFFFFFF) ||
				(ip_addr >= 0xAC100000 && ip_addr <= 0xAC1FFFFF) ||
				(ip_addr >= 0xC0A80000 && ip_addr <= 0xC0A8FFFF) ||
				ip_addr == 0x7f000001)

		{
			return true;
		}
	} else {
		auto addrP = ip6_net_endian();
		// Loopback ip
		if (*addrP == 0 && *(addrP + 1) == 0 && *(addrP + 2) == 0 &&
				ntohl(*(addrP + 3)) == 1)
			return true;
		// Privated ip is prefixed by FEC0::/10 or FE80::/10, need testing
		auto i32 = (ntohl(*addrP) & 0xffc00000);
		if (i32 == 0xfec00000 || i32 == 0xfe800000)
			return true;
		if (*addrP == 0 && *(addrP + 1) == 0 && ntohl(*(addrP + 2)) == 0xffff) {
			// the IPv6 version of an IPv4 IP address
			uint32_t ip_addr = ntohl(*(addrP + 3));
			if ((ip_addr >= 0x0A000000 && ip_addr <= 0x0AFFFFFF) ||
					(ip_addr >= 0xAC100000 && ip_addr <= 0xAC1FFFFF) ||
					(ip_addr >= 0xC0A80000 && ip_addr <= 0xC0A8FFFF) ||
					ip_addr == 0x7f000001)

			{
				return true;
			}
		}
	}
	return false;
}

bool InetAddress::is_loopback_ip() const {
	if (!is_ip_v6()) {
		uint32_t ip_addr = ntohl(_addr.sin_addr.s_addr);
		if (ip_addr == 0x7f000001) {
			return true;
		}
	} else {
		auto addrP = ip6_net_endian();
		if (*addrP == 0 && *(addrP + 1) == 0 && *(addrP + 2) == 0 &&
				ntohl(*(addrP + 3)) == 1)
			return true;
		// the IPv6 version of an IPv4 loopback address
		if (*addrP == 0 && *(addrP + 1) == 0 && ntohl(*(addrP + 2)) == 0xffff &&
				ntohl(*(addrP + 3)) == 0x7f000001)
			return true;
	}
	return false;
}

const struct sockaddr *InetAddress::get_sock_addr() const {
	return static_cast<const struct sockaddr *>((void *)(&_addr6));
}

void InetAddress::set_sock_addr_inet6(const struct sockaddr_in6 &addr6) {
	_addr6 = addr6;
	_is_ip_v6 = (_addr6.sin6_family == AF_INET6);
	_is_unspecified = false;
}

sa_family_t InetAddress::family() const {
	return _addr.sin_family;
}

String InetAddress::to_ip() const {
	char buf[64];
	if (_addr.sin_family == AF_INET) {
#if defined GCCWIN || (_MSC_VER && _MSC_VER >= 1900)
		::inet_ntop(AF_INET, (PVOID)&_addr.sin_addr, buf, sizeof(buf));
#else
		::inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf));
#endif
	} else if (_addr.sin_family == AF_INET6) {
#if defined GCCWIN || (_MSC_VER && _MSC_VER >= 1900)
		::inet_ntop(AF_INET6, (PVOID)&_addr6.sin6_addr, buf, sizeof(buf));
#else
		::inet_ntop(AF_INET6, &_addr6.sin6_addr, buf, sizeof(buf));
#endif
	}

	return buf;
}

uint32_t InetAddress::ip_net_endian() const {
	// assert(family() == AF_INET);
	return _addr.sin_addr.s_addr;
}

const uint32_t *InetAddress::ip6_net_endian() const {
// assert(family() == AF_INET6);
#if defined __linux__ || defined __HAIKU__
	return _addr6.sin6_addr.s6_addr32;
#elif defined _WIN32
	// TODO is this OK ?
	const struct in6__addruint *_addrtemp =
			reinterpret_cast<const struct in6__addruint *>(&_addr6.sin6_addr);
	return (*_addrtemp).uext.__s6_addr32;
#else
	return _addr6.sin6_addr.__u6_addr.__u6_addr32;
#endif
}

uint16_t InetAddress::port_net_endian() const {
	return _addr.sin_port;
}

void InetAddress::set_port_net_endian(uint16_t port) {
	_addr.sin_port = port;
}

inline bool InetAddress::is_unspecified() const {
	return _is_unspecified;
}

uint16_t InetAddress::to_port() const {
	return ntohs(port_net_endian());
}

bool InetAddress::is_ip_v6() const {
	return _is_ip_v6;
}

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) :
		_is_ip_v6(ipv6) {
	if (ipv6) {
		memset(&_addr6, 0, sizeof(_addr6));
		_addr6.sin6_family = AF_INET6;

		in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;

		_addr6.sin6_addr = ip;
		_addr6.sin6_port = htons(port);
	} else {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin_family = AF_INET;

		in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;

		_addr.sin_addr.s_addr = htonl(ip);
		_addr.sin_port = htons(port);
	}
	_is_unspecified = false;
}

InetAddress::InetAddress(const String &ip, uint16_t port, bool ipv6) :
		_is_ip_v6(ipv6) {
	if (ipv6) {
		memset(&_addr6, 0, sizeof(_addr6));
		_addr6.sin6_family = AF_INET6;
		_addr6.sin6_port = htons(port);

		if (::inet_pton(AF_INET6, ip.utf8().get_data(), &_addr6.sin6_addr) <= 0) {
			return;
		}
	} else {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(port);

		if (::inet_pton(AF_INET, ip.utf8().get_data(), &_addr.sin_addr) <= 0) {
			return;
		}
	}
	_is_unspecified = false;
}
