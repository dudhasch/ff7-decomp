## Intended behavior of the encounter system

The general idea of all encounter generators in Final Fantasy VII is as follows: As you spend more time without getting an encounter, it should become more and more likely that you get an encounter.

This is codified in the idea of the Danger value, which determines how likely it is that you get an encounter at any given point in time. A low danger value means that it is unlikely for you to get an encounter, and a high danger value means that it is more likely to get an encounter. Your danger value increases as you move around, and after each random battle, it resets back to zero.

In addition, the enemies you fight in battle should be unpredictable, and sometimes you fight a battle where the enemies attack you from the back, you attack the enemies from their back, you attack the enemies from both sides, or the enemies attack you from both sides. However, it should be less likely that you fight the same enemies twice in a row, because even if that statistically happens a good amount of the time, it just doesn’t *feel* random if you keep getting the same fight in a row three or four times. It also doesn’t feel random if you never get the same fight twice in a row, but that should happen less often than if the fights were generated independently.

## Exploiting the encounter system

“Time” as mentioned above is intended to be made proportional to total distance traveled. On field maps, Cloud has two forms of movement: walking and running. If the developers wanted to make it so you got battles after traveling about the same distance no matter if you were walking or running, they had two options for how to modify how the encounter system works while walking:

  1. Multiply the distance between each battle check by (walk speed / run speed)
  2. Multiply the danger value increment per battle check by (walk speed / run speed)

If the developers chose the first option, then walking would truly behave like slow running, but this would probably have been harder to implement. The developers chose the second option, which is one of two reasons why encounters can be manipulated.

The second reason is because the random number generator used to determine when battles occur is, to put it lightly, not very good. It consists of a hardcoded shuffled list of numbers which is provided below, and random numbers are pulled by indexing into this list and incrementing the index for the next random number.

## When do you get encounters? (Steps and Danger)

While moving on any field map, the game will increment a value called Step Fraction once per frame. This is a value that ranges from 0 to 7 step fractions, and when it exceeds 7, it is reset back to 0 and, if the current field map is hostile and has encounters enabled, an Encounter Check is made. FF7’s field map module runs at 30 frames per second, so an encounter check is made every 0.27 seconds while constantly moving, no matter if Cloud is walking or running currently.

On every encounter check, the game will increment your Danger value by an amount based on the current field. It then will randomly decide whether, if the subsequent check succeeds, the encounter will attempt to be a preemptive attack. Then, it will randomly determine a target danger threshold. If your Danger value is equal to or greater than this threshold, a battle is triggered. However, the process for determining the random number generation for this step is entirely predictable.

These random numbers are determined by two values: Step ID (often written as stepid) and Offset. Both of these are 1-byte values and are what determine what random numbers are generated for target danger thresholds and preemptive flag values.

Every time the game uses this generator for a random number, it increments stepid by 1 until it wraps at 256, then if it has wrapped, it increments offset by 13, which also wraps by 256. The game then fetches and uses the RNG value at this index from the field map RNG table and subtracts the current offset value, and this value is the random number.

As you expend movement on a hostile field, this random number generator is called twice for each encounter check (once for the preemptive flag check and a second time for the danger threshold check), so stepid will increment by 2 for each encounter check. They both wrap at 256, so stepid is incremented on 128 encounter checks before cycling and offset increments 256 times before cycling (since 13 and 256 are coprime), so the encounter table loops after 128 * 256 = 32768 encounter checks.

Because stepid and offset are all that determine what these random values are and because they increment each time a random number is generated, these values are collectively referred to as your position in the encounter table.

The model used to describe the behavior of field map random encounters in Final Fantasy VII is known as the Encounter Table. This is visualized as a graph with the X axis representing position in the encounter table and the Y axis representing danger. Battle checks are visualized as lines descending from the top of the graph to the Y-position that corresponds to their minimum danger threshold.

[Error creating thumbnail: limit.sh: timed out executing command "'/usr/bin/convert' '-quality' '95' '-background' 'white' '/var/www/html/images/2/2c/Stepgraph_1.png' '-thumbnail' '300x134!' '-set' 'comment' 'File source: http://ff7speedruns.com/index.php/File:Stepgraph_1.png' '+set' 'Thumb::URI' '-depth' '8' '-rotate' '-0' '/tmp/transform_209313a25299.png'" Error code: 124](/index.php/File:Stepgraph_1.png "File:Stepgraph 1.png")

