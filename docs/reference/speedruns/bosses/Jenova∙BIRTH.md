Jenova∙BIRTH is the boss fought in the Cargo Ship between Junon and Costa del Sol. It has powerful laser attacks, as well as a gas attack and the ability to inflict Stop on party members.

This boss can be fought in a [preemptive formation](/index.php/Preemptive_Boss_Battles#Jenova.E2.80.A2BIRTH "Preemptive Boss Battles").

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
25  | 4000  | 110  | 38  | 30  | 56  | 180  | 60  | 1  | 10  | 680  | 64  | 800

### Elements

Gravity (0%)

### Drops

White Cape (100%)

## AI Script

### Setup

    1/2 Chance: Count = 3
    1/2 Chance: Count = 0

### Main

    TempVar:LaserAttack = 0
    If (Count == 0) Then {
        If (At Least One Opponent has Stop Status) Then {
            Choose Random Opponent with Stop Status
            Use Gas on Target
        } Else {
            If (Jenova*BIRTH's MP >= 34) Then {
                Choose Random Opponent
                Use Stop on Target
            }
        }
        Count = 1
    } Else If (Count == 1 or 2) Then {
        Count = Count + 1
    } Else If (Count == 3) Then {
        TempVar:LaserAttack = 1
        TempVar:ChosenAtt = Laser
        Count = 4
    } Else If (Count == 4) Then {
        Choose All Opponents
        Use Tail Laser on Target
        If (1/2 Chance) Then {
            Choose All Opponents
            Use Tail Laser on Target
        }
        Count = 5
    } Else If (Count == 5) Then {
        TempVar:LaserAttack = 1
        TempVar:ChosenAtt = W-Laser
        Count = 6
    } Else {
        If (At Least One Opponent has Stop Status) Then {
            Choose Random Opponent with Stop Status
            Use Gas on Target
        } Else {
            If (1/3 Chance) Then {
                Choose Random Opponent
                Use Gas on Target
            }
        }
        Count = 0
    }
    If (TempVar:LaserAttack == 1) Then {
        If (1/3 Chance) Then {
            If (2nd Opponent has neither Death nor Stop Status) Then {
                Choose 2nd Opponent
                Use TempVar:ChosenAtt on Target
            }
            If (1st Opponent has neither Death nor Stop Status) Then {
                Choose 1st Opponent
                Use TempVar:ChosenAtt on Target
            }
            If (3rd Opponent has neither Death nor Stop Status) Then {
                Choose 3rd Opponent
                Use TempVar:ChosenAtt on Target
            }
        } Else {
            If (At Least One Opponent doesn't have Stop Status) Then {
                Choose Random Opponent without Stop Status
            } Else {
                Choose Random Opponent
            }
            Use TempVar:ChosenAtt on Target
        }
    }
