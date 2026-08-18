Jenova∙SYNTHESIS is the first of three bosses in the final sequence of the game.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
61  | 600001 | 600  | 150  | 120  | 502 | 903 | 120  | 1  | 0  | 60000  | 500  | 0

  1. Jenova∙SYNTHESIS B has 10000 HP, Jenova∙SYNTHESIS C has 8000 HP
  2. When Jenova∙SYNTHESIS B is dead, Jenova∙SYNTHESIS A has 80 defense
  3. When Jenova∙SYNTHESIS C is dead, Jenova∙SYNTHESIS A has 150 magic defense

## AI Script

The battle consists of three instances of Jenova∙SYNTHESIS: one that acts as the main body, and two that act as tentacles. They all have the same AI scripts, which is why the scripts check which enemy the current executor is.

### Setup

    TempVar:Ultima = 9
    GlobalVar:JenovaKoR = 0
    If (Self is Jenova*SYNTHESISA) Then {
        Turn on Death Handling for Jenova*SYNTHESISA
        Jenova*SYNTHESISA's [4278] = 3
        TempVar:AvgPLv: = Average Party Level
        TempVar:CharNum: = Number of Party Members
        TempVar:Cloud'sLv = Cloud's Level
        TempVar:CharLv99:= No. of Characters at L99
        TempVar:Char<Lv45 = No. of Characters below L45
        TempVar:Char<Lv35 = No. of Characters below L35
    }
    If (Self is Jenova*SYNTHESISB) Then {
        Jenova*SYNTHESISB's Max HP = 10000
        Jenova*SYNTHESISB's: HP = 10000
        Jenova*SYNTHESISB's [4278] = 16
        Jenova*SYNTHESISB's EXP Value = 0
        Jenova*SYNTHESISB's Gil Value = 0
    }
    If (Self is Jenova*SYNTHESISC) Then {
        Jenova*SYNTHESISC's Max HP = 8000
        Jenova*SYNTHESISC's: HP = 8000
        Jenova*SYNTHESISC's [4278] = 20
        Jenova*SYNTHESISC's EXP Value = 0
        Jenova*SYNTHESISC's Gil Value = 0
    }

### Main

    If (Self is Jenova*SYNTHESISA) Then {
        If (TempVar:Ultima < 6) Then {
            If (TempVar:Ultima == 0) Then {
                Choose All Opponents
                Use Ultima on Target
                If (TempVar:Cloud'sLv > 60) Then {
                    GlobalVar:BizarroParty = 3
                } Else {
                    GlobalVar:BizarroParty = 2
                }
                If ((Count < 6) OR (TempVar:Char<Lv45 == 0)) Then {
                    GlobalVar:BizarroParty = 3
                }
                If ((TempVar:Char<Lv45 > 0) OR (TempVar:CharNum < 8)
                        OR (TempVar:AvgPLv < 68)) Then {
                    GlobalVar:BizarroParty = 2
                }
                If ((Count > 12) OR (TempVar:AvgPLv < 54)
                        OR (TempVar:Char<Lv35 > 0)) Then {
                    GlobalVar:BizarroParty = 1
                }
                GlobalVar:BzHeadDeaths = 0
                GlobalVar:BzDead-MainHead = 0
                GlobalVar:BzDead-MainCore = 0
                GlobalVar:BzDead-MainRMgc = 0
                GlobalVar:BzDead-MainLMgc = 0
                GlobalVar:BzDead-MainRShl = 0
                GlobalVar:BzDead-MainRArm = 0
                GlobalVar:BzDead-Sub1Core = 0
                GlobalVar:BzDead-Sub1LArm = 0
                GlobalVar:BzDead-Sub1LShl = 0
                GlobalVar:BzDead-Sub1RArm = 0
                GlobalVar:BzDead-Sub2Core = 0
                GlobalVar:BzDead-Sub2LArm = 0
                GlobalVar:BizarroMainStart = 0
                GlobalVar:BizarroSub1Start = 0
                GlobalVar:BizarroSub2Start = 0
                GlobalVar:CharLv99 = TempVar:CharLv99
                Remove All Enemies
            } Else If (TempVar:Ultima == 1) Then {
                Print Message [1]
            } Else If (TempVar:Ultima == 2) Then {
                Print Message [2]
            } Else If (TempVar:Ultima == 3) Then {
                Print Message [3]
            } Else If (TempVar:Ultima == 4) Then {
                Print Message [4]
            } Else If (TempVar:Ultima == 5) Then {
                Print Message [5]
            }
            TempVar:Ultima = TempVar:Ultima - 1
        } Else {
            If (Count < 250) Then {
                Count = Count + 1
            } Else {
                Count = Count - 9
            }
            Choose Random Opponent
            TempVar:Sequence = (Count MOD 5)
            If (TempVar:Sequence == 1 or 3) Then {
                If (Jenova*SYNTHESISB doesn't have Death Status) Then {
                    If (Jenova*SYNTHESISC doesn't have Death Status) Then {
                        Use <Repeating Slap> (C Alive Version) on Target
                    } Else {
                        Use <Repeating Slap> (C Dead Version) on Target
                    }
                } Else {
                    Jenova*SYNTHESISA's Def = 80
                }
            } Else If (TempVar:Sequence == 2 or 4) Then {
                If (Jenova*SYNTHESISC doesn't have Death Status) Then {
                    If (Jenova*SYNTHESISB doesn't have Death Status) Then {
                        Use Absorb (B Alive Version) on Target
                    } Else {
                        Use Absorb (B Dead Version) on Target
                    }
                } Else {
                    Jenova*SYNTHESISA's MDf = 150
                }
            } Else {
                If ((Jenova*SYNTHESISC has Death Status)
                        AND (Jenova*SYNTHESISB has Death Status)) Then {
                    Choose Self
                    Use <Right Hand Revived> on Target
                    Remove all Statuses from Jenova*SYNTHESISC
                    Jenova*SYNTHESISC's HP = [Jenova*SYNTHESISC's Max HP / 4]
                    All Allies' IdleAnim = Left Arm Dead
                    Use <Left Hand Revived> on Target
                    Remove all Statuses from Jenova*SYNTHESISB
                    Jenova*SYNTHESISB's HP = [Jenova*SYNTHESISB's Max HP / 4]
                    All Allies' IdleAnim = Normal
                    All Allies' HurtAnim = Flinch (Normal)
                    Jenova*SYNTHESISA's Def = 100
                    Jenova*SYNTHESISA's MDf = 180
                } Else If ((Jenova*SYNTHESISC has Death Status)
                        AND (Jenova*SYNTHESISB doesn't have Death Status)) Then {
                    Choose Random Opponent
                    If (Jenova*SYNTHESISA's MP >= 34) Then {
                        Use Stop on Target
                    }
                } Else If ((Jenova*SYNTHESISC doesn't have Death Status)
                        AND (Jenova*SYNTHESISB has Death Status)) Then {
                    Choose All Allies
                    If (Jenova*SYNTHESISA's MP >= 64) Then {
                        Use Cure3 on Target
                    }
                } Else {
                    Choose All Opponents
                    If (Jenova*SYNTHESISA's MP >= 36) Then {
                        Use Bio2 on Target
                    }
                }
            }
        }
    }

