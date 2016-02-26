#include "server/Card.hpp"

Card::Card(cardId cardIdentifier, int cost, std::vector<EffectParamsCollection> effects):
	_cardID(cardIdentifier),
	_cost(cost),
	_effects(effects)
{

}

std::vector<EffectParamsCollection> Card::getEffects()
{
	return _effects;
}

int Card::getEnergyCost()
{
	return _cost;
}

cardId Card::getID()
{
	return _cardID;
}
