Schizo is the boss at the end of Gaea's Cliff.

## Stats

### Right Head

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
43  | 18000  | 350  | 57  | 40  | 52  | 94  | 72  | 1  | 0  | 2200  | 120  | 1500

#### Elements

Fire (-100%)

Gravity (0%)

#### Steal

Protect Ring (8)

### Left Head

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
43  | 18000  | 350  | 57  | 40  | 52  | 94  | 72  | 1  | 0  | 2200  | 120  | 1500

#### Elements

Ice (-100%)

Gravity (0%)

#### Drops

Dragon Fang (100%)

## AI Script

### Right Setup

    TempVar:HitsUntilCounter = 4
    Turn off Death Handling for Schizo(Right)

### Right Main

    If (Count == 0 or 1 or 2) Then {
        Choose Random Opponent
        If (Schizo(Left) doesn't have Death Status) Then {
            Use <Right Breath> (LeftAlive Version) on Target
        } Else {
            Use <Right Breath> (LeftDead Version) on Target
        }
        Count = Count + 1
    } Else If (Count == 3 or 4) Then {
        Count = Count + 1
    } Else {
        If (Schizo(Left) doesn't have Death Status) Then {
            Choose Random Opponent
            Use <Double Breath> (Right Version) on Target
        } Else {
            If (2nd Opponent doesn't have Death Status) Then {
                Choose 2nd Opponent
                Use <Right Breath> (LeftDead Version) on Target
            }
            If (1st Opponent doesn't have Death Status) Then {
                Choose 1st Opponent
                Use <Right Breath> (LeftDead Version) on Target
            }
            If (3rd Opponent doesn't have Death Status) Then {
                Choose 3rd Opponent
                Use <Right Breath> (LeftDead Version) on Target
            }
        }
        Count = 0
    }

### Right Counter - General

    If (TempVar:HitsUntilCounter == 0) Then {
        Choose All Opponents
        If (Schizo(Left) doesn't have Death Status) Then {
            Use <Tremor> (LeftAlive Version) on Target
        } Else {
            Use <Tremor> (LeftDead Version) on Target
        }
        TempVar:HitsUntilCounter = 4
    } Else {
        TempVar:HitsUntilCounter = TempVar:HitsUntilCounter - 1
    }

### Right Counter - Death

    If (Schizo(Left)'s CustomVar:Dead == 0) Then {
        Choose All Opponents
        Use <Right Breath 2> (LeftAlive Version) on Target
        Schizo(Right)'s CustomVar:Dead = 1
        Both Schizo's IdleAnim = Right Head Dead
    } Else {
        Turn on Death Handling for Schizo(Left)
        Choose All Opponents
        Use <Right Breath 2> (LeftDead Version) on Target
    }

### Right Counter - PreTurn

    If (Schizo(Right)'s IdleAnim == Both Heads Alive) Then {
        Schizo(Right)'s HurtAnim = Flinch (Both Heads Alive)
    } Else {
        Schizo(Right)'s HurtAnim = Flinch (Left Head Dead)
    }
    If (([2170] & 0x02 == 0)
            & (Schizo(Left) doesn't have Death Status)) Then {
        Schizo(Right)'s HurtAnim = Flinch (???)
    }

### Left Setup

    TempVar:HitsUntilCounter = 5
    Turn off Death Handling for Schizo(Left)

### Left Main

    If (Count == 0 or 1 or 2) Then {
        Choose Random Opponent
        If (Schizo(Right) doesn't have Death Status) Then {
            Use <Left Breath> (RightAlive Version) on Target
        } Else {
            Use <Left Breath> (RightDead Version) on Target
        }
        Count = Count + 1
    } Else If (Count == 3 or 4) Then {
        Count = Count + 1
    } Else {
        If (Schizo(Left) doesn't have Death Status) Then {
            Choose Random Opponent
            Use <Double Breath> (Left Version) on Target
        } Else {
            If (2nd Opponent doesn't have Death Status) Then {
                Choose 2nd Opponent
                Use <Left Breath> (RightDead Version) on Target
            }
            If (1st Opponent doesn't have Death Status) Then {
                Choose 1st Opponent
                Use <Left Breath> (RightDead Version) on Target
            }
            If (3rd Opponent doesn't have Death Status) Then {
                Choose 3rd Opponent
                Use <Left Breath> (RightDead Version) on Target
            }
        }
        Count = 0
    }

### Left Counter - General

    If (TempVar:HitsUntilCounter == 0) Then {
        Choose All Opponents
        If (Schizo(Right) doesn't have Death Status) Then {
            Use <Tremor> (RightAlive Version) on Target
        } Else {
            Use <Tremor> (RightDead Version) on Target
        }
        TempVar:HitsUntilCounter = 5
    } Else {
        TempVar:HitsUntilCounter = TempVar:HitsUntilCounter - 1
    }

### Left Counter - Death

    If (Schizo(Right)'s CustomVar:Dead == 0) Then {
        Choose All Opponents
        Use <Left Breath 2> (RightAlive Version) on Target
        Schizo(Left)'s CustomVar:Dead = 1
        Both Schizo's IdleAnim = Left Head Dead
    } Else {
        Turn on Death Handling for Schizo(Right)
        Choose All Opponents
        Use <Left Breath 2> (RightDead Version) on Target
    }

### Left Counter - PreTurn

    If (Schizo(Left)'s IdleAnim == Both Heads Alive) Then {
        Schizo(Left)'s HurtAnim = Flinch (Both Heads Alive)
    } Else {
        Schizo(Left)'s HurtAnim = Flinch (Right Head Dead)
    }
    If (([2170] & 0x02 == 0)
            & (Schizo(Right) doesn't have Death Status)) Then {
        Schizo(Left)'s HurtAnim = Flinch (???)
    }
