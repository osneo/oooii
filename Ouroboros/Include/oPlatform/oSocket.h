// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Socket abstractions for TCP and UDP. Note: Server/Client should be used 
// together and Sender/Receiver should be used together only, any other 
// match-up will not work.
#pragma once
#ifndef oSocket_h
#define oSocket_h

#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oInterface.h>

// A Host is a simple identifier for a net device. This is an IP address
// for Internet connections. Use ouro::from_string to create a new oNetHost.
struct oNetHost
{
	oNetHost()
		: IP(0)
	{
	}
	bool operator==(oNetHost rhs) const { return (IP == rhs.IP); }
	bool operator!=(oNetHost rhs) const { return !(IP == rhs.IP); }
	bool operator<(oNetHost _Other) const { return IP < _Other.IP; }
	bool Valid() const { return 0 != IP; }

private:
	unsigned long IP;
};


// An Address is the combination of a Host and any data used to distinguish
// it from other connections from the same Host. In Internet terms this is an
// IP address and a port together. Use ouro::from_string to create a new oNetAddress
// in the form "<host>:<port>" ex: "google.com:http" or "127.0.0.1:11000".
struct oNetAddr
{
	oNetAddr()
		: Port(0)
	{}
	bool operator==(const oNetAddr& rhs) const { return (Host == rhs.Host && Port == rhs.Port); }
	bool operator<(const oNetAddr& _Other) const { return Host < _Other.Host || (Host == _Other.Host && Port < _Other.Port); }
	bool Valid() const { return Host.Valid(); }

	oNetHost Host;
private:
	unsigned short Port;
};

// In addition to ouro::from_string/ouro::to_string these helper functions aid in working with oNetAddr
oAPI void oSocketPortGet(const oNetAddr& _Addr, unsigned short* _pPort);
oAPI void oSocketPortSet(const unsigned short _Port, oNetAddr* _pAddr);
oAPI bool oSocketHostIsLocal(oNetHost _Host);

interface oSocketAsyncCallback;

interface oSocket : oInterface
{
	enum PROTOCOL
	{
		UDP,
		TCP,
	};

	enum STYLE
	{
		BLOCKING, // When calls to Send/SendTo/Recv return the operation has completed and the memory can be re-used
		ASYNC,    // When calls to Send/SendTo/Recv return the operation is queued up and the memory must not be touched until the supplied call back has fired
	};

	// For efficiency oSocket uses a size type that matches the underlying implementation
	typedef unsigned int size_t;

	struct BLOCKING_SETTINGS
	{
		BLOCKING_SETTINGS()
			: SendTimeout(ouro::infinite)
			, RecvTimeout(ouro::infinite)
		{}
		unsigned int SendTimeout;
		unsigned int RecvTimeout;
	};

	struct ASYNC_SETTINGS
	{
		ASYNC_SETTINGS()
			: MaxSimultaneousMessages(16)
		{}

		ouro::intrusive_ptr<threadsafe oSocketAsyncCallback> Callback;

		// The maximum number of messages that will be in flight in either direction,
		// this only has implications for Asynchronous sockets in that if more messages
		// than Max are in flight a stall will occur.  Increasing this increases memory usage
		// but allows for more messages to be in flight.
		oSocket::size_t MaxSimultaneousMessages;
	};

	struct DESC
	{
		DESC()
			: ConnectionTimeoutMS(500)
			, Protocol(TCP)
			, Style(BLOCKING)
		{}

		PROTOCOL Protocol;

		STYLE Style;

		// IP and port (i.e. "computername:1234") of a host computer. Calling 
		// GetDesc() will retain whatever is passed in here. GetHostname() and 
		// GetPeername() can be used to get the specifics of a socket at any given
		// time.
		oNetAddr Addr;

		// Settings specific to blocking sockets
		BLOCKING_SETTINGS BlockingSettings;

		// Settings specific to ASYNC sockets
		ASYNC_SETTINGS AsyncSettings;

		// How long the Create() function should wait before giving up on creation 
		// of this connection object
		oSocket::size_t ConnectionTimeoutMS;
	};

