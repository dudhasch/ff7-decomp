Bottomswell is the boss fought immediately after entering the town beneath Junon.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
23  | 2500  | 100  | 42  | 30  | 36  | 96  | 69  | 1  | 0  | 550  | 52  | 1000

### Elements

Earth: 0%

Gravity: 50%

Wind: 200%

### Drops

Power Wrist (100%)

## AI Script

### Setup

    Turn off Death Handling for Bottomswell
    TempVar:HitsTilNextStage = 6
    Bottomswell's Range = 16

### Main

    TempVar:AttackAll = 0
    If (Bottomswell's IdleAnim == 1st Form) Then {
        If (Count == 0 or 1 or 2) Then {
            Choose Random Opponent
            Use Tail Attack on Target
            Count = Count + 1
        } Else {
            1/3 Chance:
            {
                TempVar:AttackAll = 1
                TempVar:ChosenAtt = Tail Attack
            }
            2/3 Chance:
            {
                Choose Random Opponent with Highest HP
                Use <Bodyblow> (Low Hit% Version) on Target
            }
            Count = 0
        }
    } Else If (Bottomswell's IdleAnim == 2nd Form) Then {
        If (Count == 0) Then {
            TempVar:AttackAll = 1
            TempVar:ChosenAtt = Moonstrike
            Count = 1
        } Else If (Count == 1) Then {
            Choose Random Opponent with Highest HP
            Use <Bodyblow> (High Hit% Version) on Target
            Count = 2
        } Else If (Count == 2) Then {
            Choose Random Opponent
            Use Moonstrike on Target
            Count = 3
        } Else {
            If (Stage < 1) Then {
                Choose Self
                Use <Fury Blast> on Target
                Bottomswell's IdleAnim = 1st Form
                Count = 0
                TempVar:HitsTilNextStage = 6
            } Else {
                Choose Random Opponent
                Use Moonstrike on Target
                1/2 Chance: Count = 0
                1/2 Chance: Count = 1
            }
        }
    } Else {
        If (Count == 0) Then {
            TempVar:Group = Opponents (incl. Dead) with neither Death nor
                                    Imprisoned Status
            If (TempVar:Group has more than one Opponent) Then {
                TempVar:PoloTarget = Choose Random Opponent in TempVar:Group
            } Else {
                TempVar:PoloTarget = [2040]
            }
            If (TempVar:PoloTarget != [2040]) Then {
                Choose TempVar:PoloTarget
                Use <Waterball> on Target
                Remove Death Status from Waterpolo
                Activate Waterpolo
                Waterpolo's HP = Waterpolo's Max HP
                Waterpolo's Physical Immunity = On
            }
            Count = 1
        } Else If (Count == 1 or 2) Then {
            Count = Count + 1
        } Else If (Count == 3) Then {
            If (Stage < 2) Then {
                Choose Self
                Use <Chill> on Target
                Bottomswell's IdleAnim = 1st Form
                Count = 0
                TempVar:HitsTilNextStage = 6
            } Else {
                Choose All Opponents
                Use Big Wave on Target
                Count = 4
            }
        } Else {
            If ((At Least One Opponent has Imprisoned Status)
                    & (At Least One Opponent has Death Status)) Then {
                Count = 1
            } Else {
                Count = 0
            }
        }
    }
    If (TempVar:AttackAll == 1) Then {
        If (2nd Opponent doesn't have Death Status) Then {
            Choose 2nd Opponent
            Use TempVar:ChosenAtt on Target
        }
        If (1st Opponent doesn't have Death Status) Then {
            Choose 1st Opponent
            Use TempVar:ChosenAtt on Target
        }
        If (3rd Opponent doesn't have Death Status) Then {
            Choose 3rd Opponent
            Use TempVar:ChosenAtt on Target
        }
    }

### Counter - General

    If (Bottomswell's HP <= 50% of Bottomswell's Max HP) Then {
        Stage = 2
        TempVar:HitsTilNextStage = 0
    } Else If (Bottomswell's HP <= 75% of Bottomswell's Max HP) Then {
        Stage = 1
        TempVar:HitsTilNextStage = 0
    } Else {
        Stage = 0
    }
    If (Bottomswell's IdleAnim == 1st Form) Then {
        Bottomswell's HurtAnim = 1
        If (TempVar:HitsTilNextStage == 0) Then {
            Choose Self
            Use <Fury Blast> on Target
            Bottomswell's IdleAnim = 2nd Form
            Count = 0
            TempVar:HitsTilNextStage = 6
            If (Stage == 2) Then {
                Choose Self
                Use <Fury Blast> on Target
                Bottomswell's IdleAnim = 3rd Form
                Count = 0
                TempVar:HitsTilNextStage = 3
            }
        } Else {
            TempVar:HitsTilNextStage = TempVar:HitsTilNextStage - 1
        }
    } Else If (Bottomswell's IdleAnim == 2nd Form) Then {
        Bottomswell's HurtAnim = 7
        If (TempVar:HitsTilNextStage == 0) Then {
            Choose Self
            Use <Fury Blast> on Target
            Bottomswell's IdleAnim = 3rd Form
            Count = 0
            TempVar:HitsTilNextStage = 3
        } Else {
            TempVar:HitsTilNextStage = TempVar:HitsTilNextStage - 1
        }
    } Else {
        Bottomswell's HurtAnim = 13
    }

### Counter - Death

    Choose All Opponents
    Use Big Wave on Target
    If (At Least One Target is Imprisoned) Then {
        Choose All Targets with Imprisoned Status
        Remove Imprisoned Status from Target
    }
    Choose Self
    Use <Vanish> on Target
    Remove Waterpolos
