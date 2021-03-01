#MOBPROGS
#1
Poison Bite~
Poisons the person the mob is fighting with. Chance based on mob's level.~
fight_prog 100~
if(getrand(50) <= level($i))
  mpechoat $i You bite $n!
  mpechoaround $n $i bites $n!
  mpechoat $n $i bites you!
  mpsilentcast 'poison' $n
endif
break
~
#2
Acid Breath~
Picks a target from the list of people fighting it.  25% chance per target of being picked then casts 'Acid Breath'.~
fightgroup_prog 25~
cast 'acid breath' $n
break
~
#3
Fire Breath~
Picks a target from the list of people fighting it.  25% chance per target of being picked then casts 'Fire Breath'.~
fightgroup_prog 25~
cast 'fire breath' $n
break
~
#4
Frost Breath~
Picks a target from the list of people fighting it.  25% chance per target of being picked then casts 'Frost Breath'.~
fightgroup_prog 25~
cast 'frost breath' $n
break
~
#5
Gas Breath~
Picks a target from the list of people fighting it.  25% chance per target of being picked then casts 'Gas Breath'.~
fightgroup_prog 25~
cast 'gas breath' $n
break
~
#6
Lightning Breath~
Picks a target from the list of people fighting it.  25% chance per target of being picked then casts 'Lightning Breath'.~
fightgroup_prog 25~
cast 'lightning breath' $n
break
~
#7
Random Breath~
Picks a target from the list of people fighting it.  25% chance per target of being picked then casts a random Breath spell.~
fightgroup_prog 25~
if(sgetrand(6) == 1)
  cast 'acid breath' $n
  break
endif
if(sgetrand(6) == 2)
  cast 'fire breath' $n
  break
endif
if(sgetrand(6) == 3)
  cast 'frost breath' $n
  break
endif
if(sgetrand(6) == 4)
  cast 'gas breath' $n
  break
endif
if(sgetrand(6) == 5)
  cast 'lightning breath' $n
  break
endif
break
~
#8
High Explosive Judge~
Picks a target from the list of people fighting it.  25% chance per target of being picked then casts 'High Explosive'.~
fightgroup_prog 25~
cast 'high explosive' $n
break
~
#9
Adepts~
Chance of casting a beneficial spell on a random player in the room.~
rand_prog 25~
if(sgetrand(6) == 1)
  cast 'armor' $r
  break
endif
if(sgetrand(6) == 2)
  cast 'bless' $r
  break
endif
if(sgetrand(6) == 3)
  cast 'refresh' $r
  break
endif
if(sgetrand(6) == 4)
  cast 'cure blindness' $r
  break
endif
if(sgetrand(6) == 5)
  cast 'cure poison' $r
  break
endif
if(sgetrand(6) == 6)
  cast 'heal' $r
  break
endif
break
~
#10
Offensive Cleric~
Picks a target from the list of people fighting it.  50% chance per target of being picked then casts a spell.~
fightgroup_prog 50~
if(sgetrand(30) == 1)
  cast 'blindness' $n
  break
endif
if(level($i) >= 3 && sgetrand(30) == 2)
  cast 'cause serious' $n
  break
endif
if(level($i) >= 7 && sgetrand(30) == 3)
  cast 'earthquake' $n
  break
endif
if(level($i) >= 9 && sgetrand(30) == 4)
  cast 'cause critical' $n
  break
endif
if(level($i) >= 10 && sgetrand(30) == 5)
  cast 'dispel evil' $n
  break
endif
if(level($i) >= 12 && sgetrand(30) == 6)
  cast 'curse' $n
  break
endif
if(level($i) >= 12 && sgetrand(30) == 7)
  cast 'change sex' $n
  break
endif
if(level($i) >= 13 && sgetrand(30) == 8)
  cast 'flamestrike' $n
  break
endif
if(level($i) >= 15 && sgetrand(30) == 9)
  cast 'harm' $n
  break
endif
if(level($i) >= 15 && sgetrand(30) == 10)
  cast 'plague' $n
  break
endif
if(level($i) >= 16 && sgetrand(30) == 11)
  cast 'dispel magic' $n
  break
endif
break
~
#11
Mayor Wander~
Mayor wanders around town closing and opening town gates depending on the time.~
rand_prog 100~
if(isfight($i))
  break