This is the encounter table representation for a path through the first reactor up to Guard Scorpion. The red points represent danger at each stepid throughout the journey from the entrance of the reactor to the base, and the yellow horizontal lines represent which of the six field maps each stepid is taken on in this idealized scenario. The different slopes this line of red points takes through each of these different fields represents how each of these fields has a different rate of danger increase. Higher-danger fields have a steeper line since they result in more danger gained per encounter check.

This example is with running for the entire distance from the train to guard scorpion, and in total this route gets three encounters: on the 3rd, 4th, and 6th fields. Each of these encounters resets the current danger value at that point.

### Manipulating the encounter table

Things change and become manipulatable when walking is introduced. If an encounter check happens while Run is not being held, danger increases by only ¼ as much as it would if run were held. For real-time runs, this effectively means that walking increases danger less per battle check than running, meaning that walking will result in a more shallow rate of danger increase than running.

This is the encounter table representation of KartSeven's steproute's Reactor 1 up to Guard Scorpion. Notice that it only gets a single encounter this time because it uses walking in two places to reduce the rate at which danger increases, once on the 1st field and once on the 4th field. Just enough walking is done so, when the battle checks that had previously resulted in battles occur, the current danger value is low enough to fail to meet the required danger for this encounter threshold to trigger, and so no encounter occurs.

[Error creating thumbnail: Unable to save thumbnail to destination](/index.php/File:Stepgraph_2.png "File:Stepgraph 2.png")

Also note that the path taken by this encounter table is significantly longer than the path in the first encounter table. Specifically, since there is walking on the 1st and 4th fields, more of the encounter table is expended on these fields than in the example without walking. Because walking is slower than running but encounter checks still happen at the same rate when either walking or running, four times as many encounter checks are expended per unit of distance while walking compared to running. Therefore, to cover the same distance, more encounter checks must be spent, which means expending more of the encounter table.

In this way, the encounter table can be thought of as a resource that can be expended, along with time, to reduce your danger value and potentially avoid encounters by walking. Whether it is beneficial to expend or to not expend parts of the encounter table through walking depends on various factors including the layout of the current and future parts of the encounter table, your current encounter formation state, and many other factors.

## Which encounters do you get? (Formation)

Each field that can be hostile has up to two encounter tables, one of which can be set as the current active encounter table. The encounter table consists of two sections: Standard encounters and Special encounters. Special encounters are back, side, and pincer attacks, whereas standard encounters are anything else (including preempts, which are determined differently).

The game first generates a random number and uses it to determine if this encounter will be a Special formation and, if so, which of them it is. If a special formation has been selected, the procedure quits and that is the battle generated.

If a special battle was not selected, another random number is chosen, and this value is used to determine which of the Standard encounters will be generated. If this generated encounter is the same as the Last Encounter Formation (more on this later), the game "rerolls" your encounter by generating another random number and using this as the generated encounter formation (even if it is the same as the last encounter formation again).

After any standard encounter is selected (with one exception, see Quirks), the Last Encounter Formation is set to the newly generated encounter formation. This Last Encounter Formation mechanic is meant to reduce how often the same encounter appears twice in a row, and it makes a generated formation sometimes dependent on the formation generated by the previous encounter.

Encounter tables are structured as follows: There are six slots for Standard encounters and four slots for Special encounters. Each encounter slot consists of a formation ID and an encounter rate. Formation IDs are a number from 0 to 999, and each represents a (usually) unique battle formation, a lookup table for which can be found here.

Encounter rates are a value from 0 to 63, and the higher the value, the more likely that encounter is to appear. Each Special slot is assigned a specific function, with the first two being assigned to Back Attack formations, the third for a Side Attack formation, and the fourth for a Pincer formation. The sum of all weights for Standard encounters always equals 64, and the sum of all weights for Special encounters is always significantly less than 64.

To select a formation, a random number from 0 to 63 is chosen. We then iterate through all of the encounter slots in either the special or standard encounter table area, whichever we are currently looking through. Throughout this, we keep track of a threshold value that starts at zero.

For each of these encounter slots, we start by adding this slot’s encounter rate to the threshold value. If the random value is less than this threshold value, then this is the encounter we use.

In effect, this means that we can imagine each encounter table as a list of 64 slots, each with an encounter formation. The randomly chosen number will be the index of one of these slots, and that will be the encounter chosen by this procedure.

