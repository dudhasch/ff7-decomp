Jenova∙DEATH is the boss at the end of the Whirlwind Maze.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
55  | 25000  | 800  | 140  | 70  | 90  | 320  | 150  | 1  | 0  | 6000  | 400  | 5000

### Elements

Gravity (0%)

### Drops

Reflect Ring (100%)

## AI Script

### Setup

    SpclChance = 4
    1/2 Chance: Count = 3
    1/2 Chance: Count = 0

### Main

    TempVar:RedLightAttack = 0
    If (Count == 0) Then {
        If ((At Least One Opponent has Silence Status)
                OR (Jenova∙DEATH's MP < 24)) Then {
            TempVar:RedLightAttack = 1
        } Else {
            Choose Random Opponent without Silence
            Use Silence on Target
        }
        Count = 1
    } Else If (Count == 1 or 2) Then {
        Count = Count + 1
    } Else If (Count == 3) Then {
        TempVar:RedLightAttack = 1
        Count = 4
    } Else If (Count == 4) Then {
        TempVar:RedLightAttack = 1
        Count = 5
    } Else {
        Choose All Opponents
        Use Tropic Wind on Target
        Count = 0
    }
    If (TempVar:RedLightAttack == 1) Then {
        If (Rnd(1..SpclChance) == 1) Then {
            If (2nd Opponent doesn't have Death Status) Then {
                Choose 2nd Opponent
                Use Red Light on Target
            }
            If (1st Opponent doesn't have Death Status) Then {
                Choose 1st Opponent
                Use Red Light on Target
            }
            If (3rd Opponent doesn't have Death Status) Then {
                Choose 3rd Opponent
                Use Red Light on Target
            }
        } Else {
            TempVar:Random = Rnd(1..SpclChance)
            If (TempVar:Random == 1) Then {
                Choose Random Opponent
                Use Red Light on Target
            }
            If (TempVar:Random <= 2) Then {
                Choose Random Opponent
                Use Red Light on Target
            }
            Choose Random Opponent
            Use Red Light on Target
        }
    }

### Counter - General

    If (Jenova∙DEATH's HP <= 25% of Jenova*DEATH's Max HP) Then {
        SpclChance = 2
    } Else If (Jenova∙DEATH's HP <= 75% of Jenova*DEATH's Max HP) Then {
        SpclChance = 3
    }