endif
if(hour() == 6)
  mpfollowpath w3t3003u33000v111x0x111Oy333333Oy22v222112212111t1s. restart
  break
endif
if(hour() == 20)
  mpfollowpath w3t3003u33000v111x0x111Cy333333Cy22v222112212111t1s. restart
  break
endif
if(hour() >= 6 && hour() < 19)
  mpfollowpath w3t3003u33000v111x0x111Oy333333Oy22v222112212111t1s. continue
  break
endif
mpfollowpath w3t3003u33000v111x0x111Cy333333Cy22v222112212111t1s. continue
break
~
#25
Trump Wander~
Trump wanders around town, saying offensive things.~
rand_prog 100~
if(isfight($i))
  break
endif
if(hour() == 6)
  mpfollowpath W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S. restart
  break
endif
if(hour() == 20)
  mpfollowpath W3a3003b33000c111d0d111CE333333CE22c222112212111a1S. restart
  break
endif
if(hour() >= 6 && hour() < 19)
  mpfollowpath W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S. continue
  break
endif
mpfollowpath W3a3003b33000c111d0d111CE333333CE22c222112212111a1S. continue
break
~
#13
Offensive Mage~
Picks a target from the list of people fighting it.  50% chance per target of being picked then casts a spell.~
fightgroup_prog 50~
if(sgetrand(29) == 1)
  cast 'blindness' $n
  break
endif
if(level($i) >= 3 && sgetrand(29) == 2)
  cast 'chill touch' $n
  break
endif
if(level($i) >= 7 && sgetrand(29) == 3)
  cast 'weaken' $n
  break
endif
if(level($i) >= 8 && sgetrand(29) == 4)
  cast 'teleport' $n
  break
endif
if(level($i) >= 11 && sgetrand(29) == 5)
  cast 'colour spray' $n
  break
endif
if(level($i) >= 12 && sgetrand(29) == 6)
  cast 'change sex' $n
  break
endif
if(level($i) >= 13 && sgetrand(29) == 7)
  cast 'energy drain' $n
  break
endif
if(level($i) >= 15 && sgetrand(29) == 8)
  cast 'fireball' $n
  break
endif
if(level($i) >= 20 && sgetrand(29) == 9)
  cast 'plague' $n
  break
endif
if(level($i) >= 20 && sgetrand(29) == 10)
  cast 'acid blast' $n
  break
endif
break
~
#14
Undead Mage~
Picks a target from the list of people fighting it.  50% chance per target of being picked then casts a spell.~
fightgroup_prog 50~
if(sgetrand(29) == 1)
  cast 'curse' $n
  break
endif
if(level($i) >= 3 && sgetrand(29) == 2)
  cast 'weaken' $n
  break
endif
if(level($i) >= 6 && sgetrand(29) == 3)
  cast 'chill touch' $n
  break
endif
if(level($i) >= 9 && sgetrand(29) == 4)
  cast 'blindness' $n
  break
endif
if(level($i) >= 12 && sgetrand(29) == 5)
  cast 'poison' $n
  break
endif
if(level($i) >= 15 && sgetrand(29) == 6)
  cast 'energy drain' $n
  break
endif
if(level($i) >= 18 && sgetrand(29) == 7)
  cast 'harm' $n
  break
endif
if(level($i) >= 21 && sgetrand(29) == 8)
  cast 'teleport' $n
  break
endif
if(level($i) >= 20 && sgetrand(29) == 9)
  cast 'plague' $n
  break
endif
if(level($i) >= 18 && sgetrand(29) == 10)
  cast 'harm' $n
  break
endif
break
~
#15
Executioner~
When someone walks into a room it checks to see if they've commited a particular crime, if so it summons guards and attacks.~
all_greet_prog 100~
if(isfight($i))
  break
endif
if(crimethief($n))
  yell $n is a thief! PROTECT THE INNOCENT! MORE BLOOOOD!!!
  mpmload 3060
  mpmload 3060
  mpkill $n
  mpforce cityguard kill $n
  mpforce 2.cityguard kill $n
  break
endif
break
~
#16
Fido~
Dog that walks around eating corpses, yummy.~
rand_prog 100~
mpeatcorpse
break
~
#17
Anti-Thief Guard~
Attacks thieves.~
greet_prog 100~
if(isfight($i))
  break
