On the world map, triangles will have different ground types that determine which encounter table their encounters pull from and whether the player can occupy them depending on their current state and current mount/vehicle.

## Table

This table was stolen from <https://wiki.ffrtt.ru/index.php/FF7/WorldMap_Module>

ID  | Name  | Description
---|---|---
_0_ | _Grass_ | _Most things can go here._
_1_ | _Forest_ | _No landing here, but anything else goes._
_2_ | _Mountain_ | _Chocobos and flying machines only._
_3_ | _Sea_ | _Deep water, only gold chocobo and submarine can go here._
_4_ | _River Crossing_ | _Buggy, tiny bronco and water-capable chocobos._
_5_ | _River_ | _Tiny bronco and chocobos._
_6_ | _Water_ | _Shallow water, same as above._
_7_ | _Swamp_ | _Midgar zolom can only move in swamp areas._
_8_ | _Desert_ | _No landing._
_9_ | _Wasteland_ | _Found around Midgar, Wutai and misc other. No landing._
_10_ | _Snow_ | _Leaves footprints, no landing._
_11_ | _Riverside_ | _Beach-like area where river and land meet._
_12_ | _Cliff_ | _Sharp drop, usually where the player can be on either side._
_13_ | _Corel Bridge_ | _Tiny bridge over the waterfall from Costa del Sol to Corel._
_14_ | _Wutai Bridge_ | _Rickety rope bridges south of Wutai._
_15_ | _Unused_ | _Doesn't seem to be used anywhere in the original data._
_16_ | _Hill side_ | _This is the tiny walkable part at the foot of a mountain._
_17_ | _Beach_ | _Where land and shallow water meets._
_18_ | _Sub Pen_ | _Only place where you can enter/exit the submarine._
_19_ | _Canyon_ | _The ground in cosmo canyon has this type, walkability seems to be the same as wasteland._
_20_ | _Mountain Pass_ | _The small path through the mountains connecting Costa del Sol and Corel._
_21_ | _Unknown_ | _Present around bridges, may have some special meaning._
_22_ | _Waterfall_ | _River type where the tiny bronco can't go._
_23_ | _Unused_ | _Doesn't seem to be used anywhere in the original data._
_24_ | _Gold Saucer Desert_ | _Special desert type for the golden saucer._
_25_ | _Jungle_ | _Walkability same as forest, used in southern parts of the map._
_26_ | _Sea (2)_ | _Special type of deep water, only used in one small spot next to HP-MP cave, possibly related to the underwater map/submarine._
_27_ | _Northern Cave_ | _Inside part of the crater, where you can land the highwind._
_28_ | _Gold Saucer Desert Border_ | _Narrow strip of land surrounding the golden saucer desert. Probably related to the "quicksand" script._
_29_ | _Bridgehead_ | _Small area at both ends of every bridge. May have some special meaning._
_30_ | _Back Entrance_ | _Special type that can be set unwalkable from the script._
_31_ | _Unused_ | _Doesn't seem to be used anywhere in the original data._

## Walkability

This section details the walkability conditions of each form the player can take. Essentially, a triangle being walkable means that it's legal ground that the player can occupy normally. The player will be pushed away from moving onto unwalkable triangles, and the player will be stuck if they find themselves in an unwalkable triangle.

The function that computes walkability for the player is at address 74CECA on the PC version.

"Exiting" means that the player is about to exit the vehicle, and a check is being performed to ensure the ground they will exit onto is valid. These checks are performed by the vehicle being exited instead of by the new player entity.

"On Bridge" means that the player is on currently standing on a Corel Bridge or Wutai Bridge triangle. This usually restricts the ground they are allowed to move onto to only be bridges or bridgeheads, which transition the player off the bridge. This is why the player doesn't simply walk off the side of the bridge onto the ground below if they were to try to walk off a bridge.

### Human (Cloud / Tifa / Cid)

ID  | Type  | Normal  | On Bridge
---|---|---|---
_0_ | _Grass_ | ✓  |
_1_ | _Forest_ | ✓  |
_2_ | _Mountain_ |  |
_3_ | _Sea_ |  |
_4_ | _River Crossing_ |  |
_5_ | _River_ |  |
_6_ | _Water_ |  |
_7_ | _Swamp_ | ✓  |
_8_ | _Desert_ | ✓  |
_9_ | _Wasteland_ | ✓  |
_10_ | _Snow_ | ✓  |
_11_ | _Riverside_ | ✓  |
_12_ | _Cliff_ |  |
_13_ | _Corel Bridge_ | ✓  | ✓
_14_ | _Wutai Bridge_ | ✓  | ✓
_15_ | _Unused_ |  |
_16_ | _Hill side_ | ✓  |
_17_ | _Beach_ | ✓  |
_18_ | _Sub Pen_ |  |
_19_ | _Canyon_ | ✓  |
_20_ | _Mountain Pass_ | ✓  |
_21_ | _Unknown_ |  |
_22_ | _Waterfall_ |  |
_23_ | _Unused_ |  |
_24_ | _Gold Saucer Desert_ |  |
_25_ | _Jungle_ | ✓  |
_26_ | _Sea (2)_ |  |
_27_ | _Northern Cave_ |  |
_28_ | _Gold Saucer Desert Border_ | ✓  |
_29_ | _Bridgehead_ | ✓  | ✓
_30_ | _Back Entrance_ | ✓  |
_31_ | _Unused_ |  |

