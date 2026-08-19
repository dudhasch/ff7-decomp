Motor Ball is the last boss fought before the party exits Midgar. It has 2600 HP and has powerful arm and flame attacks.

## Stats

Level  | HP  | MP  | ATK  | Magic Atk  | Defence  | Magic Def  | Dex  | Def %  | Luck  | Exp  | AP  | Gil
---|---|---|---|---|---|---|---|---|---|---|---|---
19  | 2600  | 120  | 30  | 17  | 32  | 34  | 67  | 10  | 1  | 440  | 45  | 350

### Elements

Fire (50%)

Lightning (200%)

Gravity (0%)

### Drops

Star Pendant (100%)

## Strategy

Motor Ball follows the following script: a turn of Arm Attack, another turn of Arm Attack, a turn of Twin Burner, a turn of nothing, a turn of ramming the party, a turn of standing up, and then Rolling Fire, then repeat. In speedruns, Rolling Fire deals enough damage to kill most of or all the party, so the goal is to kill Motorball before this happens.

### Standard Strategy

In all current routes, Cloud reaches Motor Ball at level 10 assuming he has survived all previous fights.

In order to deplete Motor Ball's 2600 HP, we use a combination of Cloud using Bolt and Tifa and Barret using grenades.

On Motor Ball, Grenades do from 140 to 150 damage, with a mean damage of approximately 145.33. If each grenade did minimal damage and Cloud's bolts each did 200 damage, then 10 grenades and 6 bolts would be sufficient to exactly kill Motor Ball.

If grenades do damage closer to their mean value and Cloud's bolt's have a mean damage of around 216 or 217, then only 9 grenades and 6 bolts will be sufficient to kill Motor Ball.

The standard Motor Ball strategy is as follows:

> Motor Ball rams the party, then does a turn of Arm Attack
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Motor Ball does a turn of Arm Attack
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Motor Ball does Twin Burner
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Motor Ball rams the party
>
> Cloud Bolt
>
> Motor Ball stands up
>
> Tifa Grenade, Cloud Bolt, Barret Grenade

The final turn is taken in the order Grenade Bolt Grenade so, if the final grenade is not needed because the party's grenade and bolt rolls were sufficiently powerful, Motor Ball dies before it is thrown.

### Avoiding Counter

When Motor Ball reaches 1/8 of its maximum HP (325 HP remaining or less) and it has not stood up naturally yet, it will counter attack with Twin Burner. This is to be avoided.

Assuming mean Grenade rolls, if Cloud's bolts are doing more than 222 damage on average, then the party is in danger of this situation happening. There are two main ways to recover from a situation where Cloud is dealing too much damage with Bolt.

The first is to replace one of Cloud's bolt attacks with Ice, which does half as much damage since Motor Ball is not elementally weak to Ice.

The second is to not throw one of the eight normally thrown grenades prior to Motor Ball standing up naturally. If enough damage has been done to risk a counter attack, then the final grenade will have been skipped, so instead of skipping the final grenade, it is otherwise equivalent to skip an earlier grenade, except by doing so we don't get counter attacked.

As an example, if you notice that Cloud is dealing high damage with his bolts, the following alteration to the normal strategy may be used to avoid a counter attack:

> Motor Ball rams the party, then does a turn of Arm Attack
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Motor Ball does a turn of Arm Attack
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Motor Ball does Twin Burner
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Cloud Bolt, Tifa Grenade
>
> Motor Ball rams the party
>
> Cloud Bolt
>
> Motor Ball stands up
>
> Tifa Grenade, Cloud Bolt, Barret Grenade

### Braver

An alternate strategy may be used that uses one fewer grenade, instead replacing it with Cloud's Braver limit break, if one is available towards the start of the fight. Cloud must either have a Braver ready to be used at the start of the fight or must have just over half of his limit bar or more and be targeted by Arm Attack in one of the first two turns.

> Motor Ball rams the party, then does a turn of Arm Attack
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Motor Ball does a turn of Arm Attack
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Cloud Braver with limit priority
>
> Motor Ball does Twin Burner
>
> Cloud Bolt, Tifa + Barret Grenade
>
> Cloud Bolt, Tifa Grenade
>
> Motor Ball rams the party
>
> Cloud Bolt
>
> Motor Ball stands up
>
> Tifa Grenade, Cloud Bolt, Barret Grenade