endif
if(crimethief($n))
  yell $n is a thief! PROTECT THE INNOCENT! BANZAI!!!
  mpkill $n
  break
endif
break
~
#18
Protector of Good~
If people are fighting in the room it will attack the person with the lowest alignment (if alignment is under 300).~
rand_prog 100~
if(isfight($i))
  break
endif
if(fightinroom() && alignment($x) < 300)
  :screams 'PROTECT THE INNOCENT!! BANZAI!!'
  mpkill $x
  break
endif
break
~
#19
Janitor~
Picks up trash.~
rand_prog 100~
mpclean
break
~
#21
Thief~
Steals from PCs.~
rand_prog 30~
steal gold $r
break
~
#22
Puff Social~
Puff does and says a few things randomly.~
rand_prog 100~
if(sgetrand(100) <= 20)
  break
elseif(sgetrand(100) <= 30)
  say Tongue-tied and twisted, just an earthbound misfit, ...
  break
elseif (sgetrand(100) <= 40)
  say The colors, the colors!
  break
elseif (sgetrand(100) <= 55)
  say Did you know that I'm written in MudScript?
  break
elseif (sgetrand(100) <= 75)
  mprandomsocial
  break
elseif (sgetrand(100) <= 85)
  mprandomsocial $c
  break
elseif (sgetrand(100) <= 97)
  mpecho For a moment, $i flickers and phases.
  mpechoat $i For a moment, you flicker and phase.
  break
else
  if (!isfight($i))
    mpecho For a moment, $i seems lucid...
    mpecho    ...but then $j returns to $k contemplations once again.
    mpechoat $i For a moment, the world's mathematical beauty is lost to you!
    mpechoat $i    ...but joy! yet another novel phenomenon seizes your attention.
    break
  else
    cast 'teleport'
  endif
  break
endif
break
~
#23
Puff Fight~
Puff teleports a random target she's fighting against.  25% each player.~
fightgroup_prog 25~
cast 'teleport' $n
break
~
#101
Stub Sea Captain~
A stub for Mudweiser spec_sea_captain~
rand_prog 100~
if (!isfight($i))
    if (sgetrand(20) <= 10)
      break
    elseif (sgetrand(20 <= 18))
      say Die, landlubbers!
      break
    else
      say Arrr.....
      break
    endif
endif
break
~
#28
Read~
Read the text of The Great Gatsby~
rand_prog 100~
if (!isfight($i))
  mpreadgatsby
endif
break
~
#29
Test~
testing~
rand_prog 100~
say testing...ok
break
~
#102
Stub French Taunter~
A stub for Mudweiser spec_french_taunter~
rand_prog 100~
if (sgetrand(10) == 1)
  say Go and boil your bottom, you son of a silly person!
  break
elseif (sgetrand(10) == 2)
  say I blow my nose on you, you silly person!
  break
elseif (sgetrand(10) == 3)
  emote puts his hands to his ears and blows a raspberry.
  break
elseif (sgetrand(10) == 4)
  say I don't wanna talk to you anymore, you empty-headed animal-food-trough wiper!
  break
elseif (sgetrand(10) == 5)
  say I fart in your general direction!
  break
elseif (sgetrand(10) == 6)
  say Your mother was a hamster and your father smelt of elderberries!
  break
elseif (sgetrand(10) == 7)
  say Now go away, or I shall taunt you a second time!
  break
elseif (sgetrand(10) == 8)
  say I burst my pimples at you, you tiny-brained wiper of other people's bottoms!
  break
elseif (sgetrand(10) == 9)
  say I unclog my nose towards you, you son of a window dresser!
  break
elseif (sgetrand(10) == 10)
  say I wave my private parts at your aunties, you brightly-colored, mealy-templed, cranberry-smelling, electric donkey-bottom biters!
  break
endif
break
~
#103
Stub Nee~
A stub for Mudweiser spec_nee~
rand_prog 100~
say Nee!
break
~
#104
Stub Tim~
A stub for Mudweiser spec_tim~
fight_prog 33~
mpsilentcast 'fireball' $n
break
~
#105
Stub Reaper~
A stub for Mudweiser spec_reaper~
rand_prog 100~
if(sgetrand(4) == 3)
  say Your fate lies in my hands, you fool!