### Highwind

The Highwind does not check the current ground type if it is not landing and the player is not exiting it.

The player cannot exit the Highwind onto a Script-7 triangle.

ID  | Type  | Exiting  | Landing
---|---|---|---
_0_ | _Grass_ | ✓  | ✓
_1_ | _Forest_ | ✓  |
_2_ | _Mountain_ |  |
_3_ | _Sea_ |  |
_4_ | _River Crossing_ |  |
_5_ | _River_ |  |
_6_ | _Water_ |  |
_7_ | _Swamp_ | ✓  |
_8_ | _Desert_ | ✓  |
_9_ | _Wasteland_ | ✓  |
_10_ | _Snow_ | ✓  |
_11_ | _Riverside_ | ✓  |
_12_ | _Cliff_ |  |
_13_ | _Corel Bridge_ | ✓  |
_14_ | _Wutai Bridge_ | ✓  |
_15_ | _Unused_ |  |
_16_ | _Hill side_ | ✓  |
_17_ | _Beach_ | ✓  |
_18_ | _Sub Pen_ |  |
_19_ | _Canyon_ | ✓  |
_20_ | _Mountain Pass_ | ✓  |
_21_ | _Unknown_ |  |
_22_ | _Waterfall_ |  |
_23_ | _Unused_ |  |
_24_ | _Gold Saucer Desert_ |  |
_25_ | _Jungle_ | ✓  |
_26_ | _Sea (2)_ |  |
_27_ | _Northern Cave_ |  |
_28_ | _Gold Saucer Desert Border_ |  |
_29_ | _Bridgehead_ |  |
_30_ | _Back Entrance_ |  |
_31_ | _Unused_ |  |

### Wild Chocobo

Wild Chocobos are handled separately from tamed Chocobos in the function.

The player cannot dismount a Wild Chocobo onto a Script-7 triangle.

ID  | Type  | Normal / Exiting  | Normal / Exiting On Bridge
---|---|---|---
_0_ | _Grass_ | ✓  |
_1_ | _Forest_ | ✓  |
_2_ | _Mountain_ |  |
_3_ | _Sea_ |  |
_4_ | _River Crossing_ |  |
_5_ | _River_ |  |
_6_ | _Water_ |  |
_7_ | _Swamp_ | ✓  |
_8_ | _Desert_ | ✓  |
_9_ | _Wasteland_ | ✓  |
_10_ | _Snow_ | ✓  |
_11_ | _Riverside_ | ✓  |
_12_ | _Cliff_ |  |
_13_ | _Corel Bridge_ | ✓  | ✓
_14_ | _Wutai Bridge_ | ✓  | ✓
_15_ | _Unused_ |  |
_16_ | _Hill side_ | ✓  |
_17_ | _Beach_ | ✓  |
_18_ | _Sub Pen_ |  |
_19_ | _Canyon_ | ✓  |
_20_ | _Mountain Pass_ | ✓  |
_21_ | _Unknown_ |  |
_22_ | _Waterfall_ |  |
_23_ | _Unused_ |  |
_24_ | _Gold Saucer Desert_ |  |
_25_ | _Jungle_ | ✓  |
_26_ | _Sea (2)_ |  |
_27_ | _Northern Cave_ |  |
_28_ | _Gold Saucer Desert Border_ | ✓  |
_29_ | _Bridgehead_ | ✓  | ✓
_30_ | _Back Entrance_ |  |
_31_ | _Unused_ |  |

### Tiny Bronco

There is an apparently-unused check for if the "Highwind Mode" flag is set. It appears to allow the Tiny Bronco to traverse over all ground types and land on shallow water types in a manner like the Highwind.

