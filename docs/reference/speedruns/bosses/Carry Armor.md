Carry Armor is the boss in the Junon Underwater Reactor. The boss fight consists of three separate entities: the main body, and the left and right arms. The arms can hit players or grab and immobilize them, and the main body has a powerful Lapis Laser attack.

## Stats

### Carry Armor

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
45  | 24000  | 200  | 90  | 55  | 200  | 300  | 80  | 1  | 0  | 2800  | 240  | 4000

#### Elements

Lightning (200%)

Poison (0%)

Gravity (0%)

#### Drops

God's Hand (100%)

### Right Arm

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
45  | 10000  | 100  | 80  | 55  | 200  | 300  | 80  | 1  | 0  | 1400  | 95  | 0

#### Elements

Lightning (200%)

Poison (0%)

Gravity (0%)

### Left Arm

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
45  | 10000  | 100  | 80  | 55  | 200  | 300  | 80  | 1  | 0  | 1500  | 90  | 0

#### Elements

Lightning (200%)

Poison (0%)

Gravity (0%)

## AI Script

### Carry Armor: Setup

    Turn off Death Handling for Carry Armor
    SpclChance = 4
    If (1/3 Chance) Then {
        Choose All Opponents
        Use Lapis Laser on Target
    }

### Carry Armor: Main

    If (Count == 0) Then {
        If ((At Least One Opponent doesn't have Imprisoned Status)
            AND (Rnd(1..SpclChance) == 1)) Then {
            Choose All Opponents without Imprisoned Status
            Use Lapis Laser on Target
        }
        Count = 1
    } Else {
        Count = 0
    }

### Carry Armor: Counter - General

    If (Carry Armor's IdleAnim == Both Arms Empty) Then {
        Carry Armor's HurtAnim = Flinch (Both Arms Empty)
    } Else If (Carry Armor's IdleAnim == Right Arm Full) Then {
        Carry Armor's HurtAnim = Flinch (Right Arm Full)
    } Else If (Carry Armor's IdleAnim == Left Arm Full) Then {
        Carry Armor's HurtAnim = Flinch (Left Arm Full)
    } Else If (Carry Armor's IdleAnim == Both Arms Full) Then {
        Carry Armor's HurtAnim = Flinch (Both Arms Full)
    }
    If (Right Arm's [4300] != [2040]) Then {
        Choose Right Arm's [4300]
        Use <Damage Attack> on Target
    }
    If (Left Arm's [4300] != [2040]) Then {
        Choose Left Arm's [4300]
        Use <Damage Attack> on Target
    }
    If (Carry Armor's HP <= 25% of Carry Armor's Max HP) Then {
        SpclChance = 1
    } Else If (Carry Armor's HP <= 50% of Carry Armor's Max HP) Then {
        SpclChance = 2
    } Else If (Carry Armor's HP <= 75% of Carry Armor's Max HP) Then {
        SpclChance = 3
    } Else {
        SpclChance = 4
    }

### Carry Armor: Counter - Death

    Set Right Arm as Self
    If (Right Arm's CustomVar:ArmFull = 1) Then {
        Choose Right Arm's [4300]
        Remove Target's Imprisoned Status
        Activate Target
        Use <Free Right Character> on Target
    }
    Set Left Arm as Self
    If (Left Arm's CustomVar:ArmFull = 1) Then {
        Choose Left Arm's [4300]
        Remove Target's Imprisoned Status
        Activate Target
        Use <Free Left Character> on Target
    }
    Set Carry Armor as Self
    Choose Self
    Use <Vanish> on Target
    Remove Right Arm and Left Arm

### Right Arm: Setup

    TempVar:CAGroup = Carry Armor, Right Arm and Left Arm withsame Formation ID
    Right Arm's [4300] = [2040]
    SpclChance = 16

### Right Arm: Main

    If (Right Arm's CustomVar:ArmFull == 0) Then {
        If (Rnd(1..SpclChance) == 1) Then {
            If (At Least One Opponent doesn't have Imprisoned Status) Then {
                Choose Random Opponent without Imprisoned Status
                Right Arm's [4300] = Target
                If (Right Arm's IdleAnim == Both Arms Empty) Then {
                    Use Arm Grab (Empty Left Version) on Target
                    TempVar:CAGroup's IdleAnim = Right Arm Full
                } Else If (Right Arm's IdleAnim == Left Arm Full) Then {
                    Use Arm Grab (Full Left Version) on Target
                    TempVar:CAGroup's IdleAnim = Both Arms Full
                }
                Deactivate Target
                Right Arm's CustomVar:ArmFull = 1
            }
        } Else If ((At Least One Opponent doesn't have Imprisoned Status)
                        & (1/2 Chance)) Then {
            Choose Random Opponent without Imprisoned Status
            Use <Arm Punch> on Target
        }
    }

### Right Arm: Counter - General

    If (Right Arm's IdleAnim == Both Arms Empty) Then {
        Right Arm's HurtAnim = Flinch (Both Arms Empty)
    } Else If (Right Arm's IdleAnim == Right Arm Full) Then {
        Right Arm's HurtAnim = Flinch (Right Arm Full)
    } Else If (Right Arm's IdleAnim == Left Arm Full) Then {
        Right Arm's HurtAnim = Flinch (Left Arm Full)
    } Else If (Right Arm's IdleAnim == Both Arms Full) Then {
        Right Arm's HurtAnim = Flinch (Both Arms Full)
    }
    If (Right Arm's [4300] != [2040]) Then {
        Choose Right Arm's [4300]
        Use <Damage Attack> on Target
    }
    If (Left Arm's [4300] != [2040]) Then {
        Choose Left Arm's [4300]
        Use <Damage Attack> on Target
    }
    If (Right Arm's HP <= 25% of Right Arm's Max HP) Then {
        SpclChance = 2
    } Else If (Right Arm's HP <= 50% of Right Arm's Max HP) Then {
        SpclChance = 8
    } Else If (Right Arm's HP <= 75% of Right Arm's Max HP) Then {
        SpclChance = 32
    } Else {
        SpclChance = 128
    }

### Right Arm: Counter - Death

    If (Carry Armor doesn't have Death Status) Then {
        Choose Right Arm's [4300]
        If (Target has Imprisoned Status) Then {
            Remove Target's Imprisoned Status
            Activate Target
            Use <Free Right Character> on Target
            Right Arm's [4300] = [2040]
            Right Arm's CustomVar:ArmFull = 0
            Choose Self
            Use <Vanish> on Target
            If (Left Arm's CustomVar:ArmFull == 0) Then {
                TempVar:CAGroup's IdleAnim = Both Arms Empty
            } Else {
                TempVar:CAGroup's IdleAnim = Left Arm Full
            }
        }
    } Else {
        Turn off Death Handling for Right Arm
    }

### Left Arm: Setup

    TempVar:CAGroup = Carry Armor, Right Arm and Left Arm with same Formation ID
    Left Arm's [4300] = [2040]
    SpclChance = 16

### Left Arm: Main

    If (Left Arm's CustomVar:ArmFull == 0) Then {
        If (Rnd(1..SpclChance) == 1) Then {
            If (At Least One Opponent doesn't have Imprisoned Status) Then {
                Choose Random Opponent without Imprisoned Status
                Left Arm's [4300] = Target
                If (Left Arm's IdleAnim == Both Arms Empty) Then {
                    Use Arm Grab (Empty Right Version) on Target
                    TempVar:CAGroup's IdleAnim = Left Arm Full
                } Else If (Left Arm's IdleAnim == Right Arm Full) Then {
                    Use Arm Grab (Full Right Version) on Target
                    TempVar:CAGroup's IdleAnim = Both Arms Full
                }
                Deactivate Target
                Left Arm's CustomVar:ArmFull = 1
            }
        } Else If ((At Least One Opponent doesn't have Imprisoned Status)
                        AND (1/2 Chance)) Then {
            Choose Random Opponent without Imprisoned Status
            Use <Arm Punch> on Target
        }
    }

### Left Arm: Counter - General

    If (Left Arm's IdleAnim == Both Arms Empty) Then {
        Left Arm's HurtAnim = Flinch (Both Arms Empty)
    } Else If (Left Arm's IdleAnim == Right Arm Full) Then {
        Left Arm's HurtAnim = Flinch (Right Arm Full)
    } Else If (Left Arm's IdleAnim == Left Arm Full) Then {
        Left Arm's HurtAnim = Flinch (Left Arm Full)
    } Else If (Left Arm's IdleAnim == Both Arms Full) Then {
        Left Arm's HurtAnim = Flinch (Both Arms Full)
    }
    If (Left Arm's [4300] != [2040]) Then {
        Choose Left Arm's [4300]
        Use <Damage Attack> on Target
    }
    If (Right Arm's [4300] != [2040]) Then {
        Choose Right Arm's [4300]
        Use <Damage Attack> on Target
    }
    If (Left Arm's HP <= 25% of Left Arm's Max HP) Then {
        SpclChance = 2
    } Else If (Left Arm's HP <= 50% of Left Arm's Max HP) Then {
        SpclChance = 8
    } Else If (Left Arm's HP <= 75% of Left Arm's Max HP) Then {
        SpclChance = 32
    } Else {
        SpclChance = 128
    }

### Left Arm: Counter - Death

    If (Carry Armor doesn't have Death Status) Then {
        Choose Left Arm's [4300]
        If (Target has Imprisoned Status) Then {
            Remove Target's Imprisoned Status
            Activate Target
            Use <Free Left Character> on Target
            Left Arm's [4300] = [2040]
            Left Arm's CustomVar:ArmFull = 0
            Choose Self
            Use <Vanish> on Target
            If (Right Arm's CustomVar:ArmFull == 0) Then {
                TempVar:CAGroup's IdleAnim = Both Arms Empty
            } Else {
                TempVar:CAGroup's IdleAnim = Right Arm Full
            }
        }
    } Else {
        Turn off Death Handling for Left Arm
    }
