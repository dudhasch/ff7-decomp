Red Dragon is a boss in the Temple of the Ancients.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
39  | 6800  | 300  | 95  | 85  | 80  | 260  | 90  | 5  | 5  | 3500  | 200  | 1000

### Elements

Fire (-100%)

Gravity (0%)

### Drops

Dragon Armlet (100%)

## Strategies

## AI Script

### Main

    Choose Random Opponent
    If (TempVar:OpeningAttack = 0) Then {
        1/4 Chance: Use <Dragon Fang> on Target
        3/4 Chance: Use Red Dragon Breath on Target
        TempVar:OpeningAttack = 1
    } Else {
        1/4 Chance: Use Red Dragon Breath on Target
        1/4 Chance: Use <Tail Attack> on Target
        1/2 Chance: Use <Dragon Fang> on Target
    }