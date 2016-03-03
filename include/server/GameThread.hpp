#ifndef _GAME_THREAD_HPP_
#define _GAME_THREAD_HPP_

// std-C++ headers
#include <thread>
#include <atomic>
// WizardPoker headers
#include "server/Player.hpp"
#include "server/ClientInformations.hpp"
#include "common/Identifiers.hpp"  // userId
#include "server/ServerDatabase.hpp"
// SFML headers
#include <SFML/Network/TcpSocket.hpp>

class GameThread final : public std::thread
{
public:
	/// Constructor
	GameThread(ServerDatabase& database, userId player1ID, userId player2ID);

	/// Constructor starting a thread
	template <class F, class ...Args>
	explicit GameThread(ServerDatabase& database, userId player1ID, userId player2ID, F&& f, Args&&... args);

	/// Functions which stops the running thread (abortion)
	void interruptGame();

	void establishSockets(const ClientInformations& player1, const ClientInformations& player2);

	/// \TODO change return value to give the result of the game
	void startGame(const ClientInformations& player1, const ClientInformations& player2);

	/// Interface for Player and Card classes
	void applyEffect(Card* usedCard, EffectParamsCollection effect);

	/// Destructor
	~GameThread();

	const userId _player1ID;
	const userId _player2ID;

	//~Currently low for tests
	static constexpr std::chrono::seconds _turnTime{30};  // arbitrary

private:
	//////////// Attributes
	std::atomic_bool _running;
	sf::TcpSocket _socketPlayer1;
	sf::TcpSocket _socketPlayer2;
	sf::TcpSocket _specialOutputSocketPlayer1;
	sf::TcpSocket _specialOutputSocketPlayer2;
	Player _player1;
	Player _player2;

	ServerDatabase& _database;
	int _turn;
	Player *_activePlayer;
	Player *_passivePlayer;
	bool _turnCanEnd;

	std::thread _timerThread;

	std::atomic_bool _turnSwap;

	std::string _player1DeckName;
	std::string _player2DeckName;

	//////////// Private methods
	void setSocket(sf::TcpSocket& socket, sf::TcpSocket& specialSocket, const ClientInformations& player);

	void makeTimer();
	void receiveDecks();

	void createPlayers();
	void endTurn();
};

///////// template code

template <typename Function, class... Args>
GameThread::GameThread(ServerDatabase& database, userId player1ID, userId player2ID, Function&& function, Args&&... args):
	std::thread(function, args...),
	_player1ID(player1ID),
	_player2ID(player2ID),
	_running(true),
	_player1(_player1ID, _socketPlayer1, _specialOutputSocketPlayer1),
	_player2(_player2ID, _socketPlayer2, _specialOutputSocketPlayer2),
	_database(database),
	_turn(0),
	_turnCanEnd(false)
{
	createPlayers();
}

#endif  // _GAME_THREAD_HPP_
