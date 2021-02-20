/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'. In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting. We hope that you share your changes too. What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

/* attack table  -- not very organized :( */
const struct attack_type attack_table[] = {
    {"hit", -1},                /*  0 */
    {"slice", DAM_SLASH},
    {"stab", DAM_PIERCE},
    {"slash", DAM_SLASH},
    {"whip", DAM_SLASH},
    {"claw", DAM_SLASH},        /*  5 */
    {"blast", DAM_BASH},
    {"pound", DAM_BASH},
    {"crush", DAM_BASH},
    {"grep", DAM_SLASH},
    {"bite", DAM_PIERCE},       /* 10 */
    {"pierce", DAM_PIERCE},
    {"suction", DAM_BASH},
    {"beating", DAM_BASH},
    {"digestion", DAM_ACID},
    {"charge", DAM_BASH},       /* 15 */
    {"slap", DAM_BASH},
    {"punch", DAM_BASH},
    {"wrath", DAM_ENERGY},
    {"magic", DAM_ENERGY},
    {"divine power", DAM_HOLY}, /* 20 */
    {"cleave", DAM_SLASH},
    {"scratch", DAM_PIERCE},
    {"peck", DAM_PIERCE},
    {"peck", DAM_BASH},
    {"chop", DAM_SLASH},        /* 25 */
    {"sting", DAM_PIERCE},
    {"smash", DAM_BASH},
    {"shocking bite", DAM_LIGHTNING},
    {"flaming bite", DAM_FIRE},
    {"freezing bite", DAM_COLD},    /* 30 */
    {"acidic bite", DAM_ACID},
    {"chomp", DAM_PIERCE},
    {"weapon's bite", DAM_NEGATIVE},
    {"weapon's flame", DAM_FIRE},
    {"weapon's frost", DAM_COLD},    /* 35 */
    {"chomp",DAM_PIERCE}  // From Mudweiser
};

/* race table */
const struct race_type race_table[] = {
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts,          remort only 
    },
*/
    {"unique", FALSE, 0, 0, 0, 0, 0, 0, 0, 0},

{
     "Dwarf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC | RES_POISON, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Elf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_CHARM, VULN_IRON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Giant", TRUE,
     0, 0, 0,
     0, 0, VULN_MENTAL,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Hobbit", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_MAGIC | RES_POISON, VULN_DROWNING,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Human", TRUE,
     0, 0, 0,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Troll", TRUE,
     0, AFF_REGENERATION | AFF_INFRARED, OFF_BERSERK,
     0, RES_BASH, VULN_MENTAL | VULN_FIRE | VULN_ACID,
     B | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V, FALSE},

    {
     "Wolf", TRUE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | J | K | Q | V, TRUE},

    {
     "Wyvern", TRUE,
     0, AFF_FLYING | AFF_DETECT_INVIS | AFF_DETECT_HIDDEN, OFF_BASH | OFF_FAST | OFF_DODGE,
     IMM_POISON, 0, VULN_LIGHT,
     B | Z | cc, A | C | D | E | F | H | J | K | Q | U | V | X, TRUE},

    {
     "Drow", TRUE,
     0, AFF_DARK_VISION | AFF_INFRARED, 0,
     0, RES_CHARM, VULN_IRON | VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Red Dragon", TRUE,
     0, AFF_FLYING | AFF_INFRARED, OFF_TAIL,
     IMM_FIRE, RES_ENERGY, VULN_COLD,
     H | X | Z, A | C | D | E | F | H | J | K | Q | U | V | X, TRUE},

    {
     "Blue Dragon", TRUE,
     0, AFF_FLYING | AFF_INFRARED, OFF_TAIL,
     IMM_LIGHTNING, RES_ENERGY, VULN_ACID,
     H | X | Z, A | C | D | E | F | H | J | K | Q | U | V | X, TRUE},

    {
     "Half Elf", TRUE,
     0, AFF_INFRARED, 0,
     0, RES_CHARM, VULN_IRON,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, FALSE},

    {
     "Alien", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Badger", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Elephant", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Monkey", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Shadow", TRUE,
     0, AFF_SNEAK | AFF_DARK_VISION, 0,
     IMM_COLD | IMM_CHARM, 0, VULN_LIGHT,
     C | L, A | B | C | D | E | G | H | I | J | K, FALSE},

    {
     "Half Dragon", TRUE,
     0, AFF_FLYING | AFF_INFRARED | AFF_REGENERATION, OFF_TAIL,
     IMM_CHARM, RES_MENTAL | RES_LIGHT, 0,
     C | H | M | Z, A | B | C | D | E | F | G | H | J | K | L | P | Q | U | X, FALSE},

    {
     "Treanti", TRUE,
     0, 0, 0,
     IMM_CHARM | IMM_LIGHT, RES_WEAPON, VULN_FIRE | VULN_LIGHTNING | VULN_DISEASE,
     C | E, A | B | C | D | E | G | I | K | M, FALSE},

    {
     "Gorilla", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Hedgehog", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Unknown", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Tarrasque", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Loverboy", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Cartoon", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "Newbie", TRUE,
     0, 0, OFF_FAST | OFF_TAIL | OFF_KICK,
     IMM_ACID | IMM_POISON, RES_CHARM | RES_DISEASE, VULN_FIRE | VULN_HOLY,
     H | J | M | Q | X | cc, A | B | C | D | E | F | K | L | M | Q | U | V | X, TRUE},

    {
     "bat", FALSE,
     0, AFF_FLYING | AFF_DARK_VISION, OFF_DODGE | OFF_FAST,
     0, 0, VULN_LIGHT,
     A | G | W, A | C | D | E | F | H | J | K | P, TRUE},

    {
     "bear", FALSE,
     0, 0, OFF_CRUSH | OFF_DISARM | OFF_BERSERK,
     0, RES_BASH | RES_COLD, 0,
     A | G | V, A | B | C | D | E | F | H | J | K | U | V, TRUE},

    {
     "cat", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | U | V, TRUE},

    {
     "centipede", FALSE,
     0, AFF_DARK_VISION, 0,
     0, RES_PIERCE | RES_COLD, VULN_BASH, FALSE},
    
    {
     "dog", FALSE,
     0, 0, OFF_FAST,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | U | V, TRUE},

    {
     "doll", FALSE,
     0, 0, 0,
     IMM_MAGIC, RES_BASH | RES_LIGHT, VULN_SLASH | VULN_FIRE | VULN_ACID | VULN_LIGHTNING | VULN_ENERGY,
     E | J | M | cc, A | B | C | G | H | K, TRUE},

    {
     "fido", FALSE,
     0, 0, OFF_DODGE | ASSIST_RACE,
     0, 0, VULN_MAGIC,
     B | G | V, A | C | D | E | F | H | J | K | Q | V, TRUE},

    {
     "fox", FALSE,
     0, AFF_DARK_VISION, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K | Q | V, TRUE},

    {
     "goblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE, VULN_MAGIC,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, TRUE},

    {
     "hobgoblin", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE | RES_POISON, 0,
     0, 0, FALSE},

    {
     "kobold", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_POISON, VULN_MAGIC,
     A | B | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Q, TRUE},

    {
     "lizard", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | X | cc, A | C | D | E | F | H | K | Q | V, TRUE},

    {
     "modron", FALSE,
     0, AFF_INFRARED, ASSIST_RACE | ASSIST_ALIGN,
     IMM_CHARM | IMM_DISEASE | IMM_MENTAL | IMM_HOLY | IMM_NEGATIVE, RES_FIRE | RES_COLD | RES_ACID, 0,
     H, A | B | C | G | H | J | K, TRUE},

    {
     "orc", FALSE,
     0, AFF_INFRARED, 0,
     0, RES_DISEASE, VULN_LIGHT,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, TRUE},

    {
     "pig", FALSE,
     0, 0, 0,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K, TRUE},

    {
     "rabbit", FALSE,
     0, 0, OFF_DODGE | OFF_FAST,
     0, 0, 0,
     A | G | V, A | C | D | E | F | H | J | K, TRUE},

    {
     "sailor", FALSE,
     bb, 0, 0,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, TRUE},

    {
     "school monster", FALSE,
     ACT_NOALIGN, 0, 0,
     IMM_CHARM | IMM_SUMMON, 0, VULN_MAGIC,
     A | M | V, A | B | C | D | E | F | H | J | K | Q | U, FALSE},

    {
     "shiriff", FALSE,
     T, 0, L | P,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, TRUE},

    {
     "snake", FALSE,
     0, 0, 0,
     0, RES_POISON, VULN_COLD,
     A | G | R | X | Y | cc, A | D | E | F | K | L | Q | V | X, TRUE},

    {
     "song bird", FALSE,
     0, AFF_FLYING, OFF_FAST | OFF_DODGE,
     0, 0, 0,
     A | G | W, A | C | D | E | F | H | K | P, FALSE},

    {
     "thain", FALSE,
     T, 0, L | P,
     0, 0, 0,
     A | H | M | V, A | B | C | D | E | F | G | H | I | J | K, TRUE},

    {
     "water fowl", FALSE,
     0, AFF_SWIM | AFF_FLYING, 0,
     0, RES_DROWNING, 0,
     A | G | W, A | C | D | E | F | H | K | P, FALSE},

    {
     NULL, 0, 0, 0, 0, 0, 0}
};

