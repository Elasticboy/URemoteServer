#include "Server.h"

#include <sstream>
#include <iostream>
#include <comdef.h>

#include "Exchange.h"
#include "MasterVolume.h"
#include "Keyboard.h"
#include "App.h"

int Server::s_InstanceCount = 0;

//////////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////////

Server::Server(int _port, int _maxConnections)
{
	m_Port = _port;
	m_MaxConcurrentConnections = _maxConnections;
	InitServer();
}

Server::~Server(void)
{
	FreeServer();
}

//! Lancement du serveur
bool Server::Launch()
{
	m_ContinueToListen = true;

	// Socket d'acceptation
	//SOCKADDR_IN csin;
	//int sizeofcsin = sizeof(csin);
	char buffer[BUFSIZ];

	/* connection socket */
	
	while (m_ContinueToListen) {
		cout << "Hostname : " << m_Hostname << endl;
		cout << "IP Address : " << m_IpAddress << endl;
		cout << "Open port : " << m_Port << endl;
		cout << "Waiting for client to connect..." << endl;
		
		//m_CSocket = accept(m_ListenSocket, (SOCKADDR *)&csin, &sizeofcsin);
		m_CSocket = accept(m_ListenSocket, NULL, NULL);
		if (m_CSocket == INVALID_SOCKET) {
			cerr << "accept failed with error: " << WSAGetLastError() << endl;
			FreeServer();
			return false;
		}
		
		memset(buffer, '\0', sizeof(buffer)); // On vide le buffer
		//memset(buffer, '\0', BUFSIZ); // On vide le buffer
		int res = recv(m_CSocket, buffer, sizeof(buffer), 0);
		//int res = recv(m_CSocket, buffer, BUFSIZ, 0);
		cout << "  -- result" << res <<  endl;
		//int getpeername(int sockfd, struct sockaddr *addr, int *addrlen);
		//int gethostname(char *hostname, size_t size);
		cout << buffer << endl;
		string message = buffer;
		HandleMessage(message);

		closesocket(m_CSocket);
		cout << "Socket closed" << endl << endl;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////////

//! Initialize the server
bool Server::InitServer() 
{
	// Initialize winSock library (v2.0)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,0), &wsaData) != NO_ERROR) {
		cerr << "WSAStartup failed with error: " << WSAGetLastError() << endl;
		WSACleanup();
		return false;
	}
	
	s_InstanceCount++;

	// Create listener socket for incoming connections
	m_ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_ListenSocket == INVALID_SOCKET) {
		cerr << "socket failed with error: " << WSAGetLastError() << endl;
		FreeServer();
		return false;
	}

	// Socket thecnical info
	SOCKADDR_IN socketAddress; 
	socketAddress.sin_addr.s_addr	= htonl(INADDR_ANY); // Server address
	socketAddress.sin_family		= AF_INET; // Type of socket => Internet
	socketAddress.sin_port			= htons(m_Port);

	// Bind the socket to the address and port defined in SOCKADDR
	if (bind(m_ListenSocket, (SOCKADDR*)&socketAddress, sizeof(socketAddress)) == SOCKET_ERROR) {
		cerr << "bind failed with error: " << WSAGetLastError() << endl;
		FreeServer();
		return false;
	}

	// Listen to incoming connections
	if (listen(m_ListenSocket, m_MaxConcurrentConnections) == SOCKET_ERROR) {
		cerr << "listen failed with error: " << WSAGetLastError() << endl;
		FreeServer();
		return false;
	}

	m_Hostname = GetHostName();
	m_IpAddress = GetIpAddress(m_Hostname);

	return true;
}

//! Lib�re les sockets
void Server::FreeServer()
{
	closesocket(m_ListenSocket);

	if (s_InstanceCount <= 1)
		WSACleanup();
}


string GetHostName()
{
	char hostname[80];
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
		cerr << "Error " << WSAGetLastError() << " when getting local host name." << endl;
		return "";
	}

	return hostname;
}

string GetIpAddress(string _hostname)
{
	string ipAddress = "";
	struct hostent *host = gethostbyname(_hostname.c_str());
	if (host == 0) {
		cerr << "Yow! Bad host lookup." << endl;
	}

	/*
	for (int i = 0; host->h_addr_list[i] != 0; ++i) {
		struct in_addr addr;
		memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
		cout << "Address " << i << ": " << inet_ntoa(addr) << endl;
	}
	*/
	struct in_addr addr;
	memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
	ipAddress = inet_ntoa(addr);

	return ipAddress;
}

//! Traitement de la commande envoy�e par le client
void Server::HandleMessage(string _msg) 
{
	string serializedReply = Exchange::HandleMessage(_msg, m_ContinueToListen);
	Reply(serializedReply);

	
	switch (request.type()) {
	
	case Request_Type_SIMPLE:
		Reply(ClassicCommand(requestCode));
		break;
		
	case Request_Type_KEYBOARD:
		Reply(Keyboard::Command(param));
		Reply(Keyboard::Combo(param));
		break;

	case Request_Type_MEDIA:
		Reply(Keyboard::MediaCommand(param));
		break;

	case Request_Type_AI:
		Reply(AICommand(requestCode));
		break;
		
	case Request_Type_VOLUME:
		Reply(VolumeCommand(requestCode));
		break;

	// Commande des applications
	case Request_Type_APP:
		Reply(AppCommand(param));
		break;

	default:
		if (_msg == "\0") {
			reply << "No command had been receive";
		} else {
			reply << "Unknown code received !";
		}
	}

	Reply(reply.str());
}

//! Envoie de la r�ponse au client
void Server::Reply(string _message)
{
	if (_message.empty()) {
		cout << "Server::Reply : Message is empty." << endl;
		return;
	}
	send(m_CSocket, _message.c_str(), strlen(_message.c_str()), 0);
	cout << _message << endl;
}