	// Failure cases: 
	// std::errc::operation_in_progress: Socket is already asynchronous
	// std::errc::invalid_argument: Socket is encrypted (only blocking sockets can be encrypted), no valid callback specified
	virtual bool GoAsynchronous(const oSocket::ASYNC_SETTINGS& _Settings) threadsafe = 0;

	// Failure cases: 
	// std::errc::invalid_argument: This can occur when a call to Send is made on a connectionless based protocol (i.e. UDP)
	// std::errc::no_buffer_space: This can occur when the MaxSimultaneousMessages has been exhausted
	// Send will send the provided header using the socket's protocol.  The user can optionally send a body as well that will
	// be linearized with the header on receive
	virtual bool Send(const void* _pHeader, oSocket::size_t _SizeHeader, const void* _pBody = nullptr, oSocket::size_t _SizeBody = ouro::invalid) threadsafe = 0;

	// Failure cases: 
	// std::errc::invalid_argument: This can occur when a call to SendTo is made on a connection based protocol (i.e. TCP)
	// std::errc::no_buffer_space: This can occur when the MaxSimultaneousMessages has been exhausted
	// SendTo will send the provided header using the socket's protocol.  The user can optionally send a body as well that will
	// be linearized with the header on receive
	virtual bool SendTo(const void* _pHeader, oSocket::size_t _SizeHeader, const oNetAddr& _Destination, const void* _pBody = nullptr, oSocket::size_t _SizeBody = 0) threadsafe = 0;

	// Queue up a pending receive. The RecvCallback will be called once the
	// passed in buffer has been filled with the data from a single packet.
	// This can fail if too many requests are outstanding
	virtual oSocket::size_t Recv(void* _pBuffer, oSocket::size_t _Size) threadsafe = 0;

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;

	// If false, this interface should be released. To get a more thorough error
	// message, a new interface should be created to attempt a reconnection, and 
	// if that fails oErrorGetLast() will have more robust reasoning as to why.
	virtual bool IsConnected() const threadsafe = 0;

	// Returns the host's ('this' computer's) full information including name, ip,
	// and port. Specify NULL for undesired fields from this API.
	virtual bool GetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe = 0;

	// Returns the peer's (the other computer's) full information including name, 
	// ip, and port. Specify NULL for undesired fields from this API.
	virtual bool GetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort) const threadsafe = 0;

	// Set the keep alive time and interval (in milliseconds) default time is 2 hours and 1 second interval
	virtual bool SetKeepAlive(unsigned int _TimeoutMS = 0x6DDD00, unsigned int _IntervalMS = 0x3E8) const threadsafe = 0;

	template<size_t hostnameSize> inline bool GetHostname(char (&_OutHostname)[hostnameSize]) const threadsafe { return GetHostname(_OutHostname, hostnameSize, 0, 0, 0, 0); }
	template<size_t peernameSize> inline bool GetPeername(char (&_OutPeername)[peernameSize]) const threadsafe { return GetPeername(_OutPeername, peernameSize, 0, 0, 0, 0); }
	template<size_t hostnameSize, size_t ipSize, size_t portSize> inline bool GetHostname(char (&_OutHostname)[hostnameSize], char (&_OutIPAddress)[ipSize], char (&_OutPort)[portSize]) const threadsafe { return GetHostname(_OutHostname, hostnameSize, _OutIPAddress, ipSize, _OutPort, portSize); }
	template<size_t peernameSize, size_t ipSize, size_t portSize> inline bool GetPeername(char (&_OutPeername)[peernameSize], char (&_OutIPAddress)[ipSize], char (&_OutPort)[portSize]) const threadsafe { return GetPeername(_OutPeername, peernameSize, _OutIPAddress, ipSize, _OutPort, portSize); }
};
// {68F693CE-B01B-4235-A401-787691707365}
oDEFINE_GUID_I(oSocket, 0x68f693ce, 0xb01b, 0x4235, 0xa4, 0x1, 0x78, 0x76, 0x91, 0x70, 0x73, 0x65);

