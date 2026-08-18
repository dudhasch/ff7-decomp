Air Buster is the boss of [Reactor 5](/index.php?title=Reactor_5&action=edit&redlink=1 "Reactor 5 \(page does not exist\)").

Stats  Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
15  | 1200  | 0  | 24  | 12  | 80  | 320  | 75  | 3  | 2  | 180  | 16  | 150

**Weaknesses**

Lightning (2X)

Back attack (5X)

**Resistances**

Fire (1/2)

**Drops**

Titan Bangle

## Strategies

[![](/images/thumb/a/a7/Sg-ff7-walkthrough-part-2-25.jpg/300px-Sg-ff7-walkthrough-part-2-25.jpg)](/index.php/File:Sg-ff7-walkthrough-part-2-25.jpg)<https://samurai-gamers.com/final-fantasy-7-ffvii/air-buster-boss-guide/>

The strategy to quickly defeat Air Buster is to abuse its high back-attack damage and the fact that this fight is a side attack formation to juggle the boss's facing direction and deal massive damage quickly. This is done by using two limit breaks on Barret and Cloud, so the strategy to kill [Guard Scorpion](/index.php/Guard_Scorpion#Cloud_.2B_Barret_Limits_.28No_Guard_Skip.29 "Guard Scorpion") that ends with limit breaks on both Barret and Cloud should be used in routes that defeat Air Buster.

It is best to equip Barret with the Assault Gun and to remove Cloud's Ice materia in the train while waiting for the countdown in the second train car to deal enough damage to kill Air Buster in three attacks.

### Hold Circle

The default turn order goes Barret, Cloud, Tifa, and Air Buster starts facing Cloud by default. The fight can be won by holding circle to cue Barret's Big Shot, then Cloud's Braver, then a regular attack with Tifa. Air Buster takes back-attack damage from Big Shot, turns around to face Barret and counterattacks, then takes back-attack damage from Cloud, turns towards him and counterattacks, then takes back-attack damage from Tifa's attack and dies.

### Tifa Cloud Barret

A fast strategy for [Aps](/index.php/Aps "Aps") requires Tifa to fill her Limit Gauge in the middle of that fight. At this point in the run, she has next-to-no limit gauge, and it would be beneficial for her to go into that fight with some limit. To get some of her limit bar before that fight, you can instead make Air Buster counterattack Tifa instead of Barret by attacking in the order Tifa, then Cloud, then Barret.

### 75% Barret Limit Meter

You can kill Air Buster quickly with Barret starting the fight having only about 75% of his limit meter by using a counterattack on Barret to fill the rest of his limit. To do this, attack with Barret, then limit with Cloud, then limit with Barret.

## AI Script

### Start of battle

    Declare BackToTarget
    Declare Ammo = 4
    Declare Program = 0
    Declare SelectedTarget
    Declare TurnBroken = 0
    Declare Counter = 0
    Declare FacingTarget = 0

### Turn

    BackToTarget = 0

    If (Ammo > 0) Then:
        If (Program == 0) Then:
            Display Message "Program 1 Operation"
            Program = 1

        SelectedTarget = random opponent

        If (Self is not facing SelectedTarget) Then:
            If (TurnBroken == 0) Then:
                If (Self is facing Cloud) Then:
                    Use Turn to face Barret & Tifa

                Else:
                    Use Turn to face Cloud

            Else:
                BackToTarget = 1
                If (Self is facing Cloud AND Cloud is defeated OR Self is facing Barret and Tifa AND Barret is defeated AND Tifa is defeated) Then:
                    Ammo = 0

        If (BackToTarget == 0) Then:
            Use Big Bomber on SelectedTarget
            Ammo = Ammo - 1
            If (Ammo == 0) Then:
                Display Message "Big Bomber's out of ammo."

    Else:
        If (Program == 1) Then:
            Display Message "Program 2 Operation"
            Program = 2

        SelectedTarget = random opponent
        If (SelectedTarget == Barret OR SelectedTarget == Tifa) Then:
            If (Self is facing Cloud) Then:
                Use Rear Gun on SelectedTarget

            Else:
                Use Energy Ball on SelectedTarget

        Else:
            If (Self is facing Cloud) Then:
                Use Energy Ball on SelectedTarget

            Else:
                Use Rear Gun on Selected Target

### Counter (Attacked)

    If (Self HP < 1/5 * Self Max HP) Then:
        If (TurnBroken == 0) Then:
            Display Message "Turn Function non-operational"
            TurnBroken = 1

    SelectedTarget = last attacker
    If (Self is facing Barret and Tifa AND SelectedTarget == Cloud OR Self is facing Cloud AND (SelectedTarget == Barret OR SelectedTarget == Tifa)) Then:
        Counter = 1

    If (Counter == 1) Then:
        FacingTarget = 0
        Counter = 0
        Display Message "Counter Attack"

        If (TurnBroken == 0 AND 2/3 Chance) Then:
            If (Self is facing Cloud) Then:
                Use Turn to face Barret & Tifa
                Use Bodyblow* on SelectedTarget

            Else:
                Use Turn to face Cloud
                Use Bodyblow* on SelectedTarget

            FacingTarget = 1

        If (FacingTarget == 0) Then:
            Use Rear Gun on SelectedTarget

### Counter (Death)

    Turn off Death Handling for Self
    Use Destroyed