ID  | Type  | Normal  | Exiting
---|---|---|---
_0_ | _Grass_ |  |
_1_ | _Forest_ |  |
_2_ | _Mountain_ |  |
_3_ | _Sea_ |  |
_4_ | _River Crossing_ | ✓  |
_5_ | _River_ | ✓  |
_6_ | _Water_ | ✓  |
_7_ | _Swamp_ |  |
_8_ | _Desert_ |  |
_9_ | _Wasteland_ |  |
_10_ | _Snow_ |  |
_11_ | _Riverside_ |  | ✓
_12_ | _Cliff_ |  |
_13_ | _Corel Bridge_ |  |
_14_ | _Wutai Bridge_ |  |
_15_ | _Unused_ |  |
_16_ | _Hill side_ |  |
_17_ | _Beach_ |  | ✓
_18_ | _Sub Pen_ |  |
_19_ | _Canyon_ |  |
_20_ | _Mountain Pass_ |  |
_21_ | _Unknown_ |  |
_22_ | _Waterfall_ |  |
_23_ | _Unused_ |  |
_24_ | _Gold Saucer Desert_ |  |
_25_ | _Jungle_ |  |
_26_ | _Sea (2)_ |  |
_27_ | _Northern Cave_ |  |
_28_ | _Gold Saucer Desert Border_ |  |
_29_ | _Bridgehead_ |  |
_30_ | _Back Entrance_ |  |
_31_ | _Unused_ |  |

### Buggy

The player cannot exit the Buggy onto a Script-7 triangle.

ID  | Type  | Normal  | Exiting
---|---|---|---
_0_ | _Grass_ | ✓  | ✓
_1_ | _Forest_ | ✓  | ✓
_2_ | _Mountain_ |  |
_3_ | _Sea_ |  |
_4_ | _River Crossing_ | ✓  |
_5_ | _River_ |  |
_6_ | _Water_ |  |
_7_ | _Swamp_ |  | ✓
_8_ | _Desert_ | ✓  | ✓
_9_ | _Wasteland_ | ✓  | ✓
_10_ | _Snow_ | ✓  | ✓
_11_ | _Riverside_ | ✓  | ✓
_12_ | _Cliff_ |  |
_13_ | _Corel Bridge_ | ✓  | ✓
_14_ | _Wutai Bridge_ | ✓  | ✓
_15_ | _Unused_ |  |
_16_ | _Hill side_ | ✓  | ✓
_17_ | _Beach_ | ✓  | ✓
_18_ | _Sub Pen_ |  |
_19_ | _Canyon_ | ✓  | ✓
_20_ | _Mountain Pass_ | ✓  | ✓
_21_ | _Unknown_ |  |
_22_ | _Waterfall_ |  |
_23_ | _Unused_ |  |
_24_ | _Gold Saucer Desert_ | ✓  |
_25_ | _Jungle_ | ✓  | ✓
_26_ | _Sea (2)_ |  |
_27_ | _Northern Cave_ |  |
_28_ | _Gold Saucer Desert Border_ | ✓  |
_29_ | _Bridgehead_ | ✓  |
_30_ | _Back Entrance_ |  |
_31_ | _Unused_ |  |

### Cargo Ship

The ship in the cutscene while going from Junon to Costa del Sol.

ID  | Type  | Normal
---|---|---
_0_ | _Grass_ |
_1_ | _Forest_ |
_2_ | _Mountain_ |
_3_ | _Sea_ | ✓
_4_ | _River Crossing_ |
_5_ | _River_ |
_6_ | _Water_ |
_7_ | _Swamp_ |
_8_ | _Desert_ |
_9_ | _Wasteland_ |
_10_ | _Snow_ |
_11_ | _Riverside_ |
_12_ | _Cliff_ |
_13_ | _Corel Bridge_ |
_14_ | _Wutai Bridge_ |
_15_ | _Unused_ |
_16_ | _Hill side_ |
_17_ | _Beach_ |
_18_ | _Sub Pen_ | ✓
_19_ | _Canyon_ |
_20_ | _Mountain Pass_ |
_21_ | _Unknown_ |
_22_ | _Waterfall_ |
_23_ | _Unused_ |
_24_ | _Gold Saucer Desert_ |
_25_ | _Jungle_ |
_26_ | _Sea (2)_ | ✓
_27_ | _Northern Cave_ |
_28_ | _Gold Saucer Desert Border_ |
_29_ | _Bridgehead_ |
_30_ | _Back Entrance_ |
_31_ | _Unused_ |

### Submarine

The player cannot exit the Submarine onto a Script-7 triangle.