For example, let’s take the first screen in the game, whose standard encounter table is as follows:

Encounter Formation  | Encounter Rate
---|---
301  | 24
302  | 22
303  | 18

In order, the game checks the first, then second, then third of these slots until it finds a formation that satisfies the random value. This can be represented by our 64-slot model having the first 24 slots (from 0 to 23) represent encounter formation 301, the next 22 slots (from 24 to 45) represent encounter formation 302, and the last 18 slots (from 46 to 63) representing encounter formation 303.

0 …………………………... 23  | 24 ……………………….… 45  | 46 ……………………….… 63
---|---|---
301  | 302  | 303

As for when a special formation is attempting to be generated, this table consists of some slots filled at the beginning of the table, followed by the rest of the table containing no encounter. If a section with no encounter is chosen, then the game has failed to generate a special encounter and will then go to generate a standard encounter instead.

Note that having sufficiently many and high-leveled Preemptive Materia equipped on the third party member (this is a bug) causes you to enter a Mastered Preempt state. In this state, the encounter rate for the Back Attack and Pincer slots in the special encounter slots are halved. The side attack slot is unaffected. This is the only difference between the different special encounter slots, as the data for whether each formation is a back, side, or pincer attack are stored in the formation data elsewhere. (TODO: find exact thresholds and conditions for Mastered Preempt state)

### Manipulating Formations

As you might have been able to guess, a similar method is used to generate the random numbers used for formations as the method used for danger thresholds, however it’s even simpler this time. A different random index is used for formation generation, making it independent from danger threshold generation. The index that determines what encounters you get is a 1-byte value called the Formation Accumulator, or just formation for short. When this generator calls for a random number, formation is incremented by one, then the RNG table value at the index of the new formation value is returned and is used as the random value.

This generator returns a value between 0 and 255, so to get a value between 0 and 63, it is divided by 4 and truncated to an integer.

Formation RNG is called once to determine if this encounter is a special formation. If it isn’t, it’s called a second time to determine what this encounter’s formation is. If a reroll is needed, it’s called a third time. As a result, the formation value can advance by 1, 2, or 3 depending on the current state of the formation value and the last encounter formation, which is what determines when a reroll occurs.

Formations can be manipulated by skipping or not skipping certain encounters to cause formation to increase or not increase at specific points. It can also be affected by getting encounters on certain fields, depending on if they have a special encounter table or not, or if their encounter table results in a reroll. This reroll is also known as a “bump” or “getting bumped”, since you’re bumped past an encounter to the next, or bumped past the usual formation increase of 2 and up to 3.

For example, you can intentionally waste movement and force an encounter at the end of a field instead of going to the next field, even if it was possible to make it to the next field, just because getting an encounter on the first field would result in a different and possibly more favorable enemy formation or formation increase than getting it on the second field. You can also intentionally avoid walking under an encounter that could be skipped because getting that encounter at that point would result in a more favorable formation increase.

In general, formation manipulation is needed to ensure getting specific encounters, such as those that give Enemy Skills such as Death Claws for Laser or a Boundfat for Death Sentence, or to avoid getting particularly bad encounters, such as pincers. However, formations are in general more difficult to manipulate since you don’t have much flexibility or control over their increase, but just knowing what formations you will get is often enough to prepare for a particularly bad encounter formation if one were to occur. Formation manipulation and routing is especially important in 100% because the category requires many drops, steals, morphs, enemy skills, and even kills from specific enemies all over the game.

## How it actually works

