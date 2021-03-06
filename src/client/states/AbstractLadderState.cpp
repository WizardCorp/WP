// std-C++ headers
#include <iostream>
#include <algorithm>
// WizardPoker headers
#include "client/sockets/Client.hpp"
#include "client/StateStack.hpp"
#include "client/states/AbstractLadderState.hpp"

AbstractLadderState::AbstractLadderState(Context& context):
	AbstractState(context)
{
	try
	{
		_ladder = _context.client->getLadder();
	}
	catch(const std::runtime_error& e)
	{
		std::cout << "Error: " << e.what() << "\n";
		std::cout << "Empty ladder loaded.\n";
		return;
	}
	std::sort(_ladder.begin(), _ladder.end(), [](const LadderEntry& lhs, const LadderEntry& rhs)
	{
		if(lhs.victories + lhs.defeats == 0)
			return false;
		else if(rhs.victories + rhs.defeats == 0)
			return true;
		else
			return (static_cast<double>(lhs.victories + 1) / (lhs.defeats + 1))
			        > (static_cast<double>(rhs.victories + 1) / (rhs.defeats + 1));
	});
}

void AbstractLadderState::backMainMenu()
{
	_context.stateStack->pop();
}
