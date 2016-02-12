#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/System.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>
#include <atomic>
#include <common/sockets/TransferType.hpp>

static void input(sf::TcpSocket *inputSocket, std::atomic_bool *wait);
static inline sf::Packet formatOutputMessage(std::string message);

static void input(sf::TcpSocket *inputSocket, std::atomic_bool *wait)
{
	sf::TcpSocket& in = *inputSocket;  // create reference to simplify reading
	in.setBlocking(false);  // set the socket as non-blocking so that it verifies that connection is still available
	sf::Packet packet;
	while(wait->load())
	{
		sf::Socket::Status receivedStatus = in.receive(packet);
		if(receivedStatus == sf::Socket::Done)
		{
			TransferType type;
			std::string message;
			packet >> type;
			if(!(type == TransferType::CHAT_MESSAGE))
			{
				std::cout << "Wrong transfer\n";
			}
			else
			{
				packet >> message;
				std::cout << message << std::endl;
			}
		}
		else if(receivedStatus == sf::Socket::NotReady)
		{
			sf::sleep(sf::milliseconds(250));
		}
	}
}

static inline sf::Packet formatOutputMessage(std::string message)
{
	sf::Packet packet;
	packet << TransferType::CHAT_MESSAGE << message;
	return packet;
}

int main(int argc, char **argv)
{
	if(argc != 6)
		return EXIT_FAILURE;
	/* role = "caller" if it must ask the server to tell the other player to connect
	and role = "callee" if it must make a direct connection to the host */
	std::string role = argv[1];
	sf::IpAddress address = argv[2];
	sf::Uint16 gameServerPort;
	std::stringstream(argv[3]) >> gameServerPort;
	std::string selfName = argv[4];
	std::string otherName = argv[5];
	std::atomic_bool running(true);

	sf::Packet packet;  // used to exchange data (I/O)
	sf::TcpSocket out, in;  // two separated sockets for IO operations
	sf::TcpListener listener;  // listener helping to get the entring connection
	sf::Uint16 calleeListeningPort;  // port which is asked by the out (alias `server`) socket
	/* the listener is set as blocking because it is waiting for a precise
	amount of connections that are well defined */
	listener.setBlocking(true);
	listener.listen(sf::Socket::AnyPort);
	if(role == "caller")
	{
		// out here represents the game server
		out.connect(address, gameServerPort);
		// the connection to the server is blocking because of the several delays possible
		out.setBlocking(true);
		// transfer in the right order the necessary data to the server
		packet << TransferType::CHAT_PLAYER_IP << selfName << otherName << listener.getLocalPort();
		out.send(packet);
		// get back the other player's address
		out.receive(packet);
		std::string calleeAddress;
		packet >> calleeAddress;
		// create the first socket (receiving data from the other player
		listener.accept(in);
		// get the port use to send data to the other player
		in.receive(packet);
		packet >> calleeListeningPort;
		sf::sleep(sf::seconds(0.05));
		// and finally create the second socket (sending data)
		out.disconnect();
		if(out.connect(in.getRemoteAddress(), calleeListeningPort) != sf::Socket::Done)
			std::cerr << "\tUnable to connect to called friend\n";
		// here, the users can start to discuss
		std::cout << "You are starting a discussion (as " << selfName << ") with " << otherName << std::endl;
	}
	else if(role == "callee")
	{
		// if player is the callee, everything is already done in the chatListening function
		out.connect(address, gameServerPort);
		packet << listener.getLocalPort();
		out.send(packet);
		listener.accept(in);
		std::cout << otherName << " wants to start a discussion with you" << std::endl;
	}
	else
	{
		std::cerr << "unknown role: " << role;
		return EXIT_FAILURE;
	}
	std::thread inputThread(input, &in, &running);
	// Warning: the `in` socket may not be used in this thread anymore!
	packet = formatOutputMessage("Hi " + otherName + ", I am " + selfName);
	out.send(packet);
	int n;
	std::cin >> n;
	running.store(false);
	inputThread.join();
	return EXIT_SUCCESS;
}