### Counter - General

    If (Self is Jenova*SYNTHESISA) Then {
        If ((Jenova*SYNTHESISA's HP < 15000) AND (TempVar:Ultima > 5)) Then {
            TempVar:Ultima = 5
        }
        If ((Last Command was Summon or W-Summon)
                AND (Last Attack was Knights of Round)) Then {
            GlobalVar:JenovaKoR = 1
        }
    }

### Counter - Death

    If (Self is Jenova*SYNTHESISA) Then {
        If ((Last Command was Summon or W-Summon)
                AND (Last Attack was Knights of Round)) Then {
            GlobalVar:JenovaKoR = 1
        }
        Remove All Enemies
        If (TempVar:Cloud'sLv > 60) Then {
            GlobalVar:BizarroParty = 3
        } Else {
            GlobalVar:BizarroParty = 2
        }
        If ((Count < 6) OR (TempVar:Char<Lv45 == 0)) Then {
            GlobalVar:BizarroParty = 3
        }
        If ((TempVar:Char<Lv45 > 0) OR (TempVar:CharNum < 8)
                OR (TempVar:AvgPLv < 68)) Then {
            GlobalVar:BizarroParty = 2
        }
        If ((Count > 12) OR (TempVar:AvgPLv < 54)
                OR (TempVar:Char<Lv35 > 0)) Then {
            GlobalVar:BizarroParty = 1
        }
        GlobalVar:BzHeadDeaths = 0
        GlobalVar:BzDead-MainHead = 0
        GlobalVar:BzDead-MainCore = 0
        GlobalVar:BzDead-MainRMgc = 0
        GlobalVar:BzDead-MainLMgc = 0
        GlobalVar:BzDead-MainRShl = 0
        GlobalVar:BzDead-MainRArm = 0
        GlobalVar:BzDead-Sub1Core = 0
        GlobalVar:BzDead-Sub1LArm = 0
        GlobalVar:BzDead-Sub1LShl = 0
        GlobalVar:BzDead-Sub1RArm = 0
        GlobalVar:BzDead-Sub2Core = 0
        GlobalVar:BzDead-Sub2LArm = 0
        GlobalVar:BizarroMainStart = 0
        GlobalVar:BizarroSub1Start = 0
        GlobalVar:BizarroSub2Start = 0
        GlobalVar:CharLv99 = TempVar:CharLv99
    } Else If (Jenova*SYNTHESISA does not have Death Status) Then {
        Choose Jenova*SYNTHESISA
        If (Self is Jenova*SYNTHESISB) Then {
            Target's Def = 80
        } Else {
            Target's MDf = 150
        }
        If (Jenova*SYNTHESISC doesn't have Death Status) Then {
            If (Jenova*SYNTHESISB has Death Status) Then {
                All Allies' IdleAnim = Left Arm Dead
                All Allies' HurtAnim = Flinch (Left Arm Dead)
            }
        } Else If (Jenova*SYNTHESISB has Death Status) Then {
            All Allies' IdleAnim = Both Arms Dead
            All Allies' HurtAnim = Flinch (Both Arms Dead)
        } Else {
            All Allies' IdleAnim = Right Arm Dead
            All Allies' HurtAnim = Flinch (Right Arm Dead)
        }
    }
