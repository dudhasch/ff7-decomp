Palmer is a boss fought in Rocket Town on the party's first visit.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
38  | 6000  | 240  | 100  | 25  | 100  | 200  | 60  | 50  | 0  | 1800  | 98  | 5000

### Elements

Gravity (0%)

### Drops

Edincoat (100%)

## Strategies

### Poison + Molotovs

### Aqualung

## AI Script

### Main

    1/4 Chance:
    {
        If (At Least One Opponent doesn't have Sadness Status) Then {
            Print Message [Palmer "Heh-hic-heh-hic!"]
            Choose Random Opponent without Sadness Status
            Use <Incite> on Target
        }
    }
    1/4 Chance:
    {
        If (Palmer's MP >= 22) Then {
            Print Message [Mako Gun]
            Choose Random Opponent
            Use <Fire2> on Target
        }
    }
    1/4 Chance:
    {
        If (Palmer's MP >= 22) Then {
            Print Message [Mako Gun]
            Choose Random Opponent
            Use <Ice2> on Target
        }
    }
    1/4 Chance:
    {
        If (Palmer's MP >= 22) Then {
            Print Message [Mako Gun]
            Choose Random Opponent
            Use <Bolt2> on Target
        }
    }

### Counter - Death

    Remove Sleep/Poison/Haste/Slow/Death-sentence/Berserk Statuses from Palmer
    Print Message [Palmer "Heh heh hic!"]
    Choose Self
    Use Palmer "Ugh!" on Target
