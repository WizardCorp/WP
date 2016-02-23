// SFML headers
#include <SFML/Network/IpAddress.hpp>
#include <SFML/System/Sleep.hpp>
// WizardPoker headers
#include "server/Server.hpp"
#include "common/constants.hpp"
#include "server/ErrorCode.hpp"
#include "common/sockets/TransferType.hpp"
#include "common/sockets/PacketOverload.hpp"
#include "common/PasswordHasher.hpp"
// std-C++ headers
#include <iostream>
#include <algorithm>

Server::Server():
	_clients(),
	_listener(),
	_socketSelector(),
	_done(false),
	_threadRunning(false),
	_quitThread(),
	_waitingPlayer(),
	_isAPlayerWaiting(false),
	_quitPrompt(":QUIT")
{

}

int Server::start(const sf::Uint16 listenerPort)
{
	if(_listener.listen(listenerPort) != sf::Socket::Done)
		return UNABLE_TO_LISTEN;
	_quitThread = std::thread(&Server::waitQuit, this);
	_threadRunning.store(true);
	sf::sleep(SOCKET_TIME_SLEEP);
	_socketSelector.add(_listener);
	while(!_done.load())
	{
		// if no socket is ready, wait again
		if(!_socketSelector.wait(sf::milliseconds(50)))
			continue;
		// if listener is ready, then a new connection is incoming
		if(_socketSelector.isReady(_listener))
			takeConnection();
		else  // one of the client sockets has received something
			receiveData();
	}
	return SUCCESS;
}

void Server::takeConnection()
{
	std::unique_ptr<sf::TcpSocket> newClient{new sf::TcpSocket()};
	// if listener can't accept correctly, free the allocated socket
	if(_listener.accept(*newClient) != sf::Socket::Done)
	{
		std::cout << "Error when trying to accept a new client.\n";
		return;
	}
	// receive username
	sf::Packet packet;
	newClient->receive(packet);
	TransferType type;
	packet >> type;
	if(type == TransferType::GAME_CONNECTION)
		connectUser(packet, std::move(newClient));
	else if(type == TransferType::GAME_REGISTERING)
		registerUser(packet, std::move(newClient));
	else if(type == TransferType::CHAT_PLAYER_IP)
		handleChatRequest(packet, std::move(newClient));
	else
		std::cout << "Error: wrong code!" << std::endl;
}

// TODO these methods (connect/register) should be rewritten in many smaller methods
void Server::connectUser(sf::Packet& connectionPacket, std::unique_ptr<sf::TcpSocket> client)
{
	std::string playerName, password;
	sf::Uint16 clientPort;
	connectionPacket >> playerName >> password >> clientPort;

	bool alreadyConnected{_clients.find(playerName) != _clients.end()};
	// TODO check in database for the identifiers validity
	bool wrongIdentifiers{false};
	while(alreadyConnected or wrongIdentifiers)
	{
		// Send a response indicating the error
		connectionPacket.clear();
		if(alreadyConnected)
		{
			std::cout << playerName << " tried to connect to the server but is already connected.\n";
			connectionPacket << TransferType::GAME_ALREADY_CONNECTED;
		}
		else
		{
			std::cout << playerName << " gives wrong identifiers when trying to connect.\n";
			connectionPacket << TransferType::GAME_WRONG_IDENTIFIERS;
		}
		client->send(connectionPacket);

		// Receive the new try of the client
		client->receive(connectionPacket);
		TransferType type;
		connectionPacket >> type;
		if(type != TransferType::GAME_CONNECTION)
		{
			std::cout << "Error: wrong packet transmitted after failed connection (expecting another connection packet).\n";
			return;
		}
		connectionPacket >> playerName >> password >> clientPort;

		alreadyConnected = _clients.find(playerName) != _clients.end();
		// TODO check in database for the identifiers validity
		wrongIdentifiers = false;
	}
	std::cout << "New player connected: " << playerName << std::endl;
	sendAcknowledgement(*client);
	// add this client to the selector so that its receivals are handled properly
	// (be sure that this line is before the next, otherwise we get a segfault)
	_socketSelector.add(*client);
	// and add the new socket to the clients
	_clients[playerName] = {std::move(client), clientPort};
}

