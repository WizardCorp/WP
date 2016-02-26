#ifndef _IDENTIFIERS_COMMON_HPP
#define _IDENTIFIERS_COMMON_HPP

#include <cstdint>

/// The identifier of a card, this is the identifier that is encoded in the
/// database, and the only information used to transmit a card on the network.
typedef int64_t cardId;

/// The identifier of a deck, this is the identifier that is encoded in the
/// database, and the only information used to transmit a deck on the network.
typedef int64_t deckId;

/// The identifier of a card, this is the idetnfier that is encoded in the
/// database, and is used by the class Server to identify an user.
typedef int64_t userId;

#endif  // _IDENTIFIERS_COMMON_HPP