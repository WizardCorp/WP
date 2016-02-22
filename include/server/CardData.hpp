#ifndef _CARD_DATA_HPP_
#define _CARD_DATA_HPP_

#include <string>
#include <vector>

/// Constraint Rules
enum CONSTRAINT_VALUE_RULE : unsigned
{
	VALUE_FIXED,
	VALUE_GET_DECREMENT,
	VALUE_GET_INCREMENT,
	VALUE_TURN_DECREMENT,
	VALUE_TURN_INCREMENT
};

/// PLAYER EFFECTS
enum PLAYER_EFFECTS : unsigned
{
	//constraints
	PE_SET_CONSTRAINT,
	//other effects
	PE_PICK_DECK_CARDS,
	PE_LOSE_HAND_CARDS,
	PE_REVIVE_BIN_CARD,
	PE_STEAL_HAND_CARD,
	PE_EXCHG_HAND_CARD,
	PE_SET_ENERGY_POINTS,
	PE_ADD_ENERGY_POINTS,
	PE_SUB_ENERGY_POINTs,
	PE_ADD_HEALTH_POINTS,
	PE_SUB_HEALTH_POINTS,
	//count
	P_EFFECTS_COUNT
};

/// PLAYER CONSTRAINTS
enum PLAYER_CONSTRAINTS : unsigned
{
	//turn-by-turn
	PC_CARD_PICK_AMOUNT, 			//amount of cards to pick each turn
	PC_ENERGY_POINTS_INIT,			//initial amount of energy points each turn
	PC_HEALTH_POINTS_GAIN,			//amount of health points given each turn
	PC_HEALTH_POINTS_LOSS, 			//amount of health points taken each turn
	PC_HEALTH_POINTS_LOSS_DECK_EMPTY,	//amount of health points taken each turn when deck is empty
	//passive
	PC_USE_CARD_LIMIT,				//limit for using cards
	PC_CALL_SPELL_LIMIT,			//limit for calling spells
	PC_ATTACK_WITH_CREATURE_LIMIT,	//limit for attacking with creatures
	PC_PLACE_CREATURE_LIMIT,		//limit for placing creatures on board
	PC_CREATURES_ON_BOARD_LIMIT,	//limit for number of creatures on the board
	//count
	P_CONSTRAINTS_COUNT
};

constexpr std::pair<unsigned, CONSTRAINT_VALUE_RULE> P_CONSTRAINT_DEFAULTS[P_CONSTRAINTS_COUNT] =
{
	//TURN-BASED CONSTRAINTS
	{1,VALUE_FIXED},		//PC_CARD_PICK_AMOUNT
	{10,VALUE_FIXED},		//PC_ENERGY_POINTS_INIT
	{0,VALUE_FIXED}, 		//PC_HEALTH_POINTS_GAIN
	{0,VALUE_FIXED},		//PC_HEALTH_POINTS_LOSS
	{5,VALUE_FIXED}, 		//PC_HEALTH_POINTS_LOSS_DECK_EMPTY
	//PASSIVE CONSTRAINTS
	{100,VALUE_FIXED},		//PC_USE_CARD_LIMIT,
	{100,VALUE_FIXED},		//PC_CALL_SPELL_LIMIT
	{100,VALUE_FIXED}, 		//PC_ATTACK_WITH_CREATURE_LIMIT
	{6,VALUE_FIXED},		//PC_PLACE_CREATURE_LIMIT
	{6,VALUE_FIXED}			//PC_CREATURES_ON_BOARD_LIMIT
};

/// CREATURE EFFECTS
enum CREATURE_EFFECTS : unsigned
{
	//constraints
	CE_SET_CONSTRAINT,
	//other effects
	CE_RESET_ATTACK,
	CE_RESET_HEALTH,
	CE_RESET_SHIELD,
	CE_ADD_ATTACK,
	CE_ADD_HEALTH,
	CE_ADD_SHIELD,
	CE_SUB_ATTACK,
	CE_SUB_HEALTH,
	CE_SUB_SHIELD,
	CE_FORCED_SUB_HEALTH,
	//count
	C_EFFECTS_COUNT
};

