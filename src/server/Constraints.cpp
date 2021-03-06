// WizardPoker headers
#include "server/Constraints.hpp"
#include "server/Creature.hpp"
// std-C++ headers
#include <cassert>
#include <stdexcept>
#include <iostream>

Constraints::Constraints(const std::vector<ConstraintDefaultValue>& defaultValues):
	_defaultValues(defaultValues),
	_timedValues(defaultValues.size())  // creates a vector of size `defaultValues.size()`
{

}

void Constraints::setConstraint(int constraintId, int value, int turns, const Creature* caster)
{
	assert(constraintId < static_cast<int>(_defaultValues.size()) and constraintId >= 0);
	_timedValues.at(constraintId).push_back({value, turns, caster});
}

int Constraints::getConstraint(int constraintId) const
{
	assert(constraintId < static_cast<int>(_defaultValues.size()) and constraintId >= 0);
	switch(_defaultValues.at(constraintId).orderOption)
	{
		case ConstraintOrderOption::GET_FIRST:
			return getFirstTimedValue(constraintId);
		case ConstraintOrderOption::GET_LAST:
			return getLastTimedValue(constraintId);
		case ConstraintOrderOption::GET_SUM:
			return getSumTimedValues(constraintId);
	}
	throw std::runtime_error("Order option not valid");
}

int Constraints::getOverallConstraint(int constraintId, int otherValue) const
{
	assert(constraintId < static_cast<int>(_defaultValues.size()) and constraintId >= 0);
	switch(_defaultValues.at(constraintId).orderOption)
	{
		case ConstraintOrderOption::GET_FIRST:
			if(otherValue == _defaultValues.at(constraintId).value)
				return getFirstTimedValue(constraintId);
			else
				return otherValue;
		case ConstraintOrderOption::GET_LAST:
			if(otherValue == _defaultValues.at(constraintId).value)
				return getLastTimedValue(constraintId);
			else
				return otherValue;
		case ConstraintOrderOption::GET_SUM:
			otherValue += getSumTimedValues(constraintId);
			return otherValue;
	}
	throw std::runtime_error("Order option not valid");
}

int Constraints::getValue(int constraintId, unsigned valueIndex) const
{
	assert(constraintId < static_cast<int>(_defaultValues.size()) and constraintId >= 0);
	int value{_timedValues.at(constraintId).at(valueIndex).value};
	switch(_defaultValues.at(constraintId).valueOption) //rules
	{
		case ConstraintValueOption::VALUE_GET_INCREMENT:
			_timedValues.at(constraintId).at(valueIndex).value++;
			break;
		case ConstraintValueOption::VALUE_GET_DECREMENT:
			_timedValues.at(constraintId).at(valueIndex).value--;
			break;
		default:
			// do nothing to value
			break;
	}
	return value;
}

int Constraints::getFirstTimedValue(int constraintId) const
{
	assert(constraintId < static_cast<int>(_defaultValues.size()) and constraintId >= 0);
	std::vector<ConstraintTimedValue>& vect(_timedValues.at(constraintId)); //value, turns left, caster
	for(auto vectIt(vect.begin()); vectIt != vect.end(); vectIt++)
	{
		//if the caster is not remembered, or is on the board
		if(vectIt->caster == nullptr or vectIt->caster->isOnBoard() or vectIt->caster->getConstraint(CC_TEMP_IS_PARALYZED) == 0)
			return getValue(constraintId, static_cast<unsigned>(vectIt - vect.begin()));

	}
	return _defaultValues.at(constraintId).value;
}

int Constraints::getLastTimedValue(int constraintId) const
{
	assert(constraintId < static_cast<int>(_defaultValues.size()) and constraintId >= 0);
	std::vector<ConstraintTimedValue>& vect = _timedValues.at(constraintId); //value, turns left, caster
	for(auto vectIt(vect.rbegin()); vectIt != vect.rend(); vectIt++)
	{
		//if the caster is not remembered, or is on the board and active
		if(vectIt->caster == nullptr or vectIt->caster->isOnBoard() or vectIt->caster->getConstraint(CC_TEMP_IS_PARALYZED) == 0)
			return getValue(constraintId, static_cast<unsigned>(vectIt - vect.rbegin()));

	}
	return _defaultValues.at(constraintId).value;
}

int Constraints::getSumTimedValues(int constraintId) const
{
	assert(constraintId < static_cast<int>(_defaultValues.size()) and constraintId >= 0);
	int value = _defaultValues.at(constraintId).value;
	std::vector<ConstraintTimedValue>& vect = _timedValues.at(constraintId); //value, turns left, caster
	for(auto vectIt(vect.begin()); vectIt != vect.end(); vectIt++)
	{
		//if the caster is not remembered, or is on the board and active
		if(vectIt->caster == nullptr or vectIt->caster->isOnBoard() or vectIt->caster->getConstraint(CC_TEMP_IS_PARALYZED) == 0)
			value += getValue(constraintId, static_cast<unsigned>(vectIt - vect.begin()));
	}
	return value;
}

void Constraints::timeOutConstraints()
{
	for (unsigned i{0}; i < _defaultValues.size(); i++)
	{
		std::vector<ConstraintTimedValue>& vect(_timedValues[i]); //value, turns left, caster
		for(auto vectIt(vect.begin()); vectIt != vect.end();)
		{
			//if the constraint has run our of turns or if its caster has died
			if(vectIt->turns == 1 or not (vectIt->caster == nullptr or vectIt->caster->isOnBoard()))
				vectIt = vect.erase(vectIt); //returns iterator to following object
			else
			{
				vectIt->turns--;
				switch(_defaultValues[i].valueOption)  // rules
				{
					case ConstraintValueOption::VALUE_TURN_INCREMENT:
						vectIt->value++;
						break;
					case ConstraintValueOption::VALUE_TURN_DECREMENT:
						vectIt->value--;
						break;
					default:
						//do nothing to value
						break;
				}
				vectIt++;
			}
		}
	}
}