/* Race Table for stats of 25 */
#if MAX_ATTAINABLE_STATS == 25
const struct pc_race_type pc_race_table[] = {
    {"null race", "", 0, {100, 100, 100, 100, 100, 100, 100, 100},
     {""}, {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, 0, 17},

/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills },
	{ base stats },		{ max stats },		size 
    },
*/

    {
     "dwarf","Dwarf",10,{200, 100, 125, 100, 100, 125, 150, 200},
     {"berserk", "axe"},
     {1, -1, 1, -3, 2},{20, 16, 19, 14, 21},SIZE_MEDIUM,{45,80}},

    {
     "elf","Elf",5,{100, 125, 100, 120, 110, 125, 100, 100},
     {"sneak", "hide"},
     {-1, 1, 0, 2, -2},{16, 20, 17, 21, 15},SIZE_MEDIUM, {100,150}},

    {
     "giant","Giant",6,{200, 150, 150, 100, 150, 150, 200, 200},
     {"bash"},
     {3, -2, 0, -2, 1},{22, 15, 17, 16, 20},SIZE_HUGE, {15,30}},

    {
     "hobbit","Hobbit",5,{125, 125, 100, 150, 150, 150, 100, 110},
     {"sneak", "hide"},
     {-1, -1, 0, 2, 1},{16, 16, 17, 22, 19},SIZE_MEDIUM,{20,40}},

    {
     "human","Human",0,{100, 100, 100, 100, 100, 100, 100, 100},
     {""},
     {0, 0, 0, 0, 0},{18, 18, 18, 18, 18},SIZE_MEDIUM,{17,30}},

    {
     "troll","Troll",12,{175, 150, 150, 100, 150, 120, 150, 200},
     {""},
     {2, -2, -1, -2, 3},{20, 15, 16, 15, 22},SIZE_MEDIUM, {20,35}},

    {
     "wolf","Wolf",9,{200, 200, 100, 100, 150, 125, 110, 200},
     {"sneak", "dodge", "fast healing"},
     {0, -2, -2, 3, 1},{18, 15, 16, 22, 19},SIZE_MEDIUM, {3,14}},

    {
     "wyvern","Wyvern",20,{125, 125, 175, 100, 150, 100, 175, 125},
     {"bash", "fast healing"},
     {2, -2, 0, -2, 2},{22, 18, 18, 12, 20},SIZE_HUGE, {5,40}},

    {
     "drow","Drow",7,{100, 125, 100, 120, 200, 110, 100, 100},
     {"sneak", "hide", "faerie fire", "detect magic"},
     {-1, 1, 0, 2, -2},{16, 20, 18, 21, 15},SIZE_MEDIUM,{100,150}},

    {
     "red dragon","Red Dragon",15,{125, 150, 125, 100, 150, 100, 150, 150},
     {"combat"},
     {4, -2, 0, -2, 3},{24, 16, 18, 16, 24},SIZE_MEDIUM,{70,130}},

    {
     "blue dragon","Blue Dragon",12,{100, 100, 120, 120, 110, 150, 150, 100},
     {"combat"},
     {2, 0, 2, -2, 2},{22, 18, 21, 16, 22},SIZE_MEDIUM,{70,130}},

    {
     "half elf","Half Elf",5,{110, 110, 110, 110, 110, 110, 110, 110},
     {"hide"},
     {0, 0, 0, 1, -1},{18, 18, 18, 19, 17},SIZE_MEDIUM,{17,60}},

    {
     "alien","Alien",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM,{6,20}},

    {
     "badger","Badger",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM,{3,12}},

    {
     "elephant","Elephant",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM,{18,30}},

    {
     "monkey","Monkey",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM,{4,8}},

    {
     "shadow","Shadow",9,{150, 175, 100, 175, 175, 175, 100, 150},
     {"invis", "hide", "chill touch", "energy drain"},
     {-2, 0, 0, 3, -1},{15, 18, 18, 22, 15},SIZE_MEDIUM,{50,200}},

    {
     "half dragon","Half Dragon",13,{125, 125, 175, 100, 125, 125, 175, 125},
     {"hand to hand"},
     {2, -1, 0, -1, 1},{20, 16, 18, 16, 20},SIZE_MEDIUM,{20,60}},

    {
     "treanti","Treanti",12,{175, 110, 175, 100, 100, 175, 175, 175},
     {"fast healing"},
     {2, 0, -1, -4, 3},{20, 18, 16, 12, 22},SIZE_MEDIUM,{60,300}},

    {
     "gorilla","Gorilla",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM,{6,15}},

    {
     "hedgehog","Hedgehog",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM,{4,9}},

    {
     "unknown","Unknown",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "tarrasque","Tarrasque",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM,{25,40}},

    {
     "loverboy","Loverboy",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM,{16,23}},

    {
     "cartoon","Cartoon",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_LARGE,{3,5}},

    {
     "newbie","Newbie",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_LARGE,{1,2}},

};
#endif

#if MAX_ATTAINABLE_STATS == 30
// JR: Right now this is the same as for 25
const struct pc_race_type pc_race_table[] = {
    {"null race", "", 0, {100, 100, 100, 100, 100, 100, 100, 100},
     {""}, {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, 0},

/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills },
	{ base stats },		{ max stats },		size 
    },
*/

    {
     "dwarf","Dwarf",10,{200, 100, 125, 100, 100, 125, 150, 200},
     {"berserk", "axe"},
     {1, -1, 1, -3, 2},{20, 16, 19, 14, 21},SIZE_MEDIUM},

    {
     "elf","Elf",5,{100, 125, 100, 120, 110, 125, 100, 100},
     {"sneak", "hide"},
     {-1, 1, 0, 2, -2},{16, 20, 17, 21, 15},SIZE_MEDIUM},

    {
     "giant","Giant",6,{200, 150, 150, 100, 150, 150, 200, 200},
     {"bash", "fast healing"},
     {3, -2, 0, -2, 1},{22, 15, 17, 16, 20},SIZE_HUGE},

    {
     "hobbit","Hobbit",5,{125, 125, 100, 150, 150, 150, 100, 110},
     {"sneak", "hide"},
     {-1, -1, 0, 2, 1},{16, 16, 17, 22, 19},SIZE_MEDIUM},

    {
     "human","Human",0,{100, 100, 100, 100, 100, 100, 100, 100},
     {""},
     {0, 0, 0, 0, 0},{18, 18, 18, 18, 18},SIZE_MEDIUM},

    {
     "troll","Troll",12,{175, 150, 150, 100, 150, 120, 150, 200},
     {""},
     {2, -2, -1, -2, 3},{20, 15, 16, 15, 22},SIZE_MEDIUM},

    {
     "wolf","Wolf",9,{200, 200, 100, 100, 150, 125, 110, 200},
     {"sneak", "dodge", "fast healing"},
     {0, -2, -2, 3, 1},{18, 15, 16, 22, 19},SIZE_MEDIUM},

    {
     "wyvern","Wyvern",20,{125, 125, 175, 100, 150, 100, 175, 125},
     {"bash", "fast healing"},
     {2, -2, 0, -2, 2},{22, 18, 18, 12, 20},SIZE_HUGE},

    {
     "drow","Drow",7,{100, 125, 100, 120, 200, 110, 100, 100},
     {"sneak", "hide", "faerie fire", "detect magic"},
     {-1, 1, 0, 2, -2},{16, 20, 18, 21, 15},SIZE_MEDIUM},

    {
     "red dragon","Red Dragon",15,{125, 150, 125, 100, 150, 100, 150, 150},
     {"combat"},
     {4, -2, 0, -2, 3},{24, 16, 18, 16, 24},SIZE_MEDIUM},

    {
     "blue dragon","Blue Dragon",12,{100, 100, 120, 120, 110, 150, 150, 100},
     {"combat"},
     {2, 0, 2, -2, 2},{22, 18, 21, 16, 22},SIZE_MEDIUM},

    {
     "half elf","Half Elf",5,{110, 110, 110, 110, 110, 110, 110, 110},
     {"hide"},
     {0, 0, 0, 1, -1},{18, 18, 18, 19, 17},SIZE_MEDIUM},

    {
     "alien","Alien",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "badger","Badger",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "elephant","Elephant",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "monkey","Monkey",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "shadow","Shadow",9,{150, 175, 100, 175, 175, 175, 100, 150},
     {"invis", "hide", "chill touch", "energy drain"},
     {-2, 0, 0, 3, -1},{15, 18, 18, 22, 15},SIZE_MEDIUM},

    {
     "half dragon","Half Dragon",13,{125, 125, 175, 100, 125, 125, 175, 125},
     {"hand to hand"},
     {2, -1, 0, -1, 1},{20, 16, 18, 16, 20},SIZE_MEDIUM},

    {
     "treanti","Treanti",12,{175, 110, 175, 100, 100, 175, 175, 175},
     {"fast healing"},
     {2, 0, -1, -4, 3},{20, 18, 16, 12, 22},SIZE_MEDIUM},

    {
     "gorilla","Gorilla",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "hedgehog","Hedgehog",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "unknown","Unknown",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "tarrasque","Tarrasque",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "loverboy","Loverboy",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_MEDIUM},

    {
     "cartoon","Cartoon",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_LARGE},

    {
     "newbie","Newbie",20,{125, 125, 125, 110, 125, 125, 125, 125},
     {"protection"},
     {3, 2, 2, 2, 2},{23, 22, 22, 22, 22},SIZE_LARGE},

};
#endif

