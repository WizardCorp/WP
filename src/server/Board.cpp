// WizardPoker headers
#include "server/Board.hpp"
// std-C++ headers
#include <random>

Board::Board(Player::ID player1, Player::ID player2)
{
	// A random player starts the game
	std::default_random_engine engine;
	std::uniform_int_distribution<int> distribution(1, 2);
	if (distribution(engine) == 1)
		std::swap(player1, player2);
	_activePlayer = new Player(player1);
	_passivePlayer = new Player(player2);
	_activePlayer->setOpponent(_passivePlayer);
	_passivePlayer->setOpponent(_activePlayer);
}

/*--------------------------- USER INTERFACE */
void Board::endTurn()
{
	std::swap(_activePlayer, _passivePlayer);
	_turn++;
	_passivePlayer->leaveTurn(_turn/2 +1);
	_activePlayer->enterTurn(_turn/2 +1);
}

void Board::useCard(int handIndex)
{
	_activePlayer->useCard(handIndex);
}

void Board::attackWithCreature(int boardIndex, int victim)
{
	_activePlayer->attackWithCreature(boardIndex, victim);
}

void Board::quitGame()
{
	//TODO: need identifier for player who quit
}

/*--------------------------- CARD INTERFACE */
void Board::applyEffect(Card* usedCard, EffectParamsCollection effectArgs)
{
	int subject = effectArgs.front();
	effectArgs.erase(effectArgs.begin());
	int method = effectArgs.front();
	effectArgs.erase(effectArgs.begin());

	switch (subject)
	{
		case PLAYER_SELF:
			Player::effectMethods[method](*_activePlayer, effectArgs);
			break;

		case PLAYER_OPPO:
			Player::effectMethods[method](*_passivePlayer, effectArgs);
			break;

		case CREATURE_SELF_THIS:
			{
				Creature* usedCreature = dynamic_cast<Creature*>(usedCard);
                Creature::effectMethods[method](*usedCreature, effectArgs);
			}
			break;

		case CREATURE_SELF_TEAM:
			_activePlayer->applyEffectToCreatures(method, effectArgs);
			break;

		case CREATURE_OPPO_INDX:
			{
				int boardIndex = effectArgs.front();
				effectArgs.erase(effectArgs.begin());
				_passivePlayer->applyEffectToCreature(boardIndex, method, effectArgs);
			}
			break;
		case CREATURE_OPPO_RAND:
				_passivePlayer->applyEffectToCreature(-1, method, effectArgs);
			break;
		case CREATURE_OPPO_TEAM:
			_passivePlayer->applyEffectToCreatures(method, effectArgs);
			break;
	}
}

/*--------------------------- PLAYER INTERFACE */

