#include "common/Deck.hpp"
#include "common/CardsCollection.hpp"

CardsCollection::CardsCollection()
{
	for(std::size_t i{1}; i <= Deck::size; ++i)
		_cards.insert(static_cast<CardId>(i));
}

CardsCollection::CardsCollection(const std::multiset<CardId>& cards):
	_cards{cards}
{
}

void CardsCollection::addCard(CardId card)
{
	_cards.insert(card);
}

std::size_t CardsCollection::count(CardId card)
{
	return _cards.count(card);
}

bool CardsCollection::contains(CardId card)
{
	return count(card) > 0;
}

std::size_t CardsCollection::getSize() const
{
	return _cards.size();
}

CardsCollection::Iterator CardsCollection::begin()
{
	return _cards.begin();
}

CardsCollection::Iterator CardsCollection::end()
{
	return _cards.end();
}

CardsCollection::ConstIterator CardsCollection::cbegin() const
{
	return _cards.cbegin();
}

CardsCollection::ConstIterator CardsCollection::cend() const
{
	return _cards.cend();
}
