Demons Gate is the last boss in the Temple of the Ancients.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
45  | 10000  | 400  | 150  | 96  | 100  | 450  | 120  | 0  | 0  | 3800  | 220  | 4000

### Elements

Poison (0%)

Gravity (0%)

### Drops

Gigas Armlet (100%)

## Strategies

### Tifa with Powersoul

### Yoshiyuki with Fury Ring

#### No Long Range

## AI Script

### Setup

    SpclChance = 8
    Count = Rnd(0..4)

### Main

    If (Count == 0 or 1) Then {
        If (At Least One Opponent has neither Slow-numb nor Petrify Status) Then {
            Choose Random Opponent without Slow-numb and Petrify Status
            Use <Falling Rocks> on Target
        }
        If (Rnd(1..SpclChance) == 1) Then {
            If (At Least One Opponent has neither Slow-numb nor Petrify Status) Then {
                Choose Random Opponent without Slow-numb and Petrify Status
                Use <Falling Rocks> on Target
            }
        }
        Count = Count + 1
    } Else If (Count = 2) Then {
        If (At Least One Opponent has neither Slow-numb nor Petrify Status) Then {
            Choose All Opponents without Slow-numb and Petrify Status
            Use Cave-in on Target
        }
        Count = 3
    } Else If (Count = 3) Then {
        If (At Least One Opponent has neither Slow-numb nor Petrify Status) Then {
            If (Rnd(1..SpclChance) == 1) Then {
                Choose Random Opponent without Slow-numb and Petrify Status
                Use Petrif-Eye on Target
            } Else {
                Choose Random Opponent without Slow-numb and Petrify Status
                Use <Falling Rocks> on Target
            }
        }
        Count = Rnd(0..2)
    } Else If (Count = 4) Then {
        Choose Self
        Use <Advance> on Target
        Demons Gate's IdleAnim = Demon Rush Position
        1/3 Chance: Count = 5
        2/3 Chance: Count = 6
    } Else If (Count = 5) Then {
        Count = 6
    } Else {
        If (At Least One Opponent doesn't have Petrify Status) Then {
            Choose All Opponents without Petrify Status
            Use Demon Rush on Target
        }
        Choose Self
        Use <Retreat> on Target
        Demons Gate's IdleAnim = Normal Position
        Stage = 0
        Count = 0
    }

### Counter - General

    If (Demons Gate's IdleAnim = Normal Position) Then {
        Demons Gate's HurtAnim = Flinch (Normal Position)
    } Else {
        Demons Gate's HurtAnim = Flinch (Demon Rush Position)
    }
    If (Demons Gate's HP <= 25% of Demons Gate's Max HP) Then {
        If ((SpclChance == 3) & (Stage == 0)) Then {
            Count = 4
            Stage = 1
        }
        SpclChance = 2
    } Else If (Demons Gate's HP <= 50% of Demons Gate's Max HP) Then {
        If ((SpclChance == 4) & (Stage == 0)) Then {
            Count = 4
            Stage = 1
        }
        SpclChance = 3
    } Else If (Demons Gate's HP <= 75% of Demons Gate's Max HP) Then {
        If ((SpclChance == 8) & (Stage == 0)) Then {
            Count = 4
            Stage = 1
        }
        SpclChance = 4
    } Else {
        SpclChance = 8
    }
