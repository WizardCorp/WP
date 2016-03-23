// std-C++ headers
#include <iostream>
// WizardPoker header
#include "client/Gui/SpellGui.hpp"

SpellGui::SpellGui(const std::string& name, const std::string& description, int cost):
	CardGui(name, description, cost)
{
	if(!_pictureTexture.loadFromFile("frontSpell.png"))
	{
		std::cerr << "Unable to load frontSpell.png\n";
		return;
	}  // Standard SFML 2.0 error protocol (CF documentation)
	_pictureTexture.setSmooth(true);
	_picture.setTexture(&_pictureTexture);
	_picture.setSize(SIZE);
}
