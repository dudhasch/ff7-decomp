Dyne is a boss fought in the Corel Prison with Barret alone. Dyne is usually a very safe fight, but can rarely kill the player if there are multiple critical hits. If a Right Arm is stolen from a Bomb enemy, it is usually thrown here.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
23  | 1200  | 20  | 32  | 25  | 64  | 250  | 55  | 1  | 0  | 600  | 55  | 750

### Elements

Gravity (0%)

### Drops

Silver Armlet (100%)

## Strategy

### Molotov

Four Molotovs can be thrown at Dyne to kill.

### Right Arm

If a Right Arm item is stolen from a Bomb enemy on the bridge before Corel, it can be thrown at Dyne to deal 1300+ damage and kill him instantly.

## AI Script

### Setup

    Stage = 3
    Turn off Death Handling for Dyne

### Main

    If (Stage == 0) Then {
        Choose Random Opponent
        Use Needle Gun on Target
        Choose All Opponents
        Use Molotov Cocktail on Target
    } Else If (Stage == 1) Then {
        Choose Random Opponent
        Use Needle Gun on Target
        Choose Random Opponent
        Use S-Mine on Target
    } Else If (Stage == 2) Then {
        Choose Random Opponent
        1/4 Chance: Use S-Mine on Target
        3/4 Chance: Use Needle Gun on Target
    } Else {
        Choose Random Opponent
        Use Needle Gun on Target
    }

### Counter - General

    If (Dyne's HP <= 25% of Dyne's Max HP) Then {
        Stage = 0
    } Else If (Dyne's HP <= 50% of Dyne's Max HP) Then {
        Stage = 1
    } Else If (Dyne's HP <= 75% of Dyne's Max HP) Then {
        Stage = 2
    } Else {
        Stage = 3
    }

### Counter - Death

    If (1/2 Chance) Then {
        Choose All Opponents
        Use Molotov Cocktail on Target
    }
    Print Message [Dyne "Urgh!"]
    RunCmd 0x24/0x00
    Remove Sleep/Poison/Haste/Slow/Death-sentence/Berserk Statuses from Dyne
    Choose Self
    Use <> on Target
    Dyne's IdleAnim = Kneeling
