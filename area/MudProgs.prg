#MOBPROGS
#1
Poison Bite~
Poisons the person the mob is fighting with.  Chance based on mob's level.~
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
rand_prog 15~
if(sgetrand(6) == 1)
  cast 'armor' $r
  break
endif
if(sgetrand(6) == 2)
  cast 'bless' $r
  break
endif
if(sgetrand(6) == 3)
  cast 'cure blindness' $r
  break
endif
if(sgetrand(6) == 4)
  cast 'cure light' $r
  break
endif
if(sgetrand(6) == 5)
  cast 'cure poison' $r
  break
endif
if(sgetrand(6) == 6)
  cast 'refresh' $r
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
  yell $n is a thief!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!
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
  yell $n is a thief!  PROTECT THE INNOCENT!  BANZAI!!!
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
  :screams 'PROTECT THE INNOCENT!!  BANZAI!!'
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
#25
Test~
Testing.~
rand_prog 100~
say #level($i)*3#
break
~
#0



#OBJPROGS
#0



#ROOMPROGS
#0



#PROGGROUPS
#12
Mayor~
Mayor wanders around the city opening and closing gates and he fights like a cleric.~
M 10
M 11
~
#20
CityGuard~
Kills thieves and fights evil.~
M 17
M 18
~
#24
Puff~
Figure it out. :)~
M 22
M 23
~
#0



#$
