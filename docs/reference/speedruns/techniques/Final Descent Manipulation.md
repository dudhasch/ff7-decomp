Final Descent Maniuplation is the process of manipulating [Field Map RNG](/index.php/Field_Map_RNG "Field Map RNG") to reduce the number of encounters that are obtained during the [Final Descent](/index.php/Final_Descent "Final Descent").

A list of all possible Final Descents can be found [here](/index.php/List_of_Final_Descents "List of Final Descents").

## Proactive Manipulation

The FInal Descent can be manipulated proactively by manipulating Field RNG such that it is in a specific favorable state when the player reaches the FInal Descent. Since the Field RNG state is split between the Stone and List values, these must be manipulated separately.

### Stone Manipulation

Manipulating Stone is significantly easier than manipulating List, as Stone is set at only three points throughout the game: Squats, the Shinra Elevator, and digging at Bone Village.

#### via Bone Village

When the game fades out and loads the digging version of Bone Village, Stone is set based on the current [In-Game Time](/index.php?title=In-Game_Time&action=edit&redlink=1 "In-Game Time \(page does not exist\)"). Using this, Stone can be set relatively easily and relatively quickly to any value without resetting the game.

More specifically, Stone is set to `16 * (s mod 16) + 1` where `s` is the current IGT seconds value (out of the current minute, from 0-59).

Stone by IGT Second on fadeout  1  | 17  | 33  | 49  | 65  | 81  | 97  | 113  | 127  | 143  | 161  | 177  | 193  | 209  | 225  | 241
---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---
0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  | 10  | 11  | 12  | 13  | 14  | 15
16  | 17  | 18  | 19  | 20  | 21  | 22  | 23  | 24  | 25  | 26  | 27  | 28  | 29  | 30  | 31
32  | 33  | 34  | 35  | 36  | 37  | 38  | 39  | 40  | 41  | 42  | 43  | 44  | 45  | 46  | 47
48  | 49  | 50  | 51  | 52  | 53  | 54  | 55  | 56  | 57  | 58  | 59  |

This method is used to set Stone to 49 in [Any%](/index.php/Any%25 "Any%") and [No Slots](/index.php/Any%25_No_Slots "Any% No Slots"), and 177 in [No Major Skips](/index.php/No_Major_Skips "No Major Skips").

#### via Hard Reset

Hard Resetting the game (with the Power Button on PSX or by closing and re-opening the game on PC or HD) will set the Stone value to 1. This is good since you can consistently set the game to a known stone value (and List is also set), however hard resetting takes significant time (especially on PSX). Stone 1 is also mediocre, as despite it having a low maximum number of encounters at 6, it cannot produce a 0-encounter descent.

### List Manipulation

#### via Cosmo Canyon

A glitch in the field map script in [cos_top](/index.php?title=Cos_top&action=edit&redlink=1 "Cos top \(page does not exist\)") can be used to set Field RNG to a specific set of values. This is primarily used in Any% and No Slots.

The following is a representation of the field map script for the object COOK's Main script:

    Wait 30 Frames
    Play Animation
    Label 1:

    Set random value to Var[5][10] (8-bit)

    If Var[5][10] == 0:
        Wait 160 Frames
        Play Animation
    Label 2:

    If Var[5][10] == 1:
        Wait 30 Frames
        Play Animation
    Label 3:

    If Var[5][10] == 2:
        Wait 300 Frames
        Play Animation
        Wait for animation to finish
        Play Animation
    Label 4:

    If Var[5][10] == 3:
        Wait 90 Frames
        Play Animation
    Label 5

    Goto Label 1

The problem in this script is that the function "Set random value to Var[5][10] (8-bit)" sets Var[5][10] to a random value from 0 to 255, not from 0 to 3 as is expected by the rest of the script. Therefore, 252/256 of the time that the main loop is ran, none of the conditions are met and it simply loops back to the top. This results in the game quickly jumping to the List values that result in the random numbers 0, 1, 2, or 3 being generated.

#### via Raid on Midgar

#### via Hard Reset

### Routes

#### Stone 49 (PSX Any%, No Slots)

## Reactive Manipulation

The older versions of Final Descent Manipulation reacted to the current RNG state and adjusted it to result in a more favorable Final Descent.

### Via Hojo Sparks

The main method used for this was to learn the current RNG state from the pattern of sparks emitted during the cutscene before the Hojo boss fight.