elseif(sgetrand(4) == 4)
  say `RMuahahahahaha!!
endif
break
~
#200
Mudweiser MOBProgs/3011.prg-1~
Mudweiser MOBProgs/3011.prg-1~
rand_prog 1~
say Man, I'm bored.
break
~
#201
Mudweiser MOBProgs/3011.prg-2~
Mudweiser MOBProgs/3011.prg-2~
speech_prog qwqwqwasxdazsfkuahsdakjshdljkshfa~
if ispc($n)
 shout everyone SUCKS!
endif
break
~
#202
Mudweiser MOBProgs/109.prg-1~
Mudweiser MOBProgs/109.prg-1~
speech_prog p die, bob!~
say $n, that was not very nice.
break
~
#203
Mudweiser MOBProgs/1503.prg-1~
Mudweiser MOBProgs/1503.prg-1~
hitprcnt_prog 95~
break
~
#204
Mudweiser MOBProgs/1503.prg-2~
Mudweiser MOBProgs/1503.prg-2~
death_prog 100~
mpecho $i yells "Now we see the violence inherent in the system!"
mpasound $i yells "Now we see the violence inherent in the system!"
break
~
#205
Mudweiser MOBProgs/1503.prg-3~
Mudweiser MOBProgs/1503.prg-3~
bribe_prog 1~
say I don't want your filthy money.
break
~
#206
Mudweiser MOBProgs/1503.prg-4~
Mudweiser MOBProgs/1503.prg-4~
speech_prog p old woman~
say Man!
break
~
#207
Mudweiser MOBProgs/1503.prg-5~
Mudweiser MOBProgs/1503.prg-5~
speech_prog p old woman!~
say Man!
break
~
#208
Mudweiser MOBProgs/1503.prg-6~
Mudweiser MOBProgs/1503.prg-6~
speech_prog p old man~
say I'm 37, I'm not old.
break
~
#209
Mudweiser MOBProgs/1503.prg-7~
Mudweiser MOBProgs/1503.prg-7~
speech_prog p old man!~
say I'm 37, I'm not old.
break
~
#210
Mudweiser MOBProgs/1503.prg-8~
Mudweiser MOBProgs/1503.prg-8~
speech_prog p am king~
say King of the who?
break
~
#211
Mudweiser MOBProgs/1503.prg-9~
Mudweiser MOBProgs/1503.prg-9~
speech_prog briton~
say Who are the Britons?
break
~
#212
Mudweiser MOBProgs/1503.prg-10~
Mudweiser MOBProgs/1503.prg-10~
speech_prog britons~
say Who are the Britons?
break
~
#213
Mudweiser MOBProgs/1503.prg-11~
Mudweiser MOBProgs/1503.prg-11~
speech_prog p am your king~
say I didn't vote for you.
break
~
#214
Mudweiser MOBProgs/1503.prg-12~
Mudweiser MOBProgs/1503.prg-12~
speech_prog p don't vote~
say Well, I could become king then.
break
~
#215
Mudweiser MOBProgs/1503.prg-13~
Mudweiser MOBProgs/1503.prg-13~
speech_prog lord~
say We don't have a lord. We're a anarcho-syndicalist commune.
say We take it in turns to act as a sort of executive officer of the 
say week.
break
~
#216
Mudweiser MOBProgs/1503.prg-14~
Mudweiser MOBProgs/1503.prg-14~
speech_prog p order~
say Order, eh? Who do you think you are?
break
~
#217
Mudweiser MOBProgs/1503.prg-15~
Mudweiser MOBProgs/1503.prg-15~
speech_prog p shut up!~
say Supreme power derives from a mandate from the masses, not from some
say farcical aquatic ceremony.
break
~
#218
Mudweiser MOBProgs/1503.prg-16~
Mudweiser MOBProgs/1503.prg-16~
speech_prog p shut up~
say Supreme power derives from a mandate from the masses, not from some
say farcical aquatic ceremony.
break
~
#219
Mudweiser MOBProgs/1503.prg-17~
Mudweiser MOBProgs/1503.prg-17~
speech_prog p lady of the lake~
say Strange women lying in ponds distributing swords is no basis for a 
say system of government.
break
~
#220
Mudweiser MOBProgs/1503.prg-18~
Mudweiser MOBProgs/1503.prg-18~
speech_prog p lady of the lake,~
say Strange women lying in ponds distributing swords is no basis for a 
say system of government.
break
~
#221
Mudweiser MOBProgs/1503.prg-19~
Mudweiser MOBProgs/1503.prg-19~
speech_prog p be quiet!~
say Well, you can't expect to wield supreme executive power just because
say some watery tart threw a sword at you!
break
~
#222
Mudweiser MOBProgs/1503.prg-20~
Mudweiser MOBProgs/1503.prg-20~
speech_prog p bloody peasant~
say Oh, what a give away.
say Did you hear that, did you hear that, eh?
say That's what I'm on about -- did you see him?
say Did you see him repressing me? You saw it didn't you?
break
~
#223
Mudweiser MOBProgs/1503.prg-21~
Mudweiser MOBProgs/1503.prg-21~
speech_prog p bloody peasant!~
say Oh, what a give away.
say Did you hear that, did you hear that, eh?
say That's what I'm on about -- did you see him?
say Did you see him repressing me? You saw it didn't you?
break
~
#224
Mudweiser MOBProgs/1523.prg-1~
Mudweiser MOBProgs/1523.prg-1~
death_prog 10~
mpmload 1530
mpforce eeee mpoload 1522
mpforce eeeeckyeckyeckyeckypikangzoomboing say We are no longer the knights who say Nee!
mpforce eeeeckyeckyeckyeckypikangzoomboing say We are now the knights who say Eeee-ecky-ecky-ecky-pitang-zooooom-boing!
break
~
#225
Mudweiser MOBProgs/1532.prg-1~
Mudweiser MOBProgs/1532.prg-1~
speech_prog blah~
say Check.
 
if inroom($i) == 1505
say check one
if ispc($r)
say check two
else
say check npc two
endif
say $r sucks.
endif
say $i is me
say $n is $$n
say $r is $$r
break
~
#226
Mudweiser MOBProgs/4719.prg-1~
Mudweiser MOBProgs/4719.prg-1~
act_prog p bows before you~
mpoload 4193
give hammer $n
break
~
#227
Mudweiser MOBProgs/4721.prg-1~
Mudweiser MOBProgs/4721.prg-1~
fight_prog 15~
yell HELP! $n is attacking me!
mpmload 4180
break
~
#228
Mudweiser MOBProgs/4739.prg-1~
Mudweiser MOBProgs/4739.prg-1~
rand_prog 85~
emote flicks you the bird.
say Get the hell out of my town.
say Got anyone you need erased?
emote spits.
break
~
#229
Mudweiser MOBProgs/4739.prg-2~
Mudweiser MOBProgs/4739.prg-2~
look_prog 100~
if isnpc($n)
      slap $n
         say Who  the hell are you looking at me?
else
    if level ($n) <= 18
           tell $n I warn you.. Do NOT look at me.. 
else
if level($n) > 18
     say Big mistake..
     say Who gave you permission to look at me?
     say Now, you die..
     kill $n
     break
endif
endif
break
~
#230
Mudweiser MOBProgs/4744.prg-1~
Mudweiser MOBProgs/4744.prg-1~
fight_prog 15~
yell HELP! $n is attacking me!
mpmload 4180
break
~
#231
Mudweiser MOBProgs/4745.prg-1~
Mudweiser MOBProgs/4745.prg-1~
fight_prog 15~
yell HELP! $n is attacking me!
mpmload 4180
break
~
#232
Mudweiser MOBProgs/4746.prg-1~
Mudweiser MOBProgs/4746.prg-1~
fight_prog 15~
yell HELP! $n is attacking me!
mpmload 4180
break
~
#2021
Esmerelda's daughter~
If you ask Emerelda's daughter about Esmerelda, she will tell you~
speech_prog esmerelda~
say Esmerelda went to the graveyard to pay respects to the dead.
break
~
#2022
Sheriff talk~
The Sheriff periodically says things~
rand_prog 15~
if(getrand(4)==1)
  say Everybody stay safe out there
  break
endif
if(getrand(4)==2)
  say Ham Harbor is a safe place, and I'd like to keep it that way
  break
endif
if(getrand(4)==3)
  say Don't try anything funny while you're in my town
  break
endif
if(getrand(4)==4)
  threaten
  break
endif
break
~
#2023
(no name)~
(no description)~
rand_prog 100~
break
~
#2024
Grandpa Smith~
If you give grandpa a musket, he will give you a puzzle~
give_prog old rusty musket~
say Thank you, this will go great in my collection.
say Here, take this!
mpoload 10007
give puzzle $n
mpjunk musket
break
~
#2025
Grandma Jones~
If you give grandma a puzzle, she will give you a secret symbol~
give_prog ten thousand piece puzzle~
say Thank you, I love puzzles.
say Here, if give this to Esmerelda, she will have something for you!
mpoload 10039
give secret $n
mpjunk puzzle
break
~
#2026
Esmerelda~
If you give Esmerelda a secret symbol, she will give you the green key~
give_prog secret symbol~
say Thank you, this is just what I needed.
say Here, take this
mpoload 10017
give green $n
mpjunk symbol
break
~
#2027
test cycle~
test cycle to walk~
rand_prog 100~
mpcycle e;n;say hi;s;s;s;w;w;n;say hello;n;sl;st;
break
~
#2028
Ham Harbor loop~
Walk around the area, doing things.~
rand_prog 100~
mpcycle jog;mpgoto 10147;n;n;w;w;s;s;w:say It's nice to get out of the city.;n;n;w;w;w;n;n;w;emote points up at the chimney.;e;s;s;w;s;s;s;say Don't go that way:emote points south.;n;n;n;e;e;e;e;n;n;w;wave mava;e;n;n;w;wave amelia;e;n;n;n;n;e;e;say I've never been to Midgaard;w;w;w;w;w;w;w;n;e;say I heard Jed keeps lots something important in his cellar.;w;s;s;w;w;emote points at the boathouse.:say I wish I could go in, but the door's always locked!;e;e;e;s;e;say Whoops, looks like a dead-end!;w;n;n;e;e;n;say They used to sell paper here.;s;e;e;s;s;e;e;e;s;s;say It's a lovely day for a picnic:say I think I'll go to my favorite spot;e;n;e;e;n;s;n;e;e;say Ah, my favorite spot.;emote sits down to eat his lunch.;;;;;;say All finished!;w;w;s;s;s;e;e;emote points to the east.:say It's easy to get lost in the wetlands;w;s;s;s;w;wave gran;e;n;w;wave gran;e;n;n;w;w;w;s;s;e;wave ezra;w;s;w;w;s;s;e;n;emote mails a letter.;s;w;w;w;n;e;wave matt;w;n;w;s;s;s;e;e;e;emote pays his respects;w;w;w;w;emote says a prayer and lights a candle.;e;n;n;n;e;e;e;say I need a rest!:rest;;;;;;;stand;say OK, ready to go!
break
~
#2029
(no name)~
(no description)~
rand_prog 100~
mpcycle cit;goto 10145:n;;;e;;;e;;e;;;e;;;w;;;w;;;w;;say Good day to you!;;w;w;;w;;s;say Out of my way!;s;;e;;s;;s;;e;;e;;n;;n;;n;;n;;
break
~
#2030
(no name)~
(no description)~
all_greet_prog 100~
say Welcome to Champion's Market, $N!!
say Let me know if you want to buy anything.
break
~
#2031
Innocent death~
When MOB dies, it complains~
death_prog 100~
if(sgetrand(3)==1)
yell What did I ever do to you?!?
break
elseif(sgetrand(3)==2)
yell $X is mean!
break
else
yell Oh, the humanity!
endif
break
~
#2032
Reset jogger~
When jogger dies, reset path info~
death_prog 100~
mpcycle jog;reset
break
~
#2033
(no name)~
(no description)~
death_prog 100~
mpecho `YThe hydra sprouts two new heads!
mpmload 10301
mpmload 10301
mpforce hydra kill $X
mpforce 2.hydra kill $X
break
~
#0