ID  | Type  | Normal  | Exiting
---|---|---|---
_0_ | _Grass_ |  | ✓
_1_ | _Forest_ |  | ✓
_2_ | _Mountain_ |  |
_3_ | _Sea_ | ✓  |
_4_ | _River Crossing_ |  |
_5_ | _River_ |  |
_6_ | _Water_ |  |
_7_ | _Swamp_ |  | ✓
_8_ | _Desert_ |  | ✓
_9_ | _Wasteland_ |  | ✓
_10_ | _Snow_ |  | ✓
_11_ | _Riverside_ |  | ✓
_12_ | _Cliff_ |  |
_13_ | _Corel Bridge_ |  | ✓
_14_ | _Wutai Bridge_ |  | ✓
_15_ | _Unused_ | ✓  |
_16_ | _Hill side_ |  | ✓
_17_ | _Beach_ |  | ✓
_18_ | _Sub Pen_ | ✓  |
_19_ | _Canyon_ |  | ✓
_20_ | _Mountain Pass_ |  | ✓
_21_ | _Unknown_ |  |
_22_ | _Waterfall_ |  |
_23_ | _Unused_ |  |
_24_ | _Gold Saucer Desert_ |  |
_25_ | _Jungle_ |  | ✓
_26_ | _Sea (2)_ | ✓  |
_27_ | _Northern Cave_ |  |
_28_ | _Gold Saucer Desert Border_ |  |
_29_ | _Bridgehead_ |  |
_30_ | _Back Entrance_ |  |
_31_ | _Unused_ |  |

### Chocobo

The player cannot dismount a Chocobo onto a Script-7 triangle.

ID  | Type  | Yellow  | Green  | Blue  | Black  | Gold  | On Bridge  | Dismounting  | Dismounting on Bridge
---|---|---|---|---|---|---|---|---|---
_0_ | _Grass_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_1_ | _Forest_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_2_ | _Mountain_ |  | ✓  |  | ✓  | ✓  |  |  |
_3_ | _Sea_ |  |  |  |  | ✓  |  |  |
_4_ | _River Crossing_ |  |  | ✓  | ✓  | ✓  |  |  |
_5_ | _River_ |  |  | ✓  | ✓  | ✓  |  |  |
_6_ | _Water_ |  |  | ✓  | ✓  | ✓  |  |  |
_7_ | _Swamp_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_8_ | _Desert_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_9_ | _Wasteland_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_10_ | _Snow_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_11_ | _Riverside_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_12_ | _Cliff_ |  |  |  | ✓  | ✓  |  |  |
_13_ | _Corel Bridge_ | ✓  | ✓  | ✓  | ✓  | ✓  | ✓  | ✓  | ✓
_14_ | _Wutai Bridge_ | ✓  | ✓  | ✓  | ✓  | ✓  | ✓  | ✓  | ✓
_15_ | _Unused_ |  |  |  |  |  |  |  |
_16_ | _Hill side_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_17_ | _Beach_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_18_ | _Sub Pen_ |  |  |  |  |  |  |  |
_19_ | _Canyon_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_20_ | _Mountain Pass_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_21_ | _Unknown_ |  |  |  |  |  |  |  |
_22_ | _Waterfall_ |  |  |  | ✓  | ✓  |  |  |
_23_ | _Unused_ |  |  |  |  |  |  |  |
_24_ | _Gold Saucer Desert_ |  |  |  |  | ✓  |  |  |
_25_ | _Jungle_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_26_ | _Sea (2)_ |  |  |  |  | ✓  |  |  |
_27_ | _Northern Cave_ |  |  |  |  |  |  |  |
_28_ | _Gold Saucer Desert Border_ | ✓  | ✓  | ✓  | ✓  | ✓  |  | ✓  |
_29_ | _Bridgehead_ | ✓  | ✓  | ✓  | ✓  | ✓  | ✓  | ✓  | ✓
_30_ | _Back Entrance_ |  |  |  |  |  |  |  |
_31_ | _Unused_ |  |  |  |  |  |  |  |

### Zolom

The Midgar Zolom also uses this function using a special case.

ID  | Type  | Normal
---|---|---
_0_ | _Grass_ |
_1_ | _Forest_ |
_2_ | _Mountain_ |
_3_ | _Sea_ |
_4_ | _River Crossing_ |
_5_ | _River_ |
_6_ | _Water_ |
_7_ | _Swamp_ | ✓
_8_ | _Desert_ |
_9_ | _Wasteland_ |
_10_ | _Snow_ |
_11_ | _Riverside_ |
_12_ | _Cliff_ |
_13_ | _Corel Bridge_ |
_14_ | _Wutai Bridge_ |
_15_ | _Unused_ |
_16_ | _Hill side_ |
_17_ | _Beach_ |
_18_ | _Sub Pen_ |
_19_ | _Canyon_ |
_20_ | _Mountain Pass_ |
_21_ | _Unknown_ |
_22_ | _Waterfall_ |
_23_ | _Unused_ |
_24_ | _Gold Saucer Desert_ |
_25_ | _Jungle_ |
_26_ | _Sea (2)_ |
_27_ | _Northern Cave_ |
_28_ | _Gold Saucer Desert Border_ |
_29_ | _Bridgehead_ |
_30_ | _Back Entrance_ |
_31_ | _Unused_ |