oAPI bool oSocketCreate(const char* _DebugName, const oSocket::DESC& _Desc, threadsafe oSocket** _pSocket);

// Encrypted sockets work only in blocking mode, supporting all the same behavior
// as regular sockets, but with the additional ability to receive and send encrypted data
interface oSocketEncrypted : public oSocket
{
	virtual bool SendEncrypted(const void* _pData, oSocket::size_t _Size) threadsafe = 0;
	virtual oSocket::size_t RecvEncrypted(void* _pBuffer, oSocket::size_t _Size) threadsafe = 0;
};

oAPI bool oSocketEncryptedCreate(const char* _DebugName, const oSocket::DESC& _Desc, threadsafe oSocketEncrypted** _ppSocket);

interface oSocketAsyncCallback : oInterface
{
	// Called when new data arrives over the network.
	virtual void ProcessSocketReceive(void*_pData, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe = 0;

	// Called once a send operation has finished. A buffer passed in for
	// sending must remain available until this callback is sent.
	virtual void ProcessSocketSend(void* _pHeader, void* _pBody, oSocket::size_t _SizeData, const oNetAddr& _Addr, interface oSocket* _pSocket) threadsafe = 0;
};

// {EE38455C-A057-4b72-83D2-4E809FF1C059}
oDEFINE_GUID_I(oSocketServer, 0xee38455c, 0xa057, 0x4b72, 0x83, 0xd2, 0x4e, 0x80, 0x9f, 0xf1, 0xc0, 0x59);
interface oSocketServer : oInterface
{
	// Server-side of a reliable two-way communication pipe (TCP). The server 
	// merely listens for incoming connections. Use the oSocketClientAsync returned 
	// from WaitForConnection() to do communication to the remote client.

	struct DESC
	{
		DESC()
			: ListenPort(0)
			, MaxNumConnections(1)
		{}

		unsigned short ListenPort; // The port that the server listens for incoming connections on
		unsigned int MaxNumConnections; // Maximum number of connections this server will service
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;

	virtual bool GetHostname(char* _pString, size_t _strLen)  const threadsafe = 0;
	template<size_t hostnameSize> inline bool GetHostname(char (&_OutHostname)[hostnameSize]) const threadsafe { return GetHostname(_OutHostname, hostnameSize); }

	virtual bool WaitForConnection(const oSocket::BLOCKING_SETTINGS& _BlockingSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS = ouro::infinite) threadsafe = 0;
	virtual bool WaitForConnection(const oSocket::ASYNC_SETTINGS& _AsyncSettings, threadsafe oSocket** _ppNewlyConnectedClient, unsigned int _TimeoutMS = ouro::infinite) threadsafe = 0;
};

// {8809678d-a52e-4d0d-890b-bbaa315acbdd}
oDEFINE_GUID_I(oSocketServer2, 0x8809678d, 0xa52e, 0x4d0d, 0x89, 0x0b, 0xbb, 0xaa, 0x31, 0x5a, 0xcb, 0xdd);
interface oSocketServer2 : oInterface
{
	struct DESC
	{
		DESC()
			: ListenPort(0)
		{}

		unsigned short ListenPort; // The port that the server listens for incoming connections on 
		oSocket::BLOCKING_SETTINGS BlockingSettings; // All new connections will have these settings
		std::function< void(threadsafe oSocket* _pNewlyConnectedClient)> NewConnectionCallback;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual const char* GetDebugName() const threadsafe = 0;

	virtual bool GetHostname(char* _pString, size_t _strLen)  const threadsafe = 0;
	template<size_t hostnameSize> inline bool GetHostname(char (&_OutHostname)[hostnameSize]) const threadsafe { return GetHostname(_OutHostname, hostnameSize); }
};

//Note that callbacks can start before this function even returns.
oAPI bool oSocketServer2Create(const char* _DebugName, const oSocketServer2::DESC& _Desc, threadsafe oSocketServer2** _ppSocketServer);

// Enumerate the addresses of all cmd
oAPI void oSocketEnumerateAllAddress(std::function<void(oNetAddr _Addr)> _Enumerator);

#endif
