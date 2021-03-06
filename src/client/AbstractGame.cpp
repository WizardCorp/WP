// std-C++ headers
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <limits>
// SFML headers
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/SocketSelector.hpp>
// WizardPoker headers
#include "common/constants.hpp"
#include "client/sockets/Client.hpp"
#include "common/sockets/TransferType.hpp"
#include "common/sockets/PacketOverload.hpp"
#include "common/sockets/EndGame.hpp"
#include "client/NonBlockingInput.hpp"
#include "client/AbstractGame.hpp"

AbstractGame::AbstractGame(Client& client):
	_client{client},
	_playing(false)
{
	_client.waitTillReadyToPlay();
	_playing.store(true);
}

void AbstractGame::init()
{
	initListening();
	chooseDeck();
	sf::Packet packet;

	// receive in game data
	_client.getGameSocket().receive(packet);
	handlePacket(packet);

	// receive turn informations
	_client.getGameSocket().receive(packet);
	TransferType type;
	packet >> type;
	if(type != TransferType::GAME_STARTING)
		throw std::runtime_error("Wrong signal received: " + std::to_string(static_cast<sf::Uint32>(type)));
	packet >> type;
	if(type == TransferType::GAME_PLAYER_ENTER_TURN)
		_myTurn.store(true);
	else if(type == TransferType::GAME_PLAYER_LEAVE_TURN)
	{
		_myTurn.store(false);
		displayGame();
		displayMessage("Your opponent begins, wait for your turn...");
	}
	else
		throw std::runtime_error("Wrong turn information: " + std::to_string(static_cast<sf::Uint32>(type)));
}

void AbstractGame::play()
{
	while(_playing.load())
	{
		if(_myTurn.load())
			startTurn();
		else
			waitUntil([this]()
			{
				return _myTurn.load();
			});
	}
}

void AbstractGame::waitUntil(std::function<bool()> booleanFunction)
{
	while(not booleanFunction())
		sf::sleep(sf::milliseconds(100));
}

void AbstractGame::startTurn()
{
	_myTurn.store(true);
	displayGame();
}

void AbstractGame::sendDeck(const std::string& deckName)
{
	sf::Packet deckNamePacket;
	deckNamePacket << TransferType::GAME_PLAYER_GIVE_DECK_NAMES << deckName;
	_client.getGameSocket().send(deckNamePacket);
}

//PRIVATE METHODS

//User actions
void AbstractGame::useCard(int cardIndex)
{
	if (not _myTurn)
	{
		displayMessage("You must wait your turn!");
		return;
	}
	sf::Packet actionPacket;
	actionPacket << TransferType::GAME_USE_CARD << static_cast<sf::Int32>(cardIndex);
	_client.getGameSocket().send(actionPacket);
	// receive amount of selection to make
	_client.getGameSocket().receive(actionPacket);
	TransferType responseHeader;
	actionPacket >> responseHeader;
	if(handleHeader(responseHeader))
		return;
	assert(responseHeader == TransferType::GAME_SEND_NB_OF_EFFECTS);
	sf::Uint32 nbOfEffects;
	actionPacket >> nbOfEffects;
	for(sf::Uint32 i{0}; i < nbOfEffects; ++i)
	{
		_client.getGameSocket().receive(actionPacket);
		// ask and send the additionnal inputs to the server
		if(not treatAdditionnalInputs(actionPacket))
			return;
	}
	// receive status of operation from server
	_client.getGameSocket().receive(actionPacket);
	actionPacket >> responseHeader;
	switch(responseHeader)
	{
	case TransferType::ACKNOWLEDGE:
		displayMessage("The card has been successfuly used.");
		break;

	// This line is more documentative than really needed
	case TransferType::FAILURE:
	default:
		displayMessage("An error occured when using a card.");
		break;
	}
}

bool AbstractGame::treatAdditionnalInputs(sf::Packet& actionPacket)
{
	TransferType responseHeader;
	actionPacket >> responseHeader;
	switch(responseHeader)
	{
	case TransferType::FAILURE:
		displayMessage("No card to select, unable to play this card!");
		return false;

	case TransferType::GAME_NOT_ENOUGH_ENERGY:
		displayMessage("You have not enough energy to use this card.");
		return false;

	case TransferType::GAME_CARD_LIMIT_TURN_REACHED:
		displayMessage("You can't use more cards, the limit is reached.");
		return false;

	// note the case here so that ACKNOWLEDGE is not taken as `default`
	// but the code for ACKNOWLEDGE is after the switch
	case TransferType::ACKNOWLEDGE:
		break;

	default:
		std::cerr << "Unexpected value: " << static_cast<sf::Uint32>(responseHeader) << std::endl;
		return false;
	}

	assert(responseHeader == TransferType::ACKNOWLEDGE);
	std::vector<CardToSelect> requiredInputs;
	actionPacket >> requiredInputs;

	std::vector<sf::Uint32> indices;
	indices.reserve(requiredInputs.size());
	for(const auto& input : requiredInputs)
	{
		switch(input)
		{
		case CardToSelect::SELF_BOARD:
			indices.push_back(askSelfBoardIndex());
			break;

		case CardToSelect::OPPO_BOARD:
			indices.push_back(askOppoBoardIndex());
			break;

		case CardToSelect::SELF_HAND:
			indices.push_back(askSelfHandIndex());
			break;

		/*case CardToSelect::OPPO_HAND:
			indices.push_back(askOppoHandIndex());
			break;

		case CardToSelect::SELF_GRAVEYARD:
			indices.push_back(askSelfGraveyardIndex());
			break;*/

		default:
			std::cerr << "ERROR: UNKNOWN VALUE: " << static_cast<std::underlying_type<CardToSelect>::type>(input) << std::endl;
			break;
		}
	}
	sf::Packet indicesPacket;
	indicesPacket << indices;
	_client.getGameSocket().send(indicesPacket);
	return true;
}

