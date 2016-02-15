#ifndef _CHAT_SELECTION_STATE_HPP_
#define _CHAT_SELECTION_STATE_HPP_

#include "client/AbstractState.hpp"

class ChatSelectionState : public AbstractState
{
	public:
		ChatSelectionState(StateStack& stateStack);

		virtual void display() override;

	private:

};

#endif  // _CHAT_SELECTION_STATE_HPP_
