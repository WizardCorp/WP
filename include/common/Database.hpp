#ifndef _DATABASE_COMMON_HPP
#define _DATABASE_COMMON_HPP

// std-C++ headers
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <array>
#include <utility> // std::pair
#include <mutex>

// SQLite headers
#include <sqlite3.h>
// WizardPoker headers
#include "common/Deck.hpp"
#include "common/CardsCollection.hpp"
#include "common/Identifiers.hpp"

#include "common/Ladder.hpp"

struct Statement : private std::pair<sqlite3_stmt **, const char *> // I dont care that it is a std::pair (this is just for implementation) and I prefer apply maximum restrictions rule
{
	constexpr Statement(const first_type& statement, const second_type& query)
		: std::pair<first_type, second_type>(statement, query)
	{}

	/*Statement(std::piecewise_construct_t, std::tuple<first_type> statement, std::tuple<second_type> query)
		: std::pair<first_type, second_type>(std::piecewise_construct, statement, query)
	{}*/ // unused for now

	sqlite3_stmt ** statement()
	{
		return first;
	}

	const char * query() const
	{
		return second;
	}
};

template <std::size_t N>
using StatementsList = std::array<Statement, N>;

/// Interface to the database
class Database
{
public:
	/// Constructor
	/// \param filename: relative path to sqlite3 file
	explicit Database(const std::string& filename);

	/// Destructor
	virtual ~Database();

protected:
	/// To get valid sqlite3_stmt
	void prepareStmt(Statement&);

	/// Throw exception if errcode is actually an error code
	static int sqliteThrowExcept(int errcode);

	sqlite3 *_database;

	/// Avoids race conditions on DB
	std::mutex _dbAccess;
};

struct Friend
{
	UserId id;
	std::string name;
};
typedef std::vector<Friend> FriendsList;

#endif // _DATABASE_COMMON_HPP