/// CREATURE CONSTRAINTS
enum CREATURE_CONSTRAINTS : unsigned
{
	//turn-by-turn
	CC_SELF_HEALTH_GAIN,			// Gain X health points
	CC_TEAM_HEALTH_GAIN,			// Team gains X health points
	CC_SELF_HEALTH_LOSS,
	CC_TEAM_HEALTH_LOSS,
	CC_SELF_ATTACK_GAIN,			// Gain X attack points
	CC_TEAM_ATTACK_GAIN,			// Team gains X attack points
	CC_SELF_ATTACK_LOSS,
	CC_TEAM_ATTACK_LOSS,
	CC_SELF_SHIELD_LOSS,			// Lose X shield points
	//passive
	CC_SELF_BLOCK_ATTACKS,		// Block the next X attacks on himself
	CC_TEAM_BLOCK_ATTACKS,		// Block the next X attacks on team
	CC_SELF_PARALYZED,			// Can not be used
	//ending (when creature dies)
	CC_END_TEAM_HEALTH_GAIN,
	CC_END_TEAM_ATTACK_LOSS,
	CC_END_TEAM_SHIELD_LOSS,
	//count
	C_CONSTRAINTS_COUNT
};

constexpr std::pair<unsigned, CONSTRAINT_VALUE_RULE> C_CONSTRAINT_DEFAULTS[C_CONSTRAINTS_COUNT] =
{
	//turn-by-turn: all default to 0
	{0,VALUE_FIXED},		//CC_SELF_HEALTH_GAIN
	{0,VALUE_FIXED},		//CC_TEAM_HEALTH_GAIN
	{0,VALUE_FIXED}, 		//CC_SELF_HEALTH_LOSS
	{0,VALUE_FIXED},		//CC_TEAM_HEALTH_LOSS
	{0,VALUE_FIXED}, 		//CC_SELF_ATTACK_GAIN
	{0,VALUE_FIXED},		//CC_TEAM_ATTACK_GAIN
	{0,VALUE_FIXED},		//CC_SELF_ATTACK_LOSS
	{0,VALUE_FIXED},		//CC_TEAM_ATTACK_LOSS
	{0,VALUE_FIXED},		//CC_SELF_SHIELD_LOSS
	//passive
	{0,VALUE_GET_DECREMENT},	//CC_SELF_BLOCK_ATTACKS
	{0,VALUE_GET_DECREMENT},	//CC_TEAM_BLOCK_ATTACKS
	{0,VALUE_FIXED},			//CC_SELF_PARALYZED
	//ending
	{0,VALUE_FIXED},		//CC_END_TEAM_HEALTH_GAIN
	{0,VALUE_FIXED},		//CC_END_TEAM_ATTACK_LOSS
	{0,VALUE_FIXED}			//CC_END_TEAM_SHIELD_LOSS
};


///Used types
enum CostValue : unsigned
{
	COST_0,COST_1,COST_2,COST_3,COST_4,COST_5,COST_6,COST_7,COST_8,COST_9,COST_10
};

enum AttackValue : unsigned
{
	ATTACK_0,ATTACK_1,ATTACK_2,ATTACK_3,ATTACK_4,ATTACK_5,ATTACK_6,ATTACK_7,ATTACK_8,ATTACK_9,ATTACK_10
};

enum HealthValue : unsigned
{
	HEALTH_1,HEALTH_2,HEALTH_3,HEALTH_4,HEALTH_5,HEALTH_6,HEALTH_7,HEALTH_8,HEALTH_9,
	HEALTH_10,HEALTH_11,HEALTH_12,HEALTH_13,HEALTH_14,HEALTH_15,HEALTH_16,HEALTH_17,HEALTH_18,HEALTH_19,HEALTH_20
};

enum ShieldValue: unsigned
{
	SHIELD_0,SHIELD_1,SHIELD_2,SHIELD_3,SHIELD_4,SHIELD_5
};

enum ShieldType : unsigned
{
	SHIELD_NONE,		//no shield
	SHIELD_BLUE,		//allows part of the attack to deal damage if attack is stronger than shield
	SHIELD_ORANGE,		//allows all the attack to deal damage if attack is stronger than shield
	SHIELD_LEGENDARY	//creature attacks don't go through
};

typedef std::vector<unsigned> EffectParamsCollection;

enum EFFECT_SUBJECTS : unsigned
{
	PLAYER_SELF,
	PLAYER_OPPO,
	CREATURE_SELF,
	CREATURE_TEAM,
	CREATURE_ONE_OPPO,
	CREATURE_ALL_OPPO
};

///Creature type
struct CreatureData
{
	std::string name;
	CostValue cost;
	AttackValue attack;
	HealthValue health;
	ShieldValue shield;
	ShieldType shieldType;
	std::vector<EffectParamsCollection> effects;
	std::string description;
};

///Spell type
struct SpellData
{
	std::string name;
	CostValue cost;
	std::vector<EffectParamsCollection> effects;
	std::string description;
};

extern const CreatureData ALL_CREATURES[];

extern const SpellData ALL_SPELLS[];

#endif  // _CARD_DATA_HPP
