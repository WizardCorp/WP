#ifndef _TERMINAL_FRIENDS_MANAGEMENT_STATE_CLIENT_HPP
#define _TERMINAL_FRIENDS_MANAGEMENT_STATE_CLIENT_HPP

// WizardPoker headers
#include "client/Terminal/TerminalAbstractState.hpp"
#include "client/states/AbstractFriendsManagementState.hpp"

/// Final class for the friends management in the terminal.
class TerminalFriendsManagementState : public TerminalAbstractState, public AbstractFriendsManagementState
{
	public:
		/// Constructor.
		TerminalFriendsManagementState(StateStack& stateStack, Client& client);

		/// The display function.
		/// It must do all things related to drawing or printing stuff on the screen.
		virtual void display() override;

	private:
		void addFriend();
		void removeFriend();
		void treatRequests();
		void startChat();
};

#endif// _TERMINAL_FRIENDS_MANAGEMENT_STATE_CLIENT_HPP