/*
 * Class table.
 */
/* VERY IMPORTANT NOTE:
   If you insert a new class, PUT IT AT THE END!
   If you don't the mud won't crash but will certainly get
   VERY screwed up.  

   Another very important thing:
   Don't forget the console class! If you add any more classes,
   you NEED to remember that the console class is the fifth one
   but you can probably just put zeroes for all its values.

   Kyle
 */
#if defined(cbuilder)
struct class_type class_table[MAX_CLASS + 1] =
#else
struct class_type class_table[MAX_CLASS] =
#endif
{
    {
     "Mage", "Mag", STAT_INT, OBJ_VNUM_SCHOOL_DAGGER,
     {3018, 9618}, 75, 18, 5, 6, 9, TRUE,
     "mage basics", "mage default", FALSE},

    {
     "Cleric", "Cle", STAT_WIS, OBJ_VNUM_SCHOOL_MACE,
     {3003, 9619}, 75, 18, 2, 8, 10, TRUE,
     "cleric basics", "cleric default", FALSE},

    {
     "Thief", "Thi", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,
     {3028, 9639}, 75, 18, -4, 8, 13, FALSE,
     "thief basics", "thief default", FALSE},

    {
     "Warrior", "War", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,
     {3022, 9633}, 75, 18, -10, 11, 15, FALSE,
     "warrior basics", "warrior default", FALSE},
    
    {
     "Paladin", "Pal",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
     {3022, 9633}, 75, 18, -8, 10, 14, TRUE,
     "paladin basics", "paladin default", FALSE},

    {
     "Avenger", "Avn",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
     {3022, 9633},  75, 18, -6, 9, 14, TRUE,
     "avenger basics", "avenger default", FALSE},

    {
     "Nightblade", "Nbl",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
     {3028, 9639},  75, 18, 0, 7, 10, TRUE,
     "nightblade basics", "nightblade default", FALSE},
    
    {
     "Mystic", "Mys",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
     {3018, 9618},  80, 20, 12, 4, 6, TRUE,
     "mystic basics", "mystic default", FALSE}
    
    #if defined(cbuilder)
    /* Class added solely for Win32 GUI Console Administration. -Zane */
    ,{
     "Console", "Con", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,
     {3022, 9633}, 75, 18, -10, 11, 15, FALSE,
     "warrior basics", "warrior default", FALSE}
#endif
};

/*
 * Attribute bonus tables. If you changed MAX_ATTAINABLE_STATS to a number
HIGHER then 30, you MUST edit these tables to make the higher stats affect
anything
 */
#if MAX_ATTAINABLE_STATS == 30
const struct str_app_type str_app[MAX_ATTAINABLE_STATS + 1] = {
    {-5, -4, 0, 0},             /* 0  */
    {-5, -4, 3, 1},             /* 1  */
    {-3, -2, 3, 2},
    {-3, -1, 10, 3},            /* 3  */
    {-2, -1, 25, 4},
    {-2, -1, 55, 5},            /* 5  */
    {-1, 0, 80, 6},
    {-1, 0, 90, 7},
    {0, 0, 100, 8},
    {0, 0, 100, 9},
    {0, 0, 115, 10},            /* 10  */
    {0, 0, 115, 11},
    {0, 0, 130, 12},
    {0, 0, 130, 13},            /* 13  */
    {0, 1, 140, 14},
    {1, 1, 150, 15},            /* 15  */
    {1, 2, 165, 16},
    {2, 3, 180, 22},
    {2, 3, 200, 25},            /* 18  */
    {3, 4, 225, 30},
    {3, 5, 250, 35},            /* 20  */
    {4, 6, 300, 40},
    {4, 6, 350, 45},
    {5, 7, 400, 50},
    {5, 8, 450, 55},
    {6, 9, 500, 60},            /* 25 Default Attainable  */
    {6, 9, 550, 65},
    {7, 10, 600, 70},
    {7, 10, 650, 80},
    {8, 11, 700, 85},
    {10, 13, 1000, 90}          /* 30 */

};
#else                           /* Stats table for MAX_ATTAINABLE_STATS = 25 */
const struct str_app_type str_app[MAX_ATTAINABLE_STATS + 1] = {
    {-5, -4, 0, 0},             /* 0  */
    {-5, -4, 3, 1},             /* 1  */
    {-3, -2, 3, 2},
    {-3, -1, 10, 3},            /* 3  */
    {-2, -1, 25, 4},
    {-2, -1, 55, 5},            /* 5  */
    {-1, 0, 80, 6},
    {-1, 0, 90, 7},
    {0, 0, 100, 8},
    {0, 0, 100, 9},
    {0, 0, 115, 10},            /* 10  */
    {0, 0, 115, 11},
    {0, 0, 130, 12},
    {0, 0, 130, 13},            /* 13  */
    {0, 1, 140, 14},
    {1, 1, 150, 15},            /* 15  */
    {1, 2, 165, 16},
    {2, 3, 180, 22},
    {2, 3, 200, 25},            /* 18  */
    {3, 4, 225, 30},
    {3, 5, 250, 35},            /* 20  */
    {4, 6, 300, 40},
    {4, 6, 350, 45},
    {5, 7, 400, 50},
    {5, 8, 450, 55},
    {6, 9, 500, 60}             /* 25 Default Attainable  */
};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct int_app_type int_app[MAX_ATTAINABLE_STATS + 1] = {
    {3},                        /*  0 */
    {5},                        /*  1 */
    {7},
    {8},                        /*  3 */
    {9},
    {10},                       /*  5 */
    {11},
    {12},
    {13},
    {15},
    {17},                       /* 10 */
    {19},
    {22},
    {25},
    {28},
    {31},                       /* 15 */
    {34},
    {37},
    {40},                       /* 18 */
    {44},
    {49},                       /* 20 */
    {55},
    {60},
    {70},
    {80},
    {85},                       /* 25 Default attainable */
    {87},
    {89},
    {91},
    {92},
    {95}                        /* 30 */
};
#else
const struct int_app_type int_app[MAX_ATTAINABLE_STATS + 1] = {
    {3},                        /*  0 */
    {5},                        /*  1 */
    {7},
    {8},                        /*  3 */
    {9},
    {10},                       /*  5 */
    {11},
    {12},
    {13},
    {15},
    {17},                       /* 10 */
    {19},
    {22},
    {25},
    {28},
    {31},                       /* 15 */
    {34},
    {37},
    {40},                       /* 18 */
    {44},
    {49},                       /* 20 */
    {55},
    {60},
    {70},
    {80},
    {85}                        /* 25 Default attainable */
};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct wis_app_type wis_app[MAX_ATTAINABLE_STATS + 1] = {
    {0},                        /*  0 */
    {0},                        /*  1 */
    {0},
    {0},                        /*  3 */
    {0},
    {1},                        /*  5 */
    {1},
    {1},
    {1},
    {1},
    {2},                        /* 10 */
    {2},
    {2},
    {3},
    {3},
    {3},                        /* 15 */
    {3},
    {3},
    {4},                        /* 18 */
    {4},
    {4},                        /* 20 */
    {4},
    {5},
    {5},
    {5},
    {6},                        /* 25 */
    {7},
    {8},
    {9},
    {10},
    {15}                        /* 30 */
};
#else
const struct wis_app_type wis_app[MAX_ATTAINABLE_STATS + 1] = {
    {0},                        /*  0 */
    {0},                        /*  1 */
    {0},
    {0},                        /*  3 */
    {0},
    {1},                        /*  5 */
    {1},
    {1},
    {1},
    {1},
    {2},                        /* 10 */
    {2},
    {2},
    {3},
    {3},
    {3},                        /* 15 */
    {3},
    {3},
    {4},                        /* 18 */
    {4},
    {4},                        /* 20 */
    {4},
    {5},
    {5},
    {5},
    {6}                         /* 25 */
};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct dex_app_type dex_app[MAX_ATTAINABLE_STATS + 1] = {
    {60},                       /* 0 */
    {50},                       /* 1 */
    {50},
    {40},
    {30},
    {20},                       /* 5 */
    {10},
    {0},
    {0},
    {0},
    {0},                        /* 10 */
    {0},
    {0},
    {0},
    {0},
    {-10},                      /* 15 */
    {-15},
    {-20},
    {-30},
    {-40},
    {-50},                      /* 20 */
    {-60},
    {-75},
    {-90},
    {-105},
    {-120},                     /* 25 Default Attainable */
    {-125},
    {-130},
    {-140},
    {-150},
    {-200}                      /* 30 */
};
#else
const struct dex_app_type dex_app[MAX_ATTAINABLE_STATS + 1] = {
    {60},                       /* 0 */
    {50},                       /* 1 */
    {50},
    {40},
    {30},
    {20},                       /* 5 */
    {10},
    {0},
    {0},
    {0},
    {0},                        /* 10 */
    {0},
    {0},
    {0},
    {0},
    {-10},                      /* 15 */
    {-15},
    {-20},
    {-30},
    {-40},
    {-50},                      /* 20 */
    {-60},
    {-75},
    {-90},
    {-105},
    {-120},                     /* 25 Default Attainable */
};
#endif