void Server::registerUser(sf::Packet& registeringPacket, std::unique_ptr<sf::TcpSocket> client)
{
	std::string playerName, password;
	registeringPacket >> playerName >> password;

	// TODO check in database for the identifiers validity
	bool userNameNotAvailable{false};
	bool failedToRegister{false};
	while(userNameNotAvailable or failedToRegister)
	{
		// Send a response indicating the error
		registeringPacket.clear();
		if(userNameNotAvailable)
		{
			std::cout << playerName << " tried to register to the server but the name is not available.\n";
			registeringPacket << TransferType::GAME_USERNAME_NOT_AVAILABLE;
		}
		else
		{
			std::cout << playerName << " tried to register to the server but an error occurred.\n";
			registeringPacket << TransferType::GAME_FAILED_TO_REGISTER;
		}
		client->send(registeringPacket);

		// Receive the new try of the client
		client->receive(registeringPacket);
		TransferType type;
		registeringPacket >> type;
		if(type != TransferType::GAME_REGISTERING)
		{
			std::cout << "Error: wrong packet transmitted after failed registering (expecting another registering packet).\n";
			return;
		}
		registeringPacket >> playerName >> password;

		// TODO check in database for the identifiers validity
		userNameNotAvailable = false;
		failedToRegister = false;
	}
	// TODO register the user into the database
	sendAcknowledgement(*client);
	std::cout << "New player registered: " << playerName << std::endl;
}
void Server::sendAcknowledgement(sf::TcpSocket& client)
{
	sf::Packet packet;
	packet << TransferType::GAME_CONNECTION_OR_REGISTERING_OK;
	client.send(packet);
}

void Server::receiveData()
{
	std::cout << "Data received.\n";
	// first find which socket it is
	auto it = std::find_if(_clients.begin(), _clients.end(), [this](const auto& pair)
	{
		return _socketSelector.isReady(*(pair.second.socket));
	});
	if(it == _clients.end())
	{
		std::cerr << "Unable to find which socket is ready" << std::endl;
		return;
	}

	sf::Packet packet;
	sf::TcpSocket& client(*(it->second.socket));
	sf::Socket::Status receivalStatus = client.receive(packet);
	if(receivalStatus == sf::Socket::Done)
	{
		TransferType type;
		packet >> type;
		switch(type)
		{
		case TransferType::PLAYER_DISCONNECTION:
			std::cout << "Player " << it->first << " quits the game!" << std::endl;
			removeClient(it);
			break;
		// Friendship management
		case TransferType::PLAYER_CHECK_CONNECTION:
			checkPresence(it, packet);
			break;
		case TransferType::PLAYER_ASKS_FRIENDS:
			sendFriends(it);
			break;
		case TransferType::PLAYER_NEW_FRIEND:
			handleFriendshipRequest(it, packet);
			break;
		case TransferType::PLAYER_REMOVE_FRIEND:
			handleRemoveFriend(it, packet);
			break;
		case TransferType::PLAYER_RESPONSE_FRIEND_REQUEST:
			handleFriendshipRequestResponse(it, packet);
			break;
		case TransferType::PLAYER_GETTING_FRIEND_REQUESTS_STATE:
			sendFriendshipRequestsState(it);
			break;
		case TransferType::PLAYER_GETTING_FRIEND_REQUESTS:
			sendFriendshipRequests(it);
			break;
		// Game management
		case TransferType::GAME_REQUEST:
			findOpponent(it);
			break;
		default:
			std::cerr << "Error: unknown code " << static_cast<sf::Uint32>(type) << std::endl;
			break;
		}
	}
	else if(receivalStatus == sf::Socket::Disconnected)
	{
		std::cerr << "Connection with player " << it->first << " is lost: forced disconnection from server." << std::endl;
		removeClient(it);
	}
	else
		std::cerr << "data not well received." << std::endl;
}

void Server::removeClient(const _iterator& it)
{
	// remove from the selector so it won't receive data anymore
	_socketSelector.remove(*(it->second.socket));
	// remove from the map
	_clients.erase(it);
}

void Server::checkPresence(const _iterator& it, sf::Packet& transmission)
{
	sf::Packet packet;
	std::string nameToCheck;
	transmission >> nameToCheck;
	packet << TransferType::PLAYER_CHECK_CONNECTION << (_clients.find(nameToCheck) != _clients.end());
	it->second.socket->send(packet);
}

void Server::quit()
{
	// in case the method is called even though server has not been manually ended
	_done.store(true);
	_quitThread.join();
	_threadRunning.store(false);
	_socketSelector.clear();
	_clients.clear();
}

Server::~Server()
{
	quit();
}

void Server::waitQuit()
{
	std::cout << "Type '" << _quitPrompt << "' to end the server" << std::endl;
	std::string input;
	while(!_done.load())
	{
		std::cin >> input;
		if(input == _quitPrompt)
			_done.store(true);
	}
	_threadRunning.store(false);
	std::cout << "ending server..." << std::endl;
}



// Game management

void Server::findOpponent(const _iterator& it)
{
	if(!_isAPlayerWaiting)
	{
		_isAPlayerWaiting = true;
		_waitingPlayer = it->first;
	}
	else
	{
		const auto& waitingPlayer = _clients.find(_waitingPlayer);
		// if waiting player is not present anymore
		if(waitingPlayer == _clients.end())
			_waitingPlayer = it->first;
		else
		{
			sf::Packet toFirst, toSecond;
			toFirst << it->first;
			toSecond << _waitingPlayer;
			it->second.socket->send(toSecond);
			waitingPlayer->second.socket->send(toFirst);
			_isAPlayerWaiting = false;
		}
	}
}