void AbstractGame::attackWithCreature(int selfCardIndex, bool attackOpponent, int opponentCardIndex)
{
	if (not _myTurn)
	{
		displayMessage("You must wait your turn!");
		return;
	}
	else
	{
		// If there's cards on board
		if(selfCardIndex >= 0 and (opponentCardIndex >= 0 or attackOpponent))
		{
			if(attackOpponent)
				opponentCardIndex = -1;
			sf::Packet actionPacket;
			actionPacket << TransferType::GAME_ATTACK_WITH_CREATURE
			             << static_cast<sf::Int32>(selfCardIndex)
			             << static_cast<sf::Int32>(opponentCardIndex);
			_client.getGameSocket().send(actionPacket);
			_client.getGameSocket().receive(actionPacket);
			TransferType responseHeader;
			actionPacket >> responseHeader;
			switch(responseHeader)
			{
			case TransferType::ACKNOWLEDGE:
				displayMessage("The monster successfuly attacked.");
				break;

			case TransferType::GAME_NOT_ENOUGH_ENERGY:
				displayMessage("You have not enough energy to attack with this monster.");
				break;

			// This line is more documentative than really needed
			case TransferType::FAILURE:
			default:
				displayMessage("An error occured when attacking with this monster.");
				break;
			}
		}
		else
			displayMessage("There is no card to choose, please do something else.");
	}
}


void AbstractGame::endTurn()
{
	if (not _myTurn)
	{
		displayMessage("You must wait your turn!\n");
		return;
	}
	else
	{
		sf::Packet actionPacket;
		actionPacket << TransferType::GAME_PLAYER_LEAVE_TURN;
		_client.getGameSocket().send(actionPacket);
		_myTurn = false;
	}
}

void AbstractGame::quit()
{
	// send QUIT message to server
	sf::Packet actionPacket;
	actionPacket << TransferType::GAME_QUIT_GAME;
	_client.getGameSocket().send(actionPacket);
	// internal ending
	_playing.store(false);
	_myTurn.store(false);
	if(_listeningThread.joinable())
		_listeningThread.join();
}

AbstractGame::~AbstractGame()
{
	quit();
	_client.endGame();
}

//////////////// special data receival

void AbstractGame::initListening()
{
	_listeningThread = std::thread(&AbstractGame::inputListening, this);
}

void AbstractGame::inputListening()
{
	onListeningThreadCreation();
	sf::TcpSocket& listeningSocket{_client.getGameListeningSocket()};
	_client.waitTillReadyToPlay();
	sf::Packet receivedPacket;
	sf::SocketSelector selector;
	selector.add(listeningSocket);
	while(_playing.load())
	{
		if(!selector.wait(SOCKET_TIME_SLEEP))
			continue;
		assert(selector.isReady(listeningSocket));
		auto receiveStatus{listeningSocket.receive(receivedPacket)};
		if(receiveStatus == sf::Socket::Disconnected)
		{
			std::cerr << "Connection lost with the server\n";
			_playing.store(false);
			break;
		}
		else if(receiveStatus == sf::Socket::Error)
		{
			std::cerr << "Error while transmitting data\n";
			_playing.store(false);
			break;
		}
		else
		{
			handlePacket(receivedPacket);
			updateDisplay();
		}
	}
}

void AbstractGame::onListeningThreadCreation()
{
	// Enpty body to be overriden
}

void AbstractGame::endGame(sf::Packet& transmission)
{
	EndGame endGameInfo;
	transmission >> endGameInfo;
	if(endGameInfo.cause == EndGame::Cause::ENDING_SERVER)
	{
		displayMessage("Server is ending, game is aborted.\n");
		_client.connectionLost();
		displayMessage("Connection is lost, let's try to reconnect properly.");
	}
	else
	{
		std::string endMessage{(endGameInfo.applyToSelf ? "You lost because you" : "You won because your opponent")};
		if(endGameInfo.cause == EndGame::Cause::TEN_TURNS_WITH_EMPTY_DECK)
			endMessage += " played 10 turns with an empty deck!";
		else if(endGameInfo.cause == EndGame::Cause::OUT_OF_HEALTH)
			endMessage += " ran out of health";
		else if(endGameInfo.cause == EndGame::Cause::QUITTED)
			endMessage += " quitted the game";
		else  // if cause is Cause::LOST_CONNECTION
		{
			endMessage += " lost connection with the server";
			_client.connectionLost();
			displayMessage("Connection is lost, let's try to reconnect properly.");
		}
		displayMessage(endMessage);
		if(not endGameInfo.applyToSelf)
		{
			// Get the won card
			CardId newCard;
			transmission >> newCard;
			receiveCard(newCard);
		}
		AchievementList newAchievements;
		transmission >> newAchievements;
		ClientAchievementList clientAchievements = _client.getAchievements(newAchievements);
		displayAchievements(clientAchievements);
	}
	_playing.store(false);
	_myTurn.store(!_myTurn.load());
}

