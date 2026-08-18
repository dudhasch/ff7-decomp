## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
49  | 30000  | 30000  | 50  | 50  | 250  | 250  | 180  | 1  | 0  | 35000  | 3500  | 25000

### Elements

Fire (50%)

Lightning (200%)

Gravity (0%)

### Steal

Rising Sun (32)

## AI Script

### Main

    If (Count == 0) Then {
        Choose Random Opponent
        Use <Diamond Fire> on Target
        If (2/3 Chance) Then {
            Count = 1
        }
    } Else If (Count == 1) Then {
        Choose Random Opponent with highest HP
        Use <Foot Stamp> on Target
        If (3/4 Chance) Then {
            Count = 0
        }
    } Else If (Count == 2) Then {
        Print Message [Countdown]
        Choose Self
        Use <> on Target
        Diamond Weapon's IdleAnim = Countdown Stance
        Diamond Weapon's HurtAnim = Flinch (Countdown Stance)
        Stage = 1
        Choose Random Opponent
        Use <Diamond Fire> on Target
        Count = Count + 1
        Print Message [3]
    } Else If (Count == 3) Then {
        Choose Random Opponent
        Use <Diamond Fire> on Target
        Count = Count + 1
        Print Message [2]
    } Else If (Count == 4) Then {
        Choose Random Opponent
        Use <Diamond Fire> on Target
        Count = Count + 1
        Print Message [1]
    } Else {
        Print Message [0]
        Choose All Opponents
        Use Diamond Flash on Target
        Choose Self
        Use <> on Target
        Count = 0
        Stage = 0
        TempVar:L/SLimit = 0
        Diamond Weapon's IdleAnim = Normal Stance
        Diamond Weapon's HurtAnim = Flinch (Normal Stance)
    }

### Counter - Death

    If (Stage != 0) Then {
        Diamond Weapon's IdleAnim = Normal Stance
        Choose Self
        Use <> on Target
    }
    Diamond Weapon's IdleAnim = 20 (???)
    Choose Self
    Use <(Report)> on Target
    RunCmd 0x22/0x0F

### Counter - PreTurn

    TempVar:FlashLimit = 1
    If (Diamond Weapon's HP <= 50% of Diamond Weapon's Max HP) Then {
        TempVar:FlashLimit = 2
    }
    Diamond Weapon's Physical Immunity = On
    If ((Command is Summon or W-Summon or Limit) OR (Stage == 1)) Then {
        Diamond Weapon's Physical Immunity = Off
        TempVar:L/SCount = TempVar:L/SCount + 1
        If ((TempVar:L/SCount > TempVar:FlashLimit) & (Stage == 0)) Then {
            Count = 2
        }
    }