#if MAX_ATTAINABLE_STATS == 30
const struct con_app_type con_app[MAX_ATTAINABLE_STATS + 1] = {
    {-4, 20},                   /*  0 */
    {-3, 25},                   /*  1 */
    {-2, 30},
    {-2, 35},                   /*  3 */
    {-1, 40},
    {-1, 45},                   /*  5 */
    {-1, 50},
    {0, 55},
    {0, 60},
    {0, 65},
    {0, 70},                    /* 10 */
    {0, 75},
    {0, 80},
    {0, 85},
    {0, 88},
    {1, 90},                    /* 15 */
    {2, 95},
    {2, 97},
    {3, 99},                    /* 18 */
    {3, 99},
    {4, 99},                    /* 20 */
    {4, 99},
    {5, 99},
    {6, 99},
    {7, 99},
    {8, 99},                    /* 25 Default attainable */
    {9, 99},
    {10, 99},
    {11, 99},
    {12, 99},
    {18, 99}                    /* 30 */
};
#else
const struct con_app_type con_app[MAX_ATTAINABLE_STATS + 1] = {
    {-4, 20},                   /*  0 */
    {-3, 25},                   /*  1 */
    {-2, 30},
    {-2, 35},                   /*  3 */
    {-1, 40},
    {-1, 45},                   /*  5 */
    {-1, 50},
    {0, 55},
    {0, 60},
    {0, 65},
    {0, 70},                    /* 10 */
    {0, 75},
    {0, 80},
    {0, 85},
    {0, 88},
    {1, 90},                    /* 15 */
    {2, 95},
    {2, 97},
    {3, 99},                    /* 18 */
    {3, 99},
    {4, 99},                    /* 20 */
    {4, 99},
    {5, 99},
    {6, 99},
    {7, 99},
    {8, 99}                     /* 25 Default attainable */
};
#endif

/*
 * Liquid properties.
 * Used in world.obj.
 */