A procedure is run on every frame on which Cloud moves on any field map. The procedure is as follows:

    Add one step fraction.
    If the current fraction is now zero, and we are on a hostile field, and encounters are enabled:
        Run Value = Field Scale * (4 if Run is held, 1 if it is not)
        Danger = Danger + (2 * Run Value / Encounter Rate)

        x = increment_step_id()
        If x < Preemptive Rate
            Preempt Flag = True
        Else:
            Preempt Flag = False

        x = increment_step_id()
        If x < ((Danger * Enemy Lure Value) / 4096):
            Set the game so we’re going to enter a battle
            x = increment_formation() / 4

            // Check Special Formations

            // Back Attack slot 1

            If Mastered Preempt Flag is not set:
                threshold = back attack slot 1 encounter rate
            Else:
                threshold = back attack slot 1 encounter rate / 2
            If x < threshold:
                Set battle formation to back attack slot 1
                Preempt Flag = False
                Exit this procedure

            // Back Attack slot 2

            If Mastered Preempt Flag is not set:
                threshold = threshold + back attack slot 2 encounter rate
            Else:
                threshold = threshold + back attack slot 2 encounter rate / 2
            If x < threshold:
                Set battle formation to back attack slot 2
                Preempt Flag = False
                Exit this procedure

            // Side Attack slot

            threshold = threshold + side attack slot encounter rate
            If x < threshold:
                Set battle formation to side attack slot
                Exit this procedure

            // Pincer slot

            If Mastered Preempt Flag is not set:
                threshold = threshold + pincer slot encounter rate
            Else:
                threshold = threshold + pincer slot encounter rate / 2
            If x < threshold:
                Set battle formation to pincer slot
                Exit this procedure

            // Check Standard Formations

            threshold = 0
            x = increment_formation()

            Current Encounter Slot = standard slot 6
            For i from 1 to 5:
                threshold = threshold + standard slot i encounter rate
                If x / 4 < threshold:
                    Current Encounter Slot = standard slot i
                    Break out of the loop

            Set battle formation to Current Encounter Slot encounter formation

            // Reroll (“Bump”) check

            If Current Encounter Slot encounter formation == Last Encounter Formation:
                threshold = 0
                x = increment_formation()
                Set battle formation to standard slot 6
                For i from 1 to 5:
                    threshold = threshold + standard slot i encounter rate
                    If x / 4 < threshold:
                        Set battle formation to standard slot i
                        Last Encounter Formation = std. slot i encounter formation
                        Exit this procedure

            Else:
                Last Encounter Formation = Current Encounter Slot encounter formation

Procedure increment_step_id:

    stepid = stepid + 1

    If stepid = 0:
        offset = offset + 13

    Return RNG_TABLE[stepid] - offset;

Procedure increment_formation:

    formation = formation + 1
    Return RNG_TABLE[formation]

**Notes:**

  * All divisions round down to an integer, as everything here is done with truncating integer arithmetic.
  * One Step Fraction is actually a unit of 32, so step fractions actually go 32, 64, 96… 224, 0.
  * Field Scale is a value assigned to each field. Usually it's 512, but it can be larger or smaller. See [[1]](https://pastebin.com/UrRQ7ezH) for a list of field scale values.
  * Run Value isn’t a value calculated at this part of the encounter logic, it’s actually calculated every frame based on whether Run is held or not and is constantly updated even while not moving.
  * Encounter Rate is a 1-byte value in the current encounter table. Counterintuitively, lower Encounter Rate values mean a higher danger increase per step, and higher Encounter Rate values mean a lower danger increase per step.
  * Because of the complex interaction between Run Value and Encounter Rate, It’s more useful to think of each screen as having a DIPS value, or Danger Increase Per Step, which is the final value that gets added to Danger per step of running.
  * For some reason, Preempt Flag actually gets set to a value of 4 to signal True and 0 for False.
  * In reality, the Preempt Value and Mastered Preempt Flag are combined into a single value, where the most significant bit is the Mastered Preempt Flag and the seven least significant bits are the Preempt Value.
  * The Mastered Preemptive Flag is weird, see below.

## Field Map RNG table

This is the RNG table used for field map RNG and all forms of field map encounter RNG. It consists of a shuffled list of all bytes from 0 to 255, all of which are represented below in hex.

    B1 CA EE 6C 5A 71 2E 55 D6 00 CC 99 90 6B 7D EB
    4F A0 07 AC DF 8A 56 9E F1 9A 63 75 11 91 A3 B8
    94 73 F7 54 D9 6E 72 C0 F4 80 DE B9 BB 8D 66 26
    D0 36 E1 E9 70 DC CD 2F 4A 67 5D D2 60 B5 9D 7F
    45 37 50 44 78 04 19 2C EF FD 64 81 03 DA 95 4C
    7A 0B AD 1F BA DD 3E F9 D7 1A 29 F8 18 B3 20 F6
    D1 5E 34 92 7B 24 43 88 97 D4 0F 35 AA 83 68 27
    A8 D5 BE FA 14 31 AF 10 0D D8 6A CE 23 61 F3 3D
    A4 08 33 E3 A9 38 E6 93 1D 1C F0 0E 87 59 65 82
    BC FF FE 7E 8F C1 1E F5 CB 49 02 32 09 C4 8E C6
    2B 40 A7 17 76 3B 16 2A C8 FB B2 58 A5 15 AE 25
    CF 46 C7 48 B4 0A 3F C9 06 85 51 89 62 4D 12 8C
    EA A2 98 4B 79 6F 5C 47 30 1B E7 C5 22 9C E8 96
    3A E4 7C E0 69 A1 B7 05 39 74 01 9F BD C3 84 FC
    77 86 13 4E BF F2 53 5B ED 21 8B 6D C2 41 B6 DB
    3C D3 28 EC 2D E2 9B A6 42 52 57 5F E5 AB B0 0C

