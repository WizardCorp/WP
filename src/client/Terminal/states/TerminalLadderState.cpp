// std-C++ headers
#include <iostream>
// WizardPoker headers
#include "client/Terminal/states/TerminalLadderState.hpp"

TerminalLadderState::TerminalLadderState(StateStack& stateStack, Client& client):
	AbstractState(stateStack, client),
	TerminalAbstractState(stateStack, client),
	AbstractLadderState(stateStack, client)
{
	addAction("Back to main menu", &TerminalLadderState::backMainMenu);
}

void TerminalLadderState::display()
{
	std::cout << "Here is the ladder:\n";
	for(unsigned i{0}; i < _ladder.nbOfPlayers; ++i)
	{
		const auto& ladderEntry{_ladder.ladder.at(i)};
		std::cout << i+1 << ". " << ladderEntry.name
		          << " (" << ladderEntry.victories + 1
		          << "/" << (ladderEntry.defeats + 1)
		          << ")\n";
	}
	std::cout << std::string(40, '*') << "\n";
	// Display the actions
	TerminalAbstractState::display();
}
