// WizardPoker headers
#include "client/sockets/Client.hpp"
#include "common/UnableToConnectException.hpp"
#include "client/Gui/states/GuiMainMenuState.hpp"
#include "client/Gui/states/GuiHomeState.hpp"

GuiHomeState::GuiHomeState(Context& context):
	AbstractState(context),
	GuiAbstractState(context),
	AbstractHomeState(context),
	_userNameLabel{std::make_shared<tgui::Label>()},
	_passwordLabel{std::make_shared<tgui::Label>()},
	_userNameEditBox{std::make_shared<tgui::EditBox>()},
	_passwordEditBox{std::make_shared<tgui::EditBox>()},
	_connectButton{std::make_shared<tgui::Button>()},
	_createAccountButton{std::make_shared<tgui::Button>()},
	_grid{std::make_shared<tgui::Grid>()},
	_gridBottom{std::make_shared<tgui::Grid>()}
{
	// Get a bound version of the window size
	// Passing this to setPosition or setSize will make the widget automatically
	// update when the view of the gui changes
	auto windowWidth(tgui::bindWidth(*_context.gui));
	auto windowHeight(tgui::bindHeight(*_context.gui));

	makeTitle("WizardPoker", 40U);

	// Configure the widgets
	_userNameLabel->setText("Username");
	_userNameLabel->setTextColor(sf::Color::White);
	// Focus the password edit box when pressing enter in user name edit box
	_userNameEditBox->connect("ReturnKeyPressed", [this]()
	{
		_passwordEditBox->focus();
	});
	_passwordLabel->setText("Password");
	_passwordLabel->setTextColor(sf::Color::White);
	// Connect when pressing enter in the password edit box
	_passwordEditBox->connect("ReturnKeyPressed", &GuiHomeState::connect, this);
	_passwordEditBox->setPasswordCharacter('*');  // Display only stars in the edit box
	_connectButton->setText("Connect");
	_connectButton->connect("Pressed", &GuiHomeState::connect, this);
	_createAccountButton->setText("Create an account");
	_createAccountButton->connect("Pressed", &GuiHomeState::createAccount, this);

	// Add the widgets to the grid
	_grid->setPosition(windowWidth/4.f, windowHeight/4.f);
	_grid->setSize(windowWidth/2.f, windowHeight/4.f);
	_grid->addWidget(_userNameLabel, 0, 0, {0, 0, 0, 0}, tgui::Grid::Alignment::Up);
	_grid->addWidget(_userNameEditBox, 0, 1, {2, 0, 0, 0}, tgui::Grid::Alignment::Up);
	_grid->addWidget(_passwordLabel, 1, 0, {0, 0, 0, 0}, tgui::Grid::Alignment::Up);
	_grid->addWidget(_passwordEditBox, 1, 1, {2, 0, 0, 0}, tgui::Grid::Alignment::Up);
	_context.gui->add(_grid);

	_gridBottom->setPosition(tgui::bindLeft(_grid), tgui::bindBottom(_grid));
	_gridBottom->setSize(windowWidth/2.f, windowHeight/8.f);
	_connectButton->setSize(tgui::bindWidth(_gridBottom), windowHeight/16.f);
	_createAccountButton->setSize(tgui::bindWidth(_gridBottom), windowHeight/16.f);
	_gridBottom->addWidget(_connectButton, 0, 0);
	_gridBottom->addWidget(_createAccountButton, 1, 0);
	_context.gui->add(_gridBottom);

	registerRootWidgets({_grid, _gridBottom});
	_userNameEditBox->focus();  // Focus the user name edit box by default
}

void GuiHomeState::connect()
{
	try
	{
		const std::string userName{_userNameEditBox->getText()};
		const std::string password{_passwordEditBox->getText()};
		tryToConnect(userName, password);
		_context.window->setTitle("WizardPoker (" + userName + ")");
		_context.stateStack->push<GuiMainMenuState>();
	}
	catch(const std::runtime_error& e)
	{
		displayMessage(e.what());
		// If the error was caused by client::updateFriends, the client is connected
		// but the user will stay on this state. The client must be deconnected.
		_context.client->quit();
	}
}

void GuiHomeState::createAccount()
{
	try
	{
		const std::string userName{_userNameEditBox->getText()};
		const std::string password{_passwordEditBox->getText()};
		tryToRegister(userName, password);
	}
	catch(const std::runtime_error& e)
	{
		displayMessage(e.what());
	}
}