const struct liq_type liq_table[LIQ_MAX] = {
    {"water", "clear", {0, 1, 10}}, /*  0 */
    {"beer", "amber", {3, 2, 5}},
    {"wine", "rose", {5, 2, 5}},
    {"ale", "brown", {2, 2, 5}},
    {"dark ale", "dark", {1, 2, 5}},

    {"whisky", "golden", {6, 1, 4}},    /*  5 */
    {"lemonade", "pink", {0, 1, 8}},
    {"firebreather", "boiling", {10, 0, 0}},
    {"local specialty", "everclear", {3, 3, 3}},
    {"slime mold juice", "green", {0, 4, -8}},

    {"milk", "white", {0, 3, 6}},   /* 10 */
    {"tea", "tan", {0, 1, 6}},
    {"coffee", "black", {0, 1, 6}},
    {"blood", "red", {0, 2, -1}},
    {"salt water", "clear", {0, 1, -2}},

    {"cola", "cherry", {0, 1, 5}}   /* 15 */
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

struct skill_type skill_table[MAX_SKILL] = {   /* The two lists in each entry are level and CP. But these get overwritten by Class */

/*
 * Magic spells.
 */
/*
    {
	"reserved",		{ 999, 999, 999, 999 },	{ 99, 99, 99, 99},
	spell_null,			TAR_IGNORE, POS_STANDING,
	NULL,			SLOT( 0),	 0,	 0,
	"",			""
    },*/

    {
     "acid blast", {25, 93, 93, 93, 93, 93, 93, 25},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_acid_blast, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(70), 20, 12,
     "acid blast", "!Acid Blast!"},

    {
     "armor", {1, 93, 93, 93, 93, 93, 93, 1},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_armor, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(1), 5, 0,
     "", "You feel less protected."},

    {
     "bark skin", {93, 17, 93, 93, 93, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_bark_skin, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(512), 12, 0,
     "", "Your skin feels normal again."},

    {
     "bless", {93, 1, 93, 93, 2, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_bless, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(3), 5, 0,
     "", "You feel less righteous."},

    {
     "blindness", {9, 9, 93, 93, 93, 7, 6, 9},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_blindness, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, &gsn_blindness, SLOT(4), 5, 12,
     "", "You can see again."},

    {
     "burning hands", {3, 93, 93, 93, 93, 93, 7, 3},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_burning_hands, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(5), 15, 12,
     "burning hands", "!Burning Hands!"},

    {
     "call lightning", {93, 27, 93, 93, 33, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_call_lightning, TAR_IGNORE,
     POS_FIGHTING, NULL, SLOT(6), 15, 12,
     "lightning bolt", "!Call Lightning!"},

    {
     "calm", {93, 13, 93, 93, 18, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_calm, TAR_IGNORE,
     POS_FIGHTING, NULL, SLOT(509), 30, 4,
     "", "You have lost your peace of mind."},

    {
     "cancellation", {14, 10, 93, 93, 12, 93, 93, 14},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cancellation, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(507), 20, 12,
     """!cancellation!", },

    {
     "cause critical", {93, 16, 93, 93, 93, 25, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cause_critical, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(63), 20, 12,
     "spell", "!Cause Critical!"},

    {
     "cause light", {93, 1, 93, 93, 93, 4, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cause_light, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(62), 15, 12,
     "spell", "!Cause Light!"},

    {
     "cause serious", {93, 8, 93, 93, 93, 15, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cause_serious, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(64), 17, 12,
     "spell", "!Cause Serious!"},

    {
     "chain lightning", {28, 93, 93, 93, 93, 93, 93, 28},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_chain_lightning, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(500), 25, 12,
     "lightning", "!Chain Lightning!"},

    {
     "change sex", {32, 48, 93, 93, 93, 30, 93, 32},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_change_sex, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(82), 15, 0,
     "", "Your body feels familiar again."},

    {
     "charm person", {6, 93, 15, 93, 93, 93, 8, 6},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_charm_person, TAR_CHAR_OFFENSIVE,
     POS_STANDING, &gsn_charm_person, SLOT(7), 5, 12,
     "", "You feel more self-confident."},

    {
     "chill touch", {5, 23, 23, 23, 23, 23, 11, 5},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_chill_touch, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(8), 15, 12,
     "chilling touch", "You feel less cold."},

    {
     "colour spray", {11, 90, 40, 40, 40, 40, 25, 11},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_colour_spray, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(10), 15, 12,
     "colour spray", "!Colour Spray!"},

    {
     "continual light", {12, 6, 93, 93, 10, 93, 93, 12},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_continual_light, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(57), 7, 0,
     "", "!Continual Light!"},

    {
     "control weather", {13, 18, 93, 93, 93, 93, 93, 13},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_control_weather, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(11), 25, 12,
     "", "!Control Weather!"},

    {
     "create food", {4, 2, 93, 93, 3, 93, 93, 4},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_create_food, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(12), 5, 0,
     "", "!Create Food!"},

    {
     "create spring", {10, 8, 93, 93, 9, 93, 93, 10},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_create_spring, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(80), 20, 0,
     "", "!Create Spring!"},

    {
     "create water", {4, 2, 93, 93, 3, 93, 93, 4},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_create_water, TAR_OBJ_INV,
     POS_STANDING, NULL, SLOT(13), 5, 0,
     "", "!Create Water!"},

    {
     "cure blindness", {93, 4, 93, 93, 5, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cure_blindness, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(14), 5, 0,
     "", "!Cure Blindness!"},

    {
     "cure critical", {93, 16, 93, 93, 25, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cure_critical, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(15), 20, 0,
     "", "!Cure Critical!"},

    {
     "cure disease", {93, 4, 93, 93, 5, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cure_disease, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(501), 20, 0,
     "", "!Cure Disease!"},

    {
     "cure light", {93, 1, 93, 93, 4, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cure_light, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(16), 10, 0,
     "", "!Cure Light!"},

    {
     "cure poison", {93, 4, 93, 93, 5, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cure_poison, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(43), 5, 0,
     "", "!Cure Poison!"},

    {
     "cure serious", {93, 8, 98, 93, 15, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_cure_serious, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(61), 15, 0,
     "", "!Cure Serious!"},

    {
     "curse", {22, 15, 93, 93, 93, 12, 20, 22},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_curse, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, &gsn_curse, SLOT(17), 20, 12,
     "curse", "The curse wears off."},

    {
     "demonfire", {40, 33, 93, 93, 93, 37, 93, 40},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_demonfire, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(505), 20, 12,
     "torments", "!Demonfire!"},

    {
     "detect evil", {4, 2, 93, 93, 3, 93, 1, 4},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_detect_evil, TAR_CHAR_SELF,
     POS_STANDING, NULL, SLOT(18), 5, 0,
     "", "The red in your vision disappears."},

    {
     "detect hidden", {6, 8, 93, 93, 10, 93, 1, 6},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_detect_hidden, TAR_CHAR_SELF,
     POS_STANDING, NULL, SLOT(44), 5, 0,
     "", "You feel less aware of your suroundings."},

    {
     "detect invis", {7, 9, 93, 93, 11, 93, 1, 7},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_detect_invis, TAR_CHAR_SELF,
     POS_STANDING, NULL, SLOT(19), 5, 0,
     "", "You no longer see invisible objects."},

    {
     "detect magic", {1, 5, 93, 93, 9, 93, 1, 1},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_detect_magic, TAR_CHAR_SELF,
     POS_STANDING, NULL, SLOT(20), 5, 0,
     "", "The detect magic wears off."},

    {
     "detect poison", {3, 5, 93, 93, 7, 93, 1, 3},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_detect_poison, TAR_OBJ_INV,
     POS_STANDING, NULL, SLOT(21), 5, 0,
     "", "!Detect Poison!"},

    {
     "dispel evil", {25, 13, 93, 93, 93, 93, 93, 25},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_dispel_evil, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(22), 15, 12,
     "dispel evil", "!Dispel Evil!"},

    {
     "dispel magic", {18, 22, 93, 93, 93, 93, 93, 18},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_dispel_magic, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(59), 15, 12,
     "", "!Dispel Magic!"},

    {
     "earthquake", {20, 21, 93, 93, 93, 25, 93, 20},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_earthquake, TAR_IGNORE,
     POS_FIGHTING, NULL, SLOT(23), 15, 12,
     "earthquake", "!Earthquake!"},

    {
     "enchant armor", {14, 55, 90, 90, 90, 90, 90, 14},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_enchant_armor, TAR_OBJ_INV,
     POS_STANDING, NULL, SLOT(510), 100, 0,
     "", "!Enchant Armor!"},

    {
     "enchant weapon", {15, 60, 90, 90, 90, 90, 90, 15},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_enchant_weapon, TAR_OBJ_INV,
     POS_STANDING, NULL, SLOT(24), 100, 0,
     "", "!Enchant Weapon!"},

    {
     "energy drain", {23, 25, 22, 93, 93, 33, 19, 23},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_energy_drain, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(25), 35, 12,
     "energy drain", "!Energy Drain!"},

    {
     "faerie fire", {10, 93, 33, 93, 93, 15, 10, 10},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_faerie_fire, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(72), 5, 12,
     "faerie fire", "The pink aura around you fades away."},

    {
     "faerie fog", {6, 93, 93, 93, 93, 93, 93, 6},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_faerie_fog, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(73), 12, 12,
     "faerie fog", "!Faerie Fog!"},

    {
     "fireball", {17, 93, 93, 93, 93, 93, 93, 17},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_fireball, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(26), 15, 12,
     "fireball", "!Fireball!"},

    {
     "flamestrike", {93, 19, 93, 93, 93, 23, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_flamestrike, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(65), 20, 12,
     "flamestrike", "!Flamestrike!"},

    {
     "fly", {17, 93, 93, 93, 93, 93, 93, 17},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_fly, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(56), 10, 0,
     "", "You slowly float to the ground."},

    {
     "frenzy", {93, 40, 93, 93, 32, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_frenzy, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(504), 30, 24,
     "", "Your rage ebbs."},

    {
     "gate", {37, 93, 93, 93, 93, 93, 93, 37},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_gate, TAR_IGNORE,
     POS_FIGHTING, NULL, SLOT(83), 80, 0,
     "", "!Gate!"},

    {
     "giant strength", {10, 93, 93, 93, 93, 20, 33, 10},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_giant_strength, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(39), 20, 0,
     "", "You feel weaker."},

    {
     "harm", {93, 23, 93, 93, 93, 35, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_harm, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(27), 35, 12,
     "harm spell", "!Harm!"},

    {
     "haste", {21, 93, 93, 93, 93, 40, 18, 21},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_haste, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(502), 30, 0,
     "", "You feel yourself slow down."},

    {
     "heal", {93, 23, 93, 93, 35, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_heal, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(28), 50, 0,
     "", "!Heal!"},

    {
     "holy word", {93, 50, 93, 93, 93, 93, 93, 93},
     {2, 2, 4, 4, 4, 4, 4, 2}, spell_holy_word, TAR_IGNORE,
     POS_FIGHTING, NULL, SLOT(506), 200, 24,
     "divine wrath", "!Holy Word!"},

    {
     "identify", {12, 18, 93, 93, 93, 30, 15, 12},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_identify, TAR_OBJ_INV,
     POS_STANDING, NULL, SLOT(53), 12, 0,
     "", "!Identify!"},

    {
     "infravision", {2, 93, 93, 93, 93, 5, 2, 2},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_infravision, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(77), 5, 0,
     "", "You no longer see in the dark."},

    {
     "invis", {12, 93, 25, 93, 93, 25, 14, 12},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_invis, TAR_CHAR_DEFENSIVE,
     POS_STANDING, &gsn_invis, SLOT(29), 5, 0,
     "", "You are no longer invisible."},

    {
     "know alignment", {5, 3, 93, 93, 4, 93, 4, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_know_alignment, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(58), 9, 0,
     "", "!Know Alignment!"},

    {
     "lightning bolt", {13, 93, 93, 93, 93, 93, 93, 13},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_lightning_bolt, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(30), 15, 12,
     "lightning bolt", "!Lightning Bolt!"},

    {
     "locate object", {9, 12, 93, 93, 15, 93, 10, 9},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_locate_object, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(31), 20, 0,
     "", "!Locate Object!"},

    {
     "magic missile", {1, 93, 93, 93, 93, 93, 3, 1},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_magic_missile, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(32), 15, 12,
     "magic missile", "!Magic Missile!"},

    {
     "mass healing", {93, 28, 93, 93, 50, 93, 93, 93},
     {2, 2, 4, 4, 4, 4, 4, 2}, spell_mass_healing, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(508), 100, 0,
     "", "!Mass Healing!"},

    {
     "mass invis", {24, 93, 31, 93, 93, 93, 93, 24},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_mass_invis, TAR_IGNORE,
     POS_STANDING, &gsn_mass_invis, SLOT(69), 20, 0,
     "", "!Mass Invis!"},

    {
     "pass door", {29, 93, 93, 93, 93, 93, 45, 29},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_pass_door, TAR_CHAR_SELF,
     POS_STANDING, NULL, SLOT(74), 20, 0,
     "", "You feel solid again."},

    {
     "plague", {19, 15, 93, 93, 99, 13, 93, 19},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_plague, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, &gsn_plague, SLOT(503), 20, 12,
     "sickness", "Your sores vanish."},

    {
     "poison", {11, 93, 93, 93, 99, 93, 2, 11},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_poison, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, &gsn_poison, SLOT(33), 10, 12,
     "poison", "You feel less sick."},

    {
     "protection evil", {16, 11, 93, 93, 13, 93, 93, 16},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_protection, TAR_CHAR_SELF,
     POS_STANDING, NULL, SLOT(34), 5, 0,
     "", "You feel less protected."},

    {
     "refresh", {5, 3, 93, 93, 6, 93, 3, 5},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_refresh, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(81), 12, 0,
     "refresh", "!Refresh!"},

    {
     "remove curse", {93, 16, 93, 93, 23, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_remove_curse, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(35), 5, 0,
     "", "!Remove Curse!"},

    {
     "sanctuary", {31, 24, 93, 93, 27, 93, 93, 31},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_sanctuary, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(36), 75, 0,
     "", "The white aura around your body fades."},

    {
     "shield", {19, 93, 93, 93, 93, 93, 93, 19},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_shield, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(67), 12, 0,
     "", "Your force shield shimmers then fades away."},

    {
     "shocking grasp", {7, 93, 93, 93, 93, 93, 17, 7},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_shocking_grasp, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(53), 15, 12,
     "shocking grasp", "!Shocking Grasp!"},

    {
     "sleep", {8, 93, 20, 93, 93, 93, 10, 8},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_sleep, TAR_CHAR_OFFENSIVE,
     POS_STANDING, &gsn_sleep, SLOT(38), 15, 12,
     "", "You feel less tired."},

    {
     "stone skin", {26, 75, 93, 93, 93, 93, 93, 26},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_stone_skin, TAR_CHAR_SELF,
     POS_STANDING, NULL, SLOT(66), 12, 0,
     "", "Your skin feels soft again."},

    {
     "summon", {33, 93, 93, 93, 93, 93, 93, 33},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_summon, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(40), 50, 12,
     "", "!Summon!"},

    {
     "teleport", {27, 93, 93, 93, 93, 93, 40, 27},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_teleport, TAR_CHAR_SELF,
     POS_FIGHTING, NULL, SLOT(2), 35, 0,
     "", "!Teleport!"},

    {
     "ventriloquate", {3, 93, 7, 93, 93, 93, 5, 3},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_ventriloquate, TAR_IGNORE,
     POS_STANDING, NULL, SLOT(41), 5, 0,
     "", "!Ventriloquate!"},

    {
     "weaken", {16, 20, 93, 93, 93, 18, 17, 16},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_weaken, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(68), 20, 12,
     "spell", "You feel stronger."},

    {
     "word of recall", {93, 93, 93, 93, 93, 93, 93, 93},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_word_of_recall, TAR_CHAR_SELF,
     POS_RESTING, NULL, SLOT(42), 5, 0,
     "", "!Word of Recall!"},

    {
     "acid breath", {45, 90, 90, 90, 90, 90, 90, 45},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_acid_breath, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(200), 0, 4,
     "blast of acid", "!Acid Breath!"},

    {
     "fire breath", {47, 90, 90, 90, 90, 90, 90, 47},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_fire_breath, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(201), 0, 4,
     "blast of flame", "!Fire Breath!"},

    {
     "frost breath", {40, 90, 90, 90, 90, 90, 90, 40},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_frost_breath, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(202), 0, 4,
     "blast of frost", "!Frost Breath!"},

    {
     "gas breath", {50, 90, 90, 90, 90, 90, 90, 50},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_gas_breath, TAR_IGNORE,
     POS_FIGHTING, NULL, SLOT(203), 0, 4,
     "blast of gas", "!Gas Breath!"},

    {
     "lightning breath", {42, 90, 90, 90, 90, 90, 90, 42},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_lightning_breath, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(204), 0, 4,
     "blast of lightning", "!Lightning Breath!"},

    {
     "general purpose", {90, 90, 90, 90, 90, 90, 90, 90},
     {0, 0, 0, 0, 0, 0, 0, 0}, spell_general_purpose, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(401), 0, 12,
     "general purpose ammo", "!General Purpose Ammo!"},

    {
     "high explosive", {90, 90, 90, 90, 90, 90, 90, 90},
     {0, 0, 0, 0, 0, 0, 0, 0}, spell_high_explosive, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(402), 0, 12,
     "high explosive ammo", "!High Explosive Ammo!"},

    {
     "firewind", {35, 90, 90, 90, 90, 90, 90, 35},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_firewind, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(300), 25, 12,
     "flaming winds", "!Firewind"},

    {
     "meteor swarm", {37, 90, 90, 90, 90, 90, 90, 37},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_meteor_swarm, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(301), 25, 12,
     "fireball", "!Fireball"},

    {
     "multi missile", {9, 90, 90, 90, 90, 90, 90, 7},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_multi_missile, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(302), 25, 12,
     "magic missile", "!Missile"},

    {
     "disintegrate ", {33, 90, 90, 90, 90, 90, 90, 33},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_disintegrate, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(303), 25, 12,
     "energy blast", "!Disint"},

    {
     "ice ray", {30, 90, 90, 90, 90, 90, 90, 30},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_ice_ray, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(304), 20, 8,
     "ice ray", "The chill leaves your body."},

    {
     "hellfire", {15, 90, 90, 90, 90, 90, 90, 15},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_hellfire, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(305), 25, 12,
     "flames", "!Hellfire"},

    {
     "ice storm", {38, 90, 90, 90, 90, 90, 90, 38},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_ice_storm, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(306), 33, 8,
     "ice ray", "The chill leaves your body."},

    {
     "vision", {36, 17, 32, 28, 90, 90, 90, 36},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_vision, TAR_IGNORE,
     POS_FIGHTING, NULL, SLOT(307), 40, 20,
     "", "!Vision!"},

    {
     "restoration", {90, 40, 90, 90, 90, 90, 90, 90},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_restoration, TAR_CHAR_DEFENSIVE,
     POS_FIGHTING, NULL, SLOT(308), 75, 0,
     "", "!Restore!"},

    {
     "regeneration", {90, 30, 90, 90, 90, 90, 90, 90},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_regeneration, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(309), 50, 0,
     "", "Your body slows down."},

    {
     "test area", {14, 90, 90, 90, 90, 90, 90, 90},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_test_area, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT(310), 25, 12,
     "breath", "!TEST"},

    {
     "web", {13, 93, 93, 93, 93, 93, 8, 13},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_web, TAR_CHAR_DEFENSIVE,
     POS_STANDING, NULL, SLOT(511), 20, 0,
     "", "The webs disolve."},

    {
     "axe", {93, 93, 93, 1, 1, 1, 93, 93},
     {0, 0, 0, 4, 4, 4, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_axe, SLOT(0), 0, 0,
     "", "!Axe!"},

    {
     "dagger", {1, 1, 1, 1, 1, 1, 1, 1},
     {2, 3, 2, 2, 2, 2, 1, 1}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_dagger, SLOT(0), 0, 0,
     "", "!Dagger!"},

    {
     "flail", {93, 1, 93, 1, 1, 1, 93, 93},
     {0, 3, 0, 4, 4, 4, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_flail, SLOT(0), 0, 0,
     "", "!Flail!"},

    {
     "mace", {93, 1, 93, 1, 1, 1, 93, 93},
     {0, 2, 0, 3, 3, 4, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_mace, SLOT(0), 0, 0,
     "", "!Mace!"},

    {
     "polearm", {93, 93, 93, 1, 1, 1, 93, 93},
     {0, 0, 0, 4, 4, 4, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_polearm, SLOT(0), 0, 0,
     "", "!Polearm!"},

    {
     "shield block", {93, 1, 93, 1, 1, 1, 93, 93},
     {0, 4, 0, 2, 2, 5, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_shield_block, SLOT(0), 0, 0,
     "", "!Shield!"},

    {
     "spear", {93, 1, 93, 1, 1, 1, 93, 93},
     {0, 4, 0, 3, 3, 4, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_spear, SLOT(0), 0, 0,
     "", "!Spear!"},

    {
     "sword", {1, 1, 1, 1, 1, 1, 1, 1},
     {5, 6, 3, 2, 2, 2, 5, 6}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_sword, SLOT(0), 0, 0,
     "", "!sword!"},

    {
     "whip", {1, 1, 1, 1, 1, 1, 1, 1},
     {6, 5, 5, 4, 3, 3, 3, 3}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_whip, SLOT(0), 0, 0,
     "", "!Whip!"},

    {
     "backstab", {90, 90, 1, 90, 90, 90, 7, 90},
     {0, 0, 5, 0, 0, 0, 6, 0}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_backstab, SLOT(0), 0, 36,
     "backstab", "!Backstab!"},

    {
     "circle", {90, 90, 5, 90, 90, 90, 90, 90},
     {0, 0, 5, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_circle, SLOT(0), 0, 24,
     "circle", "!Circle!"},

    {
     "bash", {90, 90, 90, 3, 90, 90, 90, 90},
     {0, 0, 0, 4, 0, 0, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_bash, SLOT(0), 0, 24,
     "bash", "!Bash!"},

    {
     "berserk", {90, 90, 90, 15, 90, 90, 90, 90},
     {0, 0, 0, 5, 0, 0, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_berserk, SLOT(0), 0, 24,
     "", "You feel your pulse slow down."},

    {
     "dirt kicking", {10, 90, 3, 3, 90, 5, 3, 90},
     {5, 0, 4, 4, 0, 4, 4, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_dirt, SLOT(0), 0, 24,
     "kicked dirt", "You rub the dirt out of your eyes."},

    {
     "disarm", {90, 90, 26, 10, 11, 14, 90, 90},
     {0, 0, 6, 4, 5, 5, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_disarm, SLOT(0), 0, 24,
     "", "!Disarm!"},

    {
     "dodge", {20, 22, 1, 13, 17, 17, 8, 20},
     {8, 8, 4, 6, 5, 7, 4, 8}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_dodge, SLOT(0), 0, 0,
     "", "!Dodge!"},

    {
     "enhanced damage", {90, 90, 25, 1, 17, 17, 90, 90},
     {0, 0, 5, 3, 5, 5, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_enhanced_damage, SLOT(0), 0, 0,
     "", "!Enhanced Damage!"},

    {
     "hand to hand", {30, 20, 15, 4, 11, 11, 15, 30},
     {8, 5, 6, 4, 5, 5, 4, 8}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_hand_to_hand, SLOT(0), 0, 0,
     "", "!Hand to Hand!"},

    {
     "kick", {90, 90, 14, 8, 18, 13, 15, 90},
     {0, 0, 6, 3, 8, 5, 6, 0}, spell_null, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, &gsn_kick, SLOT(0), 0, 12,
     "kick", "!Kick!"},

    {
     "parry", {90, 18, 14, 1, 9, 11, 18, 90},
     {0, 8, 6, 3, 4, 5, 6, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_parry, SLOT(0), 0, 0,
     "", "!Parry!"},

    {
     "rescue", {90, 10, 90, 1, 1, 90, 90, 90},
     {0, 6, 0, 4, 3, 0, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_rescue, SLOT(0), 0, 12,
     "", "!Rescue!"},

    {
     "trip", {90, 90, 1, 11, 90, 15, 10, 90},
     {0, 0, 4, 6, 0, 4, 4, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_trip, SLOT(0), 0, 24,
     "trip", "!Trip!"},

    {
     "second attack", {30, 20, 12, 5, 12, 12, 15, 30},
     {9, 8, 5, 3, 5, 5, 5, 9}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_second_attack, SLOT(0), 0, 0,
     "", "!Second Attack!"},

    {
     "third attack", {90, 90, 60, 10, 24, 24, 90, 90},
     {0, 0, 10, 4, 6, 6, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_third_attack, SLOT(0), 0, 0,
     "", "!Third Attack!"},
    
    {
     "fourth attack", {90, 90, 60, 10, 24, 24, 90, 90},
     {0, 0, 10, 4, 6, 6, 0, 0}, spell_null, TAR_IGNORE,
     POS_FIGHTING, &gsn_fourth_attack, SLOT(0), 0, 0,
     "", "!Third Attack!"}, // JR

    {
     "fast healing", {18, 6, 16, 6, 6, 12, 17, 18},
     {8, 3, 7, 4, 4, 8, 6, 8}, spell_null, TAR_IGNORE,
     POS_SLEEPING, &gsn_fast_healing, SLOT(0), 0, 0,
     "", "!Fast Healing!"},

    {
     "haggle", {10, 18, 1, 14, 15, 4, 5, 10},
     {6, 8, 3, 6, 5, 5, 3, 6}, spell_null, TAR_IGNORE,
     POS_RESTING, &gsn_haggle, SLOT(0), 0, 0,
     "", "!Haggle!"},

    {
     "hide", {90, 90, 1, 90, 90, 90, 5, 90},
     {0, 0, 4, 0, 0, 0, 4, 0}, spell_null, TAR_IGNORE,
     POS_RESTING, &gsn_hide, SLOT(0), 0, 0,
     "", "!Hide!"},

    {
     "lore", {6, 90, 90, 90, 90, 90, 90, 6},
     {4, 0, 0, 0, 0, 0, 0, 4}, spell_null, TAR_IGNORE,
     POS_RESTING, &gsn_lore, SLOT(0), 0, 0,
     "", "!Lore!"},

    {
     "meditation", {3, 6, 90, 90, 90, 90, 15, 3},
     {4, 5, 0, 0, 0, 0, 7, 4}, spell_null, TAR_IGNORE,
     POS_SLEEPING, &gsn_meditation, SLOT(0), 0, 0,
     "", "Meditation"},

    {
     "peek", {90, 90, 1, 90, 90, 22, 90, 90},
     {0, 0, 3, 0, 0, 7, 0, 0}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_peek, SLOT(0), 0, 0,
     "", "!Peek!"},

    {
     "pick lock", {90, 90, 7, 90, 90, 90, 90, 90},
     {0, 0, 4, 0, 0, 0, 0, 0}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_pick_lock, SLOT(0), 0, 0,
     "", "!Pick!"},

    {
     "sneak", {90, 90, 4, 90, 90, 90, 6, 90},
     {0, 0, 4, 0, 0, 0, 3, 0}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_sneak, SLOT(0), 0, 0,
     "", "You no longer feel stealthy."},

    {
     "steal", {90, 90, 5, 90, 90, 15, 10, 90},
     {0, 0, 4, 0, 0, 5, 5, 0}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_steal, SLOT(0), 0, 24,
     "", "!Steal!"},

    {
     "scrolls", {1, 18, 20, 93, 25, 25, 15, 1},
     {0, 3, 5, 0, 6, 6, 6, 1}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_scrolls, SLOT(0), 0, 0,
     "", "!Scrolls!"},

    {
     "staves", {1, 93, 93, 93, 93, 93, 93, 1},
     {2, 0, 0, 0, 0, 0, 0, 2}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_staves, SLOT(0), 0, 0,
     "", "!Staves!"},

    {
     "wands", {1, 93, 18, 93, 93, 93, 10, 1},
     {2, 0, 5, 0, 0, 0, 5, 1}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_wands, SLOT(0), 0, 0,
     "", "!Wands!"},

    {
     "recall", {1, 1, 1, 1, 1, 1, 1, 1},
     {2, 2, 2, 2, 2, 2, 2, 2}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_recall, SLOT(0), 0, 0,
     "", "!Recall!"},

    {
     "brew", {22, 30, 93, 93, 93, 93, 93, 22},
     {2, 3, 0, 0, 0, 0, 0, 2}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_brew, SLOT(0), 0, 24,
     "", "!Brew!"},

    {
     "scribe", {25, 35, 93, 93, 93, 93, 93, 20},
     {2, 3, 0, 0, 0, 0, 0, 2}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_scribe, SLOT(0), 0, 24,
     "", "!Scribe!"},

/* Added by JR */
    {
     "vicious strike", {90, 90, 60, 10, 24, 24, 90, 90},
     {0, 0, 10, 4, 6, 6, 0, 0}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_vicious_strike, SLOT( 0 ), 0, 0,
     "", "!Vicious!"},
    
    {
     "nexus", {45, 93, 93, 93, 93, 93, 93, 45},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_nexus, TAR_IGNORE,
     POS_FIGHTING, NULL, SLOT( 84 ), 180, 0,
     "", "!Nexus!"},
    
    {
     "holyfire", {14, 90, 90, 90, 90, 90, 90, 12},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_holyfire, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT( 405 ), 25, 12,
     "flames", "!Holyfire"},
    
    {
     "life drain", {26, 28, 25, 93, 93, 36, 22, 26},
     {1, 1, 2, 2, 2, 2, 2, 1}, spell_life_drain, TAR_CHAR_OFFENSIVE,
     POS_FIGHTING, NULL, SLOT( 512 ), 75, 12,
     "life drain", "!Drain!"},

    {
     "energize", {30, 30, 93, 93, 50, 93, 93, 30}, {3,3,0,0,4,0,0,3},
     spell_energize, TAR_CHAR_DEFENSIVE, POS_STANDING,
     NULL, SLOT( 513 ), 25, 0,
     "", "!Energize!"},
    
    {
     "counter", {93, 93, 45, 18, 36, 18, 45, 93}, {0, 0, 8, 6, 6, 6, 8, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_counter, SLOT( 0 ), 0, 0,
     "", "!Counter!"},

    {
     "grip", {93, 93, 93, 12, 93, 12, 93, 93}, {0, 0, 0, 6, 0, 6, 0, 0},
     spell_null, TAR_IGNORE, POS_FIGHTING,
     &gsn_grip, SLOT( 0 ), 0, 18,
     "", "!Grip!"},

    {
     "blind fighting", {93, 93, 93, 10, 93, 10, 93, 93}, {0, 0, 0, 5, 0, 5, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_blind_fighting, SLOT( 0 ), 0, 36,
     "", "!Blind_Fighting!"},

    {
     "shield cleave", {93, 93, 1, 93, 12, 1, 93, 93}, {0, 0, 0, 3, 3, 3, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_shield_cleave, SLOT( 0 ), 0, 5,
     "", "!Shield_Cleave!"},

    {
     "weapon cleave", {93, 93, 3, 93, 14, 3, 93, 93}, {0, 0, 0, 7, 7, 7, 0, 0},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_weapon_cleave, SLOT( 0 ), 0, 25,
     "", "!Weapon_Cleave!"},

    {
     "blackjack", {60, 93, 3, 3, 93, 5, 5, 93},
     {6, 0, 4, 4, 0, 4, 4, 0}, spell_null, TAR_IGNORE,
     POS_STANDING, &gsn_blackjack, SLOT( 0 ), 0, 24,
     "blackjack", "Man what a headache, good thing it's over."},

#ifdef TRACK_IS_SKILL
    {
     "track", {80, 80, 25, 35, 35, 35, 25, 80}, {8, 5, 3, 2, 2, 2, 5, 8},
     spell_null, TAR_IGNORE, POS_STANDING,
     &gsn_track, SLOT( 0 ), 0, 0,
     "", "!track!"},
#endif
    
};

const struct group_type group_table[MAX_GROUP] = {
    {
     "rom basics", {0, 0, 0, 0, 0, 0, 0, 0},
     { "scrolls", "staves", "wands", "recall", "track" } // JRwho
    },

    {
     "mage basics", {0, -1, -1, -1, -1, -1, -1, -1},
     { "dagger", "meditation" } 
    },

    {
     "cleric basics", {-1, 0, -1, -1, -1, -1, -1, -1},
     { "mace" } 
    },

    {
     "thief basics", {-1, -1, 0, -1, -1, -1, -1, -1},
     { "dagger", "steal" } 
    },

    {
     "warrior basics", {-1, -1, -1, 0, -1, -1, -1, -1},
     { "sword", "second attack" } 
    },

    {
     "paladin basics", {-1, -1, -1, -1, 0, -1, -1, -1},
     { "sword" } 
    },

    {
     "avenger basics", {-1, -1, -1, -1, -1, 0, -1, -1},
     { "sword", "dagger" } 
    },

    {
     "nightblade basics", {-1, -1, -1, -1, -1, -1, 0, -1},
     { "dagger", "sneak" } 
    },

    {
     "mystic basics", {-1, -1, -1, -1, -1, -1, -1, 0},
     { "dagger", "meditation" } 
    },

    {
     "mage default", {40, -1, -1, -1, -1, -1, -1, -1},
     { "lore", "beguiling", "combat", "detection", "enhancement", "illusion",
      "maladictions", "protective", "transportation", "weather", "wizard only" } 
    },

    {
     "cleric default", {-1, 40, -1, -1, -1, -1, -1, -1},
     { "flail", "attack", "creation", "curative",  "benedictions",  "detection",
      "healing", "maladictions", "protective", "shield block",  "harmful", "weather" } 
    },

    {
     "thief default", {-1, -1, 40, -1, -1, -1, -1, -1},
     { "circle", "sword", "backstab", "disarm", "dodge", "second attack",
       "trip", "hide", "peek", "pick lock", "sneak" } 
    },

    {
     "warrior default", {-1, -1, -1, 40, -1, -1, -1, -1},
     { "weaponsmaster", "shield block", "bash", "disarm", "enhanced damage",
      "parry", "rescue", "third attack", "berserk" } 
    },

    {
     "paladin default", {-1, -1, -1, -1, 40, -1, -1, -1},
     { "weaponsmaster", "shield block", "protective", "disarm", "curative",  "parry", "rescue", "healing" } 
    },

    {
     "avenger default", {-1, -1, -1, -1, -1, 40, -1, -1},
     { "weaponsmaster", "maladictions", "disarm", "attack",
       "parry", "harmful", "third attack" } 
    },

    {
     "nightblade default", {-1, -1, -1, -1, -1, -1, 40, -1},
     {"illusion", "sword", "backstab", "transportation", "dodge",  "trip", "hide", "beguiling", "combat", "sneak" } 
    },

    {
     "mystic default", {-1, -1, -1, -1, -1, -1, -1, 40},
     {"lore", "enchantment", "combat", "detection", "enhancement", "wizard only",
      "maladictions", "protective", "transportation", "weather", "draconian" } 
    },

    {
     "weaponsmaster", {50, 40, 40, 20, 30, 30, 40, 50},
     { "axe", "dagger", "flail", "mace", "polearm", "spear",
      "sword","whip" } 
    },

    {
     "attack", {4, 5, -1, -1, -1, 6, -1, 4},
     { "demonfire", "dispel evil", "earthquake", "flamestrike" } 
    },

    {
     "beguiling", {3, -1, 6, -1, -1, -1, 4, 3},
     { "charm person", "sleep" } 
    },

    {
     "benedictions", {-1, 3, -1, -1, 4, -1, -1, -1},
     { "bless", "calm", "frenzy", "holy word", "remove curse"} 
    },

    {
     "combat", {6, -1, -1, -1, -1, -1, 5, 6},
       { "acid blast", "burning hands", "chain lightning", "chill touch",
        "colour spray", "fireball", "lightning bolt", "magic missile", "shocking grasp"} 
    },

    {
     "creation", {3, 4, -1, -1, 5, -1, -1, 3},
     { "continual light", "create food", "create spring", "create water" } 
    },

    {
     "curative", {-1, 4, -1, -1, 2, -1, -1, -1},
     { "cure blindness", "cure disease", "cure poison" } 
    },

    {
     "detection", {4, 3, -1, -1, 7, -1, 2, 4},
     { "detect evil", "detect hidden", "detect invis", "detect magic",
      "detect poison", "identify", "know alignment", "locate object" } 
    },

    {
     "draconian", {7, -1, -1, -1, -1, -1, -1, 7},
     { "acid breath", "fire breath", "frost breath", "gas breath",  "lightning breath"  } 
    },

    {
     "enchantment", {6, 8, -1, -1, -1, -1, -1, 6},
     { "enchant armor", "enchant weapon" } 
    },

    {
     "enhancement", {4, -1, -1, -1, -1, 8, 6, 4},
     { "giant strength", "haste", "infravision", "refresh" } 
    },

    {
     "harmful", {-1, 3, -1, -1, -1, 3, -1, -1},
     { "cause critical", "cause light", "cause serious", "harm" } 
    },

    {
     "healing", {-1, 4, -1, -1, 4, -1, -1, -1},
     { "cure critical", "cure light", "cure serious", "heal",  "mass healing", "refresh", "restoration", "regeneration"} 
    },

    {
     "illusion", {4, -1, 4, -1, -1, -1, 4, 4},
     { "invis", "mass invis", "ventriloquate" } 
    },

    {
     "maladictions", {5, 4, -1, -1, -1, 5, 6, 5},
     { "blindness", "change sex", "curse", "energy drain",
       "plague", "poison", "weaken", "web", "life drain" } 
    },

    {
     "protective", {4, 3, -1, -1, 3, -1, -1, 4},
     { "armor", "cancellation", "dispel magic", "protection evil",
       "sanctuary", "shield", "stone skin", "bark skin" } 
    },

    {
     "transportation", {4, -1, -1, -1, -1, -1, 3, 4},
     { "fly", "gate", "nexus", "pass door", "summon",
       "teleport", "word of recall"} 
    },

    {
     "weather", {4, 4, -1, -1, -1, -1, -1, 4},
     { "call lightning", "control weather", "faerie fire", "faerie fog",  "lightning bolt" } 
    },

    {
     "wizard only", {6, -1, -1, -1, -1, -1, -1, 6},
     { "firewind", "meteor swarm", "disintegrate", "ice ray", "hellfire", "vision", "ice storm" }
    },
    
};
