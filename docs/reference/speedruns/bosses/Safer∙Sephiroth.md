Safer∙Sephiroth is the final of three bosses in the final sequence of the game.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
87  | 800001 | 6802 | 2303 | 1004 | 1005 | 1806 | 160  | 1  | 0  | 0  | 0  | 0

  1. Safer∙Sephiroth gains 30000 HP for each lv99 character (excluding Aerith). Safer∙Sephiroth loses 100 HP for each time Bizarro∙Sephiroth's head was killed (with a maximum reduction of 24900 HP). If Knights of the Round was cast on Jenova∙SYNTHESIS, Safer∙Sephiroth gains 80000 more HP.
  2. Safer∙Sephiroth's current MP is set to be equal to its maximum MP at the start of every turn, effectively giving Safer∙Sephiroth infinite MP.
  3. Safer∙Sephiroth gains 2 Attack for each lv99 character (excluding Aerith).
  4. Safer∙Sephiroth gains 20 Defense for each lv99 character (excluding Aerith).
  5. Safer∙Sephiroth gains 5 Magic Attack for each lv99 character (excluding Aerith).
  6. Safer∙Sephiroth gains 16 Magic Defense for each lv99 character (excluding Aerith).

## AI Script

### Setup

    Turn off Death Handling for Safer*Sephiroth
    TempVar:CharLv99 = GlobalVar:CharLv99
    TempVar:BzHeadDeaths = GlobalVar:BzHeadDeaths
    If (GlobalVar:JenovaKoR == 1) Then {
        TempVar:JenovaBonus = 80000
    } Else {
        TempVar:JenovaBonus = 0
    }
    TempVar:Stat = 320000
    TempVar:Stat = TempVar:Stat - 30000 * (8 - TempVar:CharLv99)
    Safer*Sephiroth's Max HP = TempVar:Stat + TempVar:JenovaBonus
    Safer*Sephiroth's HP = Safer*Sephiroth's Max HP - (TempVar:BzHeadDeaths * 100)
    Safer*Sephiroth's Att = Safer*Sephiroth's Att + 2 * TempVar:CharLv99
    Safer*Sephiroth's Def = Safer*Sephiroth's Def + 20 * TempVar:CharLv99
    Safer*Sephiroth's MAt = Safer*Sephiroth's MAt + 5 * TempVar:CharLv99
    Safer*Sephiroth's MDf = Safer*Sephiroth's MDf + 16 * TempVar:CharLv99
    Choose Self
    Use <x> on Target
    Use <Appear> on Target

### Main

    Safer*Sephiroth's MP = Safer*Sephiroth's Max MP
    Count = Count + 1
    If (Count == 1) Then {
        If (Self has Slow Status) Then {
            Choose Self
            Cast DeSpell on Target
        } Else If (TempVar:MoveSet == 1) Then {
            Choose All Opponents
            Cast DeSpell on Target
            TempVar:MoveSet = 0
        } Else {
            Choose Self
            Cast Wall on Target
            TempVar:MoveSet = 1
        }
    } Else If (Count == 2) Then {
        If (TempVar:MoveSet == 0) Then {
            Choose All Opponents
            Use Deen on Target
        } Else {
            Choose Random Opponent
            Use Shadow Flare on Target
        }
    } Else If (Count == 3) Then {
        Choose Random Opponent with Highest HP
        Use < > (Physical Attack) on Target
    } Else If (Count == 4) Then {
        Choose Self
        Use <> (Fly Up) on Target
        Stage = 1
        Safer*Sephiroth's IdleAnim = Flying High
        Safer*Sephiroth's HurtAnim = Flinch (Flying High)
        Safer*Sephiroth's Range = 16
    } Else If (Count == 5) Then {
        Choose Random Opponent
        Use Pale Horse on Target
    } Else If (Count == 6) Then {
        Choose All Opponents
        Use Super Nova on Target
    } Else If (Count == 7) Then {
        If (Safer*Sephiroth's HP > 25% of Safer*Sephiroth's Max HP) Then {
            Choose Random Opponent
            Use Break on Target
        } Else {
            Choose Random Opponent
            Use Heartless Angel on Target
        }
    } Else {
        Choose Self
        Use <> (Fly Down) on Target
        Stage = 0
        Safer*Sephiroth's IdleAnim = Flying Low
        Safer*Sephiroth's HurtAnim = Flinch (Flying Low)
        Safer*Sephiroth's Range = 1
        Count = 0
    }

### Counter - Death

    Choose Self
    If (Stage == 0) Then {
        Use <> on Target
    } Else {
        Use <> on Target
    }

### Counter - 13

    Remove Self
