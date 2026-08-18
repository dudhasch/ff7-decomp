Aps is the boss of the [Sewers](/index.php?title=Sewers&action=edit&redlink=1 "Sewers \(page does not exist\)"). It is the first scripted encounter of the game that can be encountered in a preemptive formation.

Stats  Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
18  | 1800  | 0  | 40  | 40  | 36  | 160  | 63  | 4  | 5  | 240  | 22  | 253

**Weaknesses**

Fire (2X)

Back attack (4X)

**Drops**

Phoenix Down

## Strategies

### Preemptive Encounter

Aps can be fought as a [Preemptive Boss Battle](/index.php/Preemptive_Boss_Battles "Preemptive Boss Battles").

#### Fast Aps

Enter the battle with a braver on Cloud.

Hold Up+Circle to throw grenades with everyone, then hold Down+Circle to Braver with Cloud. Aps will do a Sewer Tsunami. Throw grenades with everyone until Aps dies (reselect grenades with Cloud the first time)

#### Slow Aps

If you don't remove any materia from Cloud, he is more likely to do too little damage with his Braver to kill Aps with the fastest strategy, requiring him to use a second Sewer Tsunami for additional damage. We can speed up this version of the fight by choosing who we throw grenades with to get the fastest possible grenade throwing animations.

After the first Sewer Tsunami, defend on Aerith's next turn and queue a grenade with Cloud and then Tifa. Aps will use a Sewer Tsunami after Tifa's grenade, then queue another grenade with Tifa to be used after that should kill.

Load video

YouTube

YouTube might collect personal data. [Privacy Policy](https://www.youtube.com/howyoutubeworks/user-settings/privacy/)

ContinueDismiss

### Non-Preemptive Encounter

Using a Magic attack on Aps will change its AI mode so, for the next four turns, it does: nothing, Sewer Tsunami, nothing, Reverse Sewer Tsunami.

Aps starts the fight with a Sewer Tsunami. Cloud uses Bolt for his first attack on this fight for this script manipulation, then everyone throws grenades. Aps will use one more sewer tsunami.

If the first Sewer Tsunami of the fight gets Tifa her Limit Break, then her fast ATB recovery speed allows her to get another grenade in before Aps' next Sewer Tsunami, and that results in dealing enough damage before Aps' Reverse Sewer Tsunami to kill. Otherwise, Aps will do a Reverse Sewer Tsunami before the last grenade is thrown. A strategy is used on [Air Buster](/index.php/Air_Buster#Tifa_Cloud_Barret "Air Buster") to make this likely to happen.

## AI Script

### Start of Battle

    Declare OpeningAttack = 0
    Declare SewerTsunami = 0
    Declare Count = 0
    Declare HPDanger = 0

### Turn

    If (OpeningAttack == 0) Then:
        Use Sewer Tsunami* on all targets
        OpeningAttack = 1

    ElseIf (SewerTsunami == 0) Then:
        3/4 Chance:
            If (any opponent is on the back row) Then:
                Use Tail Attack on random opponent on the back row

            Else:
                Use Tail Attack on random opponent

        1/4 Chance:
            If (2nd opponent is alive) Then:
                Use Lick on 2nd opponent

            Else:
                Use Lick on random opponent

    ElseIf (Count == 0) Then:
        Count = 1

    ElseIf (Count == 1) Then:
        Use Sewer Tsunami* on all targets
        Count = 2

    ElseIf (Count == 2) Then:
        Count = 3

    ElseIf (Count == 3) Then:
        Use Sewer Tsunami* on all targets
        Count = 0
        SewerTsunami = 0

### Counter (Physical)

    If (HPDanger == 0 AND Self HP < 1/3 * Self Max HP) Then:
        SewerTsunami = 1
        HPDanger = 1

### Counter (Magic)

    SewerTsunami = 1

## Statistics

### Preemptive Battle

Chance of killing with the standard strat based on the number of materia on Cloud and random stat rolls:

  * with no materia removed: 73.78%
  * with 1 materia removed: 95.45%
  * with 2 materia removed: 99.49%

Given a Braver damage roll, here are the chances that Aps will die on the last grenade and not use a Sewer Tsunami:

    446 0.0008886751773037042
    447 0.0014328603145875786
    448 0.0022466203901531927
    449 0.0034327514862553515
    450 0.005120740150693457
    451 0.007469385130216481
    452 0.010668291818029765
    453 0.014937801163253526
    454 0.020526945096876182
    455 0.027709094249441388
    456 0.03677508323063365
    457 0.04802375965930096
    458 0.061750106412412575
    459 0.07823132742107279
    460 0.09771152878667483
    461 0.12038583468288117
    462 0.1463849343972502
    463 0.17576114593370085
    464 0.20847708662825104
    465 0.2443979549690235
    466 0.2832882575480464
    467 0.3248135598564251
    468 0.36854750051974794
    469 0.4139839253727942
    470 0.4605536149371729
    471 0.507644723323845
    472 0.5546257537967373
    473 0.600869700334488
    474 0.6457779014151636
    475 0.6888021754097264
    476 0.729463936145332
    477 0.7673692255008501
    478 0.8022189200622999
    479 0.8338137352813035
    480 0.8620540388296383
    481 0.8869348541654257
    482 0.9085367412314607
    483 0.9270134602211335
    484 0.9425774576861389
    485 0.9554842629325182
    486 0.9660168428763205
    487 0.9744708469399311
    488 0.9811414978639293
    489 0.9863126621444132
    490 0.9902483928488649
    491 0.9931870134933356
    492 0.9953376264772432
    493 0.9968787869482191
    494 0.9979589826193328
    495 0.9986985040061511
    496 0.9991922749455318
    497 0.9995132348985697
    498 0.9997159190626989
    499 0.9998399588328338
    500 0.9999133065537497
    501 0.9999550623850385
    502 0.9999778434855502
    503 0.9999896861075768
    504 0.9999955089638669
    505 0.9999981917025453
    506 0.9999993362904908
    507 0.9999997818408582
    508 0.9999999371639363
    509 0.9999999845521945
    510 0.9999999968588178
    511 0.9999999994914497
    512 0.9999999999375799
    513 0.9999999999945866
    514 0.9999999999997062
    515 0.9999999999999926
    516+ 1.0