#OBJPROGS
#1234
Blah~
Blah~
wear_prog 100~
mpechoat $n You shimmer and begin to fade.
mpechoaround $n $n begins to fade.
cast 'invis' $n
break
~
#600
Mudweiser OBJProgs/4151.prg-1~
Mudweiser OBJProgs/4151.prg-1~
wear_prog 100~
mpechoat $n You shimmer and begin to fade.
mpechoaround $n $n begins to fade.
cast 'invis' $n
break
~
#601
Mudweiser OBJProgs/4151.prg-2~
Mudweiser OBJProgs/4151.prg-2~
remove_prog 100~
cast 'continual light' $n
break
~
#602
Mudweiser OBJProgs/4152.prg-1~
Mudweiser OBJProgs/4152.prg-1~
remove_prog 100~
cast 'curse' $n
break
~
#603
Mudweiser OBJProgs/4152.prg-2~
Mudweiser OBJProgs/4152.prg-2~
fight_prog 100~
break
~
#604
Mudweiser OBJProgs/4152.prg-3~
Mudweiser OBJProgs/4152.prg-3~
remove_prog 100~
mppurge test
break
~
#605
Mudweiser OBJProgs/4194.prg-1~
Mudweiser OBJProgs/4194.prg-1~
wear_prog 100~
mpecho The accursed pendant seems to radiate darkness and evil.
cast 'curse' $n
mpechoaround $n $n seems to be cursed.
mpechoat $n You have been cursed by the pendant.
break
~
#606
Mudweiser OBJProgs/4194.prg-2~
Mudweiser OBJProgs/4194.prg-2~
speech_prog p pendant to blade~
mpecho An accursed pendant shudders violently and explodes.
mpoload 4198 21
drop accbl
mpforce $n sac accpnd
mpforce $n get accbl
break
~
#607
Mudweiser OBJProgs/4194.prg-3~
Mudweiser OBJProgs/4194.prg-3~
remove_prog 98~
mpecho The accursed pendant's curse loses its effectiveness.
cast 'remove curse' $n
cast 'cause serious' $n
break
~
#608
Mudweiser OBJProgs/4199.prg-1~
Mudweiser OBJProgs/4199.prg-1~
wear_prog 100~
mpecho The TEST weapon hums with power as it is wielded.
break
~
#609
Mudweiser OBJProgs/4199.prg-2~
Mudweiser OBJProgs/4199.prg-2~
fight_prog 100~
break
~
#610
Mudweiser OBJProgs/4749.prg-1~
Mudweiser OBJProgs/4749.prg-1~
use_prog 100~
mpecho The `RFlame `yBlade `wflickers momentarily and erupts into flame.
break
~
#611
Mudweiser OBJProgs/4753.prg-1~
Mudweiser OBJProgs/4753.prg-1~
wear_prog 100~
mpechoat $n You recieve nature's blessing.
mpechoaround $n $n is granted nature's blessing.
cast 'bless' $n
cast 'cure light' $n
cast 'remove curse' $n
break
~
#612
Mudweiser OBJProgs/4753.prg-2~
Mudweiser OBJProgs/4753.prg-2~
remove_prog 100~
mpecho The wooden ring loses its powers and disintegrates.
mpforce $n junk gnarled wooden ring
break
~
#613
Mudweiser OBJProgs/4783.prg-1~
Mudweiser OBJProgs/4783.prg-1~
wear_prog 100~
mpecho A `yTimion `GFullplate `yglows `Cwith a radiant `Ressence.`w
break
~
#614
Mudweiser OBJProgs/4783.prg-2~
Mudweiser OBJProgs/4783.prg-2~
remove_prog 100~
mpecho A `yTimion `GFullplate `Cshimmers briefly and `Kfades.`w
break
~
#615
Mudweiser OBJProgs/4787.prg-1~
Mudweiser OBJProgs/4787.prg-1~
wear_prog 100~
if class($n) != 3
break
endif
if inroom($n) == 4766
break
endif
if isfight($n)
mpechoat $n You may not invoke this item while fighting.
break
endif
mpechoat $n You are transported to the warrior's guild.
mpechoaround $n $n invokes the symbol of $s guild and vanishes in a puff of smoke.
mptran $n 4766
mpgoto 4766
mpechoaround $n $n arrives in a beam of bright light.
break
~
#616
Mudweiser OBJProgs/4788.prg-1~
Mudweiser OBJProgs/4788.prg-1~
wear_prog 100~
if class($n) == 0
or class($n) == 7
if inroom($n) == 4155
break
endif
if isfight($n)
mpechoat $n You may not invoke this item while fighting.
break
endif
mpechoat $n You are transported to the mage's guild.
mpechoaround $n $n invokes the symbol of $s guild and vanishes in a puff of smoke.
mptran $n 4155
mpgoto 4155
mpechoaround $n $n arrives in a beam of bright light.
break
endif
break
~
#617
Mudweiser OBJProgs/4798.prg-1~
Mudweiser OBJProgs/4798.prg-1~
remove_prog 100~
mpechoat $n `rYou have angered the mighty diety, prepare to be punished.`w
mpecho `mTimion, God of Justice, eyes the room and glares at $N.
mpechoat $n `gTimion says, 'You have disowned me, prepare to die!'
cast 'harm' $n
cast 'curse' $n
cast 'blindness' $n
mpasound `rYou hear Timion's laughter.
break
~
#618
Mudweiser OBJProgs/4798.prg-2~
Mudweiser OBJProgs/4798.prg-2~
wear_prog 100~
mpechoaround `r$n's figure shimmers briefly.`w
mpechoat $n `gTimion says, 'Congratulations $N, for choosing the ways of Justice.'`w
mpechoat $n `gTimion says, 'Be fair and just in your decisions and follow the way of the law'`w
mpechoat $n `mTimion blesses you and his likeness fades.`w
break
~
#0



