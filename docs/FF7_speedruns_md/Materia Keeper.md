Materia Keeper is the boss in Mt. Nibel.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
38  | 8400  | 300  | 90  | 12  | 100  | 280  | 90  | 10  | 10  | 3000  | 200  | 2400

### Elements

Fire (-100%)

Gravity (0%)

### Drops

Jem Ring (100%)

## Strategies

### 7777

#### Cosmo Canyon Skip

Cloud should always be level 13 here. The following is a map of the chance of each of Cloud's possible base magic stats at this level:

Base  | Chance  | Percent
---|---|---
25  | 576/2097152  | 0.03%
26  | 117636/2097152  | 5.61%
27  | 413716/2097152  | 19.73%
28  | 698216/2097152  | 33.29%
29  | 624896/2097152  | 29.80%
30  | 215572/2097152  | 10.28%
31  | 26540/2097152  | 1.27%

#### Cosmo Canyon

### Deathblow (100% Only)

## AI Script

### Setup

    Count = Rnd(0..4)
    SpclChance = 5

### Main

    If (Count == 0 or 1 or 2) Then {
        If (Rnd(1..SpclChance) == 1) Then {
            Choose Random Opponent
            Use Hell Combo on Target
        } Else {
            Choose Random Opponent
            1/2 Chance: Use <Keyclaw> on Target
            1/2 Chance: Use Big Horn on Target
        }
        Count = Count + 1
    } Else If (Count = 3) Then {
        If ((SpclChance < 4) & (Materia Keeper's MP >= 24) Then {
            Choose Self
            Use Cure2 on Target
            Count = 4
        } Else {
            If (Rnd(1..SpclChance) == 1) Then {
                Choose Random Opponent
                Use Hell Combo on Target
                Count = 0
            } Else {
                Choose Random Opponent
                1/2 Chance: Use <Keyclaw> on Target
                1/2 Chance: Use Big Horn on Target
                Count = Rnd(0..2)
            }
        }
    } Else {
        If (Materia Keeper's MP >= 20) Then {
            Choose All Opponents
            Use Trine on Target
        } Else {
            Choose Random Opponent
            Use Hell Combo on Target
        }
        Count = 0
    }

### Counter - General

    If (Materia Keeper's HP <= 25% of Materia Keeper's Max HP) Then {
        SpclChance = 2
    } Else If (Materia Keeper's HP <= 50% of Materia Keeper's Max HP) Then {
        SpclChance = 3
    } Else If (Materia Keeper's HP <= 75% of Materia Keeper's Max HP) Then {
        SpclChance = 4
    } Else {
        SpclChance = 5
    }
