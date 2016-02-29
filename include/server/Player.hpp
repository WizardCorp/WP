#ifndef _PLAYER_HPP
#define _PLAYER_HPP

// std-C++ headers
#include <stack>
#include <random>
#include <array>
#include <functional>
#include <cstddef>
// WizardPoker headers
#include "server/Card.hpp"
#include "server/Spell.hpp"
#include "server/Constraints.hpp"
#include "common/CardData.hpp"
#include "common/GameData.hpp"
#include "common/Identifiers.hpp"  // userId
#include "common/sockets/TransferType.hpp"
#include "common/Deck.hpp"
// SFML headers
#include <SFML/Network/TcpSocket.hpp>

class GameThread;
class Creature;

/// Represents one of the two players for a given game.
class Player
{
public:
	/// Constructor
	Player(userId id, sf::TcpSocket& socket, sf::TcpSocket& specialSocket);
	void setOpponent(Player* opponent);  // Complementary

	/// Destructor.
	~Player() = default;

	/// Interface for basic gameplay (board)
	void beginGame(bool isActivePlayer);
	void enterTurn(int turn);
	void leaveTurn();
	void useCard(int handIndex); 	///< Use a card
	void attackWithCreature(int boardIndex, int victim);  ///< Attack victim (-1 for opponent) with a card
	void endGame(); //TODO: define behavior and call for opponent when quitting

	/// Interface for applying effects
	//to Player
	void applyEffect(const Card* usedCard, EffectParamsCollection effectArgs);
	//to a Creature
	void applyEffectToCreature(Creature* casterAndSubject, EffectParamsCollection effectArgs); //With ref. to creature
	void applyEffectToCreature(const Card* usedCard, EffectParamsCollection effectArgs, std::vector<int> boardIndexes); //With creature index
	//to all Creatures
	void applyEffectToCreatureTeam(const Card* usedCard, EffectParamsCollection effectArgs);

	/// Getters
	int getCreatureConstraint(const Creature& subject, int constraintIDD);
	const Card* getLastCaster();
	userId getID();
	const std::vector<Creature *>& getBoard();

	/// Setters
	void setDeck(const Deck& newDeck);

	///
	/// \return a vector of indices selected
	/// \param selection a vector of values telling whether the choice must be in player's cards or opponent's cards
	std::vector<int>&& getRandomBoardIndexes(const std::vector<CardToSelect>& selection);
	std::vector<int>&& askUserToSelectCards(const std::vector<CardToSelect>& selection);

private:
	/// Types
	struct TurnData
	{
		int cardsUsed;
		int creaturesPlaced;
		int creatureAttacks;
		int spellCalls;
	};
	constexpr static TurnData _emptyTurnData = {0, 0, 0, 0};

	/// Attributes
	GameThread* _board;
	Player* _opponent = nullptr;
	userId _id;

	//Client communication
	sf::TcpSocket& _socketToClient;
	sf::TcpSocket& _specialSocketToClient;

	// Gameplay
	int _energy, _energyInit = 0, _health;
	int _turnsSinceEmptyDeck;
	static const int _maxEnergy = 10, _maxHealth = 20;
	TurnData _turnData;

	// Constraints
	Constraints _constraints = Constraints(P_CONSTRAINT_DEFAULTS, P_CONSTRAINTS_COUNT);
	Constraints _teamConstraints = Constraints(C_CONSTRAINT_DEFAULTS, C_CONSTRAINTS_COUNT);

	// Card holders
	std::stack<Card *> _cardDeck;  ///< Cards that are in the deck (not usable yet)
	std::vector<Card *> _cardHand;  ///< Cards that are in the player's hand (usable)
	std::vector<Creature *> _cardBoard;  ///< Cards that are on the board (usable for attacks)
	std::vector<Card *> _cardBin;  ///< Cards that are discarded (dead creatures, used spells)
	const Card* _lastCasterCard=nullptr; ///<Last card that was used to cast an effect (his or opponent's)

	// Random management
	std::default_random_engine _engine;

	// Effects container
	static std::function<void(Player&, const EffectParamsCollection&)> _effectMethods[P_EFFECTS_COUNT];

	/// Effects (private)
	void setConstraint(const EffectParamsCollection& args);
	void pickDeckCards(const EffectParamsCollection& args);
	void loseHandCards(const EffectParamsCollection& args);
	void reviveBinCard(const EffectParamsCollection& args);
	void stealHandCard(const EffectParamsCollection& args);
	void exchgHandCard(const EffectParamsCollection& args);
	void resetEnergy(const EffectParamsCollection& args);
	void changeEnergy(const EffectParamsCollection& args);
	void changeHealth(const EffectParamsCollection& args);

	/// Other private methods
	void exploitCardEffects(Card* usedCard);
	void setTeamConstraint(const Card* usedCard, const EffectParamsCollection& effectArgs);

	void cardDeckToHand(int amount);
	void cardHandToBoard(int handIndex);
	void cardHandToBin(int handIndex);  ///< Move the card at handIndex from the player's hand to the bin
	void cardBoardToBin(int boardIndex);  ///< Move the card at boardIndex from the board to the bin
	void cardBinToHand(int binIndex);
	void cardAddToHand(Card* given);
	Card* cardRemoveFromHand();
	Card* cardExchangeFromHand(Card* given);
	Card* cardExchangeFromHand(Card* given, int handIndex);

	void useCreature(int handIndex, Card *& usedCard);
	void useSpell(int handIndex, Card *& useSpell);

	void sendCurrentEnergy();
	void sendCurrentHealth();

	void sendHandState();
	void sendBoardState();
	void sendOpponentBoardState();
	void sendGraveyardState();

	template <typename CardType>
	void sendIDsFromVector(TransferType type, const std::vector<CardType *>& vect);
	void sendCardDataFromVector(TransferType type, const std::vector<Card*>& vect);
	void sendBoardCreatureDataFromVector(TransferType type, const std::vector<Creature*>& vect);
	void sendValueToClient(sf::TcpSocket& socket, TransferType value);
};


#endif  // _PLAYER_HPP