// Friends management

void Server::handleChatRequest(sf::Packet& packet, std::unique_ptr<sf::TcpSocket> client)
{
	sf::Packet responseToCaller;
	responseToCaller << TransferType::CHAT_PLAYER_IP;
	std::string calleeName;
	std::string callerName;
	sf::Uint16 callerPort;
	packet >> callerName >> calleeName >> callerPort;
	std::cout << "start request\n";
	// first of all, verify that player exist
	// if it does, send his IP
	auto callee = _clients.find(calleeName);
	if(callee == _clients.end())
	{
		std::cout << "player does not exist!\n";
		responseToCaller << static_cast<sf::Uint32>(0);
	}
	else
	{
		responseToCaller << _clients[calleeName].socket->getRemoteAddress().toInteger();
		sf::Packet packetToCalle;
		sf::TcpSocket toCallee;
		std::cout << "address is " << _clients[calleeName].socket->getRemoteAddress() << " and port is " << _clients[calleeName].listeningPort << std::endl;
		if(toCallee.connect(_clients[calleeName].socket->getRemoteAddress(), _clients[calleeName].listeningPort) != sf::Socket::Done)
			std::cerr << "Unable to connect to calle (" << calleeName << ")\n";
		else
		{
			std::cout << "connected\n";
			// as the listening port has a delay, set communications as blokcing
			toCallee.setBlocking(true);
			packetToCalle << client->getRemoteAddress().toInteger() << callerPort << calleeName << callerName;
			std::cout << "sending to callee\n";
			if(toCallee.send(packetToCalle) != sf::Socket::Done)
				std::cerr << "unable to send to calle\n";
			else
				std::cout << "sent to callee (" << calleeName << ")\n";
		}
	}
	client->send(responseToCaller);
}

void Server::handleFriendshipRequest(const _iterator& it, sf::Packet& transmission)
{
	sf::Packet response;
	std::string friendName;
	transmission >> friendName;
	/// \TODO check database for existence of the user friend
	bool friendExists{true};
	if(not friendExists)
		// Send an error to the user
		response << TransferType::NOT_EXISTING_FRIEND;
	else
	{
		/// \TODO Add the user friendName to the friendship requests list of the user it
		it->second.friendshipRequests.push_back(friendName);
		// Send an acknowledgement to the user
		response << TransferType::PLAYER_NEW_FRIEND;
		/// \TODO Add the user it to the external requests list of the user friendName
	}
	it->second.socket->send(response);
}

void Server::handleFriendshipRequestResponse(const _iterator& it, sf::Packet& transmission)
{
	bool accepted;
	std::string name;
	transmission >> name >> accepted;
	const auto& asker = _clients.find(name);
	/// \TODO check if the other user is in the external requests list
	bool otherUserInExternalRequestList{true};
	if(not otherUserInExternalRequestList)
	{
		sf::Packet response;
		response << TransferType::NOT_EXISTING_FRIEND;
		it->second.socket->send(response);
		return;
	}
	/// \TODO remove the other user from the external requests list of the user it
	auto& extRequests = it->second.externalRequests;
	extRequests.erase(std::find(extRequests.begin(), extRequests.end(), name));
	/// \TODO remove the user it from the friendship requests list of the other user
	auto& friendRequests = asker->second.friendshipRequests;
	friendRequests.erase(std::find(friendRequests.begin(), friendRequests.end(), it->first));
	/// \TODO add the user it to the right list of the other user
	(accepted ? asker->second.acceptedRequests : asker->second.refusedRequests).push_back(it->first);
	// acknowledge to client
	sf::Packet response;
	response << TransferType::PLAYER_RESPONSE_FRIEND_REQUEST;
	it->second.socket->send(response);
}

void Server::sendFriendshipRequests(const _iterator& it)
{
	sf::Packet response;
	/// \TODO get the external requests list of the user it
	response << TransferType::PLAYER_GETTING_FRIEND_REQUESTS << it->second.externalRequests;
	it->second.socket->send(response);
}

void Server::sendFriendshipRequestsState(const _iterator& it)
{
	sf::Packet response;
	/// \TODO get the accepted and refused requests of the user it
	response << TransferType::PLAYER_GETTING_FRIEND_REQUESTS_STATE
	         << it->second.acceptedRequests
	         << it->second.refusedRequests;
	it->second.socket->send(response);
	/// \TODO clear the accepted and refused requests of the user it
	it->second.acceptedRequests.clear();
	it->second.refusedRequests.clear();
}

void Server::sendFriends(const _iterator& it)
{
	std::vector<std::string> friends;
	/// \TODO get the friends list of the user it
	sf::Packet packet;
	packet << friends;
	it->second.socket->send(packet);
}

void Server::handleRemoveFriend(const _iterator& it, sf::Packet& transmission)
{
	std::string removedFriend;
	transmission >> removedFriend;
	/// \TODO remove removedFriend user from the friend list of the user it
}
