Jenova∙LIFE is a boss fought at the very end of Disc 1 in the Forgotten City. It uses water-magic attacks, including the Enemy Skill Aqualung.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
50  | 10000  | 300  | 128  | 40  | 110  | 290  | 140  | 10  | 40  | 4000  | 350  | 1500

### Elements

Earth (200%)

Gravity (0%)

Water (-100%)

### Drops

Wizard Bracelet (100%)

## Strategies

## AI Script

### Setup

    Count = Rnd(0..3)
    SpclChance = 5

### Main

    If (Count == 0 or 1) Then {
        Choose Random Opponent
        Use Blue Light on Target
        If (Rnd(1..SpclChance) == 1) Then {
            Choose Random Opponent
            Use Blue Light on Target
        }
        Count = Count + 1
    } Else If (Count == 2) Then {
        Choose Random Opponent
        Use Blue Flame on Target
        If (Rnd(1..SpclChance) == 1) Then {
            Count = 3
        } Else {
            Count = 0
        }
    } Else {
        Choose All Opponents
        Use Aqualung on Target
        Count = 0
    }

### Counter - General

    If (Jenova*LIFE's HP <= 25% of Jenova*LIFE's Max HP) Then {
        SpclChance = 2
    } Else If (Jenova*LIFE's HP <= 50% of Jenova*LIFE's Max HP) Then {
        SpclChance = 3
    } Else If (Jenova*LIFE's HP <= 75% of Jenova*LIFE's Max HP) Then {
        SpclChance = 4
    } Else {
        SpclChance = 5
    }

### Counter - Magical

    If ((Jenova*LIFE doesn't have Reflect Status)
            AND (Jenova*LIFE's MP >= 30)) Then {
        Choose Self
        Use Reflect on Target
    }