#ROOMPROGS
#0



#PROGGROUPS
#12
Mayor~
Mayor wanders around the city opening and closing gates and he fights like a cleric.~
M 11
M 10
~
#26
Trump~
Trump wanders around the city saying offensive things and he fights like a cleric.~
M 25
M 10
~
#20
CityGuard~
Kills thieves and fights evil.~
M 18
M 17
~
#24
Puff~
Figure it out. :)~
M 23
M 22
~
#2000
Mudweiser MOBProgs/3011.prg~
Mudweiser MOBProgs/3011.prg~
M 200
M 201
~
#2001
Mudweiser MOBProgs/109.prg~
Mudweiser MOBProgs/109.prg~
M 202
~
#2002
Mudweiser OBJProgs/4151.prg~
Mudweiser OBJProgs/4151.prg~
O 600
O 601
~
#2003
Mudweiser OBJProgs/4152.prg~
Mudweiser OBJProgs/4152.prg~
O 602
O 603
O 604
~
#2004
Mudweiser OBJProgs/4194.prg~
Mudweiser OBJProgs/4194.prg~
O 605
O 606
O 607
~
#2005
Mudweiser OBJProgs/4199.prg~
Mudweiser OBJProgs/4199.prg~
O 608
O 609
~
#2006
Mudweiser MOBProgs/1503.prg~
Mudweiser MOBProgs/1503.prg~
M 203
M 204
M 205
M 206
M 207
M 208
M 209
M 210
M 211
M 212
M 213
M 214
M 215
M 216
M 217
M 218
M 219
M 220
M 221
M 222
M 223
~
#2007
Mudweiser MOBProgs/1523.prg~
Mudweiser MOBProgs/1523.prg~
M 224
~
#2008
Mudweiser MOBProgs/1532.prg~
Mudweiser MOBProgs/1532.prg~
M 225
~
#2009
Mudweiser MOBProgs/4719.prg~
Mudweiser MOBProgs/4719.prg~
M 226
~
#2010
Mudweiser MOBProgs/4721.prg~
Mudweiser MOBProgs/4721.prg~
M 227
~
#2011
Mudweiser MOBProgs/4739.prg~
Mudweiser MOBProgs/4739.prg~
M 228
M 229
~
#2012
Mudweiser MOBProgs/4744.prg~
Mudweiser MOBProgs/4744.prg~
M 230
~
#2013
Mudweiser MOBProgs/4745.prg~
Mudweiser MOBProgs/4745.prg~
M 231
~
#2014
Mudweiser MOBProgs/4746.prg~
Mudweiser MOBProgs/4746.prg~
M 232
~
#2015
Mudweiser OBJProgs/4749.prg~
Mudweiser OBJProgs/4749.prg~
O 610
~
#2016
Mudweiser OBJProgs/4753.prg~
Mudweiser OBJProgs/4753.prg~
O 611
O 612
~
#2017
Mudweiser OBJProgs/4783.prg~
Mudweiser OBJProgs/4783.prg~
O 613
O 614
~
#2018
Mudweiser OBJProgs/4787.prg~
Mudweiser OBJProgs/4787.prg~
O 615
~
#2019
Mudweiser OBJProgs/4788.prg~
Mudweiser OBJProgs/4788.prg~
O 616
~
#2020
Mudweiser OBJProgs/4798.prg~
Mudweiser OBJProgs/4798.prg~
O 617
O 618
~
#0



#$
