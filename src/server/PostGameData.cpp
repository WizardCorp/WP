#include "server/PostGameData.hpp"

PostGameData::PostGameData() :
	opponentInDaClub(false),
	playerStarted(false),
	playerTookDamage(false),
	playerWon(false),
	playerRageQuit(false),
	unlockedCard(0),
	remainingHealth(0),
	gameDuration(0)
{
	assert(static_cast<int>(false) == 0 and static_cast<int>(true) == 1);
}

std::string PostGameData::display()
{
	return 	std::string("\t-opponentInDaClub : ") + std::to_string(opponentInDaClub) +
			std::string("\n\t-playerStarted : ") + std::to_string(playerStarted) +
			std::string("\n\t-playerTookDamage : ") + std::to_string(playerTookDamage) +
			std::string("\n\t-playerWon : ") + std::to_string(playerWon) +
			std::string("\n\t-playerRageQuit : ") + std::to_string(playerRageQuit) +
			std::string("\n\t-unlockedCard : ") + std::to_string(unlockedCard) +
			std::string("\n\t-remainingHealth : ") + std::to_string(remainingHealth) +
			std::string("\n\t-gameDuration : ") + std::to_string(gameDuration);
}