With this strategy, we can easily fall back on the normal strategy if we fail to get a limit to be used when it is ready.

Just as before, if Bolt rolls high, throw one fewer grenade before Motor Ball stands up.

Note that if Braver deals a critical hit, one fewer grenade must be thrown before Motor Ball stands up. It may be possible that both of these situations happen, and so a total of two fewer grenades must be thrown before Motor Ball naturally stands up.

#### Manipulating HP for Braver

Because the party's starting HP in this battle is determined by their health in the motorbike chase minigame and because Motor Ball's first two Arm Attack turns attack the members of the party with the most HP, we can ensure that Cloud does not have the least HP at the start of this fight by damaging Tifa during the motor bike game. That way, the first Arm Attack hits and damages Barret, and the second hits and damages Cloud and gives him a limit break to be used in the above strategy. This can only guarantee a limit break if Cloud has at least some limit starting the fight (and if the first Arm Attack doesn't miss Barret, failing to damage him)

### Contingency Situations

If Motor Ball does a critical hit on one of its Arm Attacks, it can be possible that the final roll attack before Motor Ball stands up kills someone in the party. In this case, after Motor Ball has stood up, queue limits with Tifa and/or Barret after they do their normal attacks so their additional limits get queued as extra attacks with limit priority.

In the worst case, someone can die from Motor Ball's Twin Burner, and in this case it may not be possible to recover quickly and/or easily.

### Cloud Magic

The chances for each of Cloud's possible magic stats at level 10 and the damage ranges for each (with the Buster Sword (+2) and four green Materia (+4) equipped) are as follows:

Base  | Net  | Range  | Chance  | Percent
---|---|---|---|---
22  | 28  | 198-212  | 72/4096  | 1.76%
23  | 29  | 204-218  | 642/4096  | 15.67%
24  | 30  | 210-224  | 1282/4096  | 31.30%
25  | 31  | 212-228  | 1304/4096  | 31.84%
26  | 32  | 218-234  | 716/4096  | 17.48%
27  | 33  | 224-240  | 78/4096  | 1.90%
28  | 34  | 230-246  | 2/4096  | 0.05%

## AI Script

### Setup

    Choose Self
    Use <x> on Target
    Choose All Opponents
    Use <Highway> on Target
    Motor Ball's IdleAnim = Upright Form

### Main

    If (Count == 0 or 1) Then {
        Choose Random Opponent with Highest HP
        Use Arm Attack on Target
        If (1/2 Chance) Then {
            Choose Random Opponent with Highest HP
            Use Arm Attack on Target
        }
        Count = Count + 1
    } Else If (Count == 2) Then {
        Choose Self
        Use <> on Target
        Motor Ball's IdleAnim = Flamethrower Form
        If (Motor Ball's MP >= 16) Then {
            Choose All Opponents
            Use Twin Burner on Target
        } Else {
            Choose All Opponents
            Use <Deadly Wheel> on Target
        }
        Count = 3
    } Else If (Count == 3) Then {
        Count = 4
    } Else If (Count == 4) Then {
        Choose All Opponents
        Use <Deadly Wheel> on Target
        Count = 5
    } Else If (Count == 5) Then {
        Choose Self
        Use <> on Target
        Motor Ball's IdleAnim = Upright Form
        Count = 6
    } Else {
        If (Motor Ball's MP >= 24) Then {
            Choose All Opponents
            Use Rolling Fire on Target
        } Else {
            Choose All Opponents
            Use Deadly Wheel on Target
        }
        Count = 0
    }

### Counter - General

    If (Motor Ball's IdleAnim == Flamethrower Form) Then {
        Motor Ball's HurtAnim = Flinch (Flamethrower Form)
    } Else {
        Motor Ball's HurtAnim = Flinch (Upright Form)
    }
    If (TempVar:HPReact == 0) Then {
        If (Motor Ball's HP <= 1/8th of Motor Ball's Max HP) Then {
            If (Motor Ball's IdleAnim == Flamethrower Form) Then {
                Choose Self
                Use <> on Target
                Motor Ball's IdleAnim = Upright Form
            }
            Count = 2
            TempVar:HPReact = 1
        }
    }