void AbstractGame::handlePacket(sf::Packet& transmission)
{
	TransferType type;
	while(not transmission.endOfPacket())
	{
		transmission >> type;
		switch(type)
		{
		case TransferType::GAME_OVER:
			endGame(transmission);
			break;

		case TransferType::GAME_PLAYER_ENTER_TURN:
			displayMessage("You got the turn");
			_myTurn.store(true);
			break;

		case TransferType::GAME_PLAYER_LEAVE_TURN:
			displayMessage("You lost the turn");
			_myTurn.store(false);
			break;

		case TransferType::GAME_PLAYER_ENERGY_UPDATED:
		{
			std::lock_guard<std::mutex> lock{_accessEnergy};
			_selfEnergy = receiveFromPacket<sf::Uint32>(transmission);
		}
			break;

		case TransferType::GAME_PLAYER_HEALTH_UPDATED:
		{
			std::lock_guard<std::mutex> lock{_accessHealth};
			_selfHealth = receiveFromPacket<sf::Uint32>(transmission);
		}
			break;

		case TransferType::GAME_OPPONENT_HEALTH_UPDATED:
		{
			std::lock_guard<std::mutex> lock{_accessHealth};
			_oppoHealth = receiveFromPacket<sf::Uint32>(transmission);
		}
			break;

		case TransferType::GAME_BOARD_UPDATED:
			_selfBoardCreatures.clear();
			transmission >> _selfBoardCreatures;
			break;

		case TransferType::GAME_OPPONENT_BOARD_UPDATED:
			_oppoBoardCreatures.clear();
			transmission >> _oppoBoardCreatures;
			break;
		case TransferType::GAME_GRAVEYARD_UPDATED:
			_selfGraveCards.clear();
			transmission >> _selfGraveCards;
			break;

		case TransferType::GAME_HAND_UPDATED:
			_selfHandCards.clear();
			transmission >> _selfHandCards;
			break;

		case TransferType::GAME_OPPONENT_HAND_UPDATED:
			_oppoHandSize = receiveFromPacket<sf::Uint32>(transmission);
			break;

		case TransferType::GAME_DECK_UPDATED:
			_selfDeckSize = receiveFromPacket<sf::Uint32>(transmission);
			break;

		default:
			std::cerr << "Unknown message received: " << static_cast<std::underlying_type<TransferType>::type>(type) << "; ignore." << std::endl;
			break;
		}
	}
}

const std::string& AbstractGame::getCardName(CardId id)
{
	const CommonCardData* card = _client.getCardData(id);
	// Maybe I should have use multiple inheritance to avoid this
	return card->isSpell()
		? static_cast<const ClientSpellData*>(card)->getName()
		: static_cast<const ClientCreatureData*>(card)->getName();
}

CostValue AbstractGame::getCardCost(CardId id)
{
	const CommonCardData* card = _client.getCardData(id);
	// Maybe I should have use multiple inheritance to avoid this
	return card->isSpell()
		? static_cast<CostValue>(static_cast<const ClientSpellData*>(card)->getCost())
		: static_cast<CostValue>(static_cast<const ClientCreatureData*>(card)->getCost());
}

const std::string& AbstractGame::getCardDescription(CardId id)
{
	const CommonCardData* card = _client.getCardData(id);
	// Maybe I should have use multiple inheritance to avoid this
	return card->isSpell()
		? static_cast<const ClientSpellData*>(card)->getDescription()
		: static_cast<const ClientCreatureData*>(card)->getDescription();
}

bool AbstractGame::isSpell(CardId id)
{
	return _client.getCardData(id)->isSpell();
}

bool AbstractGame::handleHeader(TransferType header)
{
	switch(header)
	{
	case TransferType::GAME_NOT_ENOUGH_ENERGY:
		displayMessage("You haven't enough energy to play this card.");
		break;

	case TransferType::FAILURE:
		displayMessage("Error while playing the card.");
		break;

	case TransferType::GAME_CARD_LIMIT_TURN_REACHED:
		displayMessage("You cannot play cards for this turn anymore: you reached your limit.");
		break;

	default:
		return false;
	}
	return true;
}

template <typename FixedSizeIntegerType>
FixedSizeIntegerType AbstractGame::receiveFromPacket(sf::Packet& receivedPacket)
{
	assert(not receivedPacket.endOfPacket());
	FixedSizeIntegerType value;
	receivedPacket >> value;
	return std::move(value);
}