## Quirks and other misc. stuff

### Mastered Preemptive Glitch

When equipping a Mastered Preemptive Materia, the Mastered Preemptive Flag is always set, no matter what character it is equipped on.

On any of the following conditions, the Mastered Preemptive Flag is recalculated based only on whether the third character has a Mastered Preemptive Materia equipped, ignoring any equipped on either the first or second characters:

  * A battle starts (If this is a random encounter, the Mastered Preemptive Flag does affect this battle's generation before it is recalculated)
  * The menu is closed and opened again
  * The Materia section of the menu is exited and entered again

On any of the following conditions, the Mastered Preemptive Flag is recalculated based only on whether the _current_ character has a Mastered Preemptive Materia equipped, ignoring any equipped on any of the other characters:

  * Any Materia is equipped or unequipped
  * A piece of equipment (including an accessory) is equipped or unequipped

In short, the Mastered Preemptive Flag is the least broken when Preemptive Materia are equipped on the third character, but they can still be temporarily broken.

### Menu Skipping

On field maps, it is possible to interrupt the transition from the field to a random encounter, effectively skipping that encounter, by opening the menu on the same frame that the encounter would happen on.

There are two caveats:

  1. Your danger value gets reset after a random encounter finishes, so that danger reset never happens on a skipped encounter.
  2. Skipping an encounter and then entering a scripted battle before the next field map encounter will result in the Duplicating Boss Glitch, where that scripted encounter plays again immediately after it finishes, which also sometimes results in the game crashing after that repeated boss.

A similar mechanic is used to perform [Stinger Skip](/index.php?title=Stinger_Skip&action=edit&redlink=1 "Stinger Skip \(page does not exist\)") where a random encounter is triggered on the same frame that you enter the Stingers' webs in the Cave of the Gi, resulting in both being skipped.

### Offset Math

You can find the index of any offset value (the number of times you have to overflow stepid and increment offset to get to this offset value) with the following formula:

    (offset * 197) mod 256

so offset 0 has index 0, offset 13 has index 1, offset 177 has index 53, and offset 243 has index 255.

This works because the modular multiplicative inverse of 13 (modulo 256) is 197. Read more about it on wikipedia if you want because math is pretty cool <https://en.wikipedia.org/wiki/Modular_multiplicative_inverse>

### 32/64 glitch

If an encounter formation has an encounter rate of 32 or greater, it acts as if it has an encounter formation of 64/64. This is due to using signed 16-bit values to store world map encounter data, as opposed to unsigned values.

This affects five encounter tables in the game: [mtnvl3](/index.php?title=Mtnvl3&action=edit&redlink=1 "Mtnvl3 \(page does not exist\)") encounter table 2, [junin2](/index.php?title=Junin2&action=edit&redlink=1 "Junin2 \(page does not exist\)") encounter table 2, [junsbd1](/index.php?title=Junsbd1&action=edit&redlink=1 "Junsbd1 \(page does not exist\)"), [mtcrl_4](/index.php?title=Mtcrl_4&action=edit&redlink=1 "Mtcrl 4 \(page does not exist\)"), and [mtcrl_9](/index.php?title=Mtcrl_9&action=edit&redlink=1 "Mtcrl 9 \(page does not exist\)").

<https://pastebin.com/yJ6N0H9d>

### Reroll 6 / Bump 6 glitch

When determining a field map encounter formation, if you get bumped and then the game rolls the encounter formation in slot 6 on the reroll, your last encounter formation is NOT updated. You can trace a path through the codepath above and see that this is the case. This should almost never actually affect anything.

Load video

YouTube

YouTube might collect personal data. [Privacy Policy](https://www.youtube.com/howyoutubeworks/user-settings/privacy/)

ContinueDismiss