# Abilities

Every castable ability in Final Fantasy VII that comes from a Materia: magic spells, summon abilities and enemy skills. One file each, with MP cost, spell power, full effect and the in-game description text.

| Group | Count | Granted by |
| --- | ---: | --- |
| [Magic](magic/) | 54 | [Magic Materia](../materia/magic/README.md) |
| [Summon](summon/) | 16 | [Summon Materia](../materia/summon/README.md) |
| [Enemy Skill](enemy-skill/) | 24 | [Enemy Skill Materia](../materia/command/enemy-skill.md) |

## Damage and healing formulas

Restorative magic that heals a fixed amount:

```
HP restored = (SpellPower * 22) + ((Level + MagicAttack) * 6)
```

Attack magic that deals flat (non-fractional) damage:

```
damage = (SpellPower / 16) * ((Level + MagicAttack) * 6)
```

Spells with a fractional spell power such as `16/32` ignore these formulas and act on a proportion of the target's current or maximum HP instead.

## Magic spells by category

The battle menu groups magic into Restore, Attack and Indirect, and the player can reorder those three under Config -> Magic order. The fourth group always sorts last.

### Restore

Restorative magic - healing, revival and status recovery.

| Spell | MP | Power | Materia | Effect |
| --- | ---: | ---: | --- | --- |
| [Cure](magic/cure.md) | 5 | 5 | Restore(1) | Heal a small amount of HP. |
| [Cure2](magic/cure2.md) | 24 | 35 | Restore(2) | Heal a moderate amount of HP. |
| [Cure3](magic/cure3.md) | 64 | 130 | Restore(4) | Heal a large amount of HP. |
| [Regen](magic/regen.md) | 30 | 0 | Restore(3) | Inflicts Regen. |
| [Poisona](magic/poisona.md) | 3 | 0 | Heal(1) | Cures Poison. |
| [Esuna](magic/esuna.md) | 15 | 0 | Heal(2) | Cures Sleep, Poison, Sadness, Fury, Confusion, Silence, Frog, Small, Slow-numb, Petrify, Berserk, Paralyzed, and Darkness. |
| [Resist](magic/resist.md) | 120 | 0 | Heal(3) | Inflicts Resist status. |
| [Life](magic/life.md) | 34 | 8/32 | Revive(1) | Revive a party member with minimal health. |
| [Life2](magic/life2.md) | 100 | 32/32 | Revive(2) | Revive a party member with full health. |

### Attack

Damaging magic. Most lines are elemental and come in three escalating tiers.

| Spell | MP | Power | Materia | Effect |
| --- | ---: | ---: | --- | --- |
| [Fire](magic/fire.md) | 4 | 8 | Fire(1) | Low fire-elemental damage. |
| [Fire2](magic/fire2.md) | 22 | 20 | Fire(2) | Moderate fire-elemental damage. |
| [Fire3](magic/fire3.md) | 52 | 64 | Fire(3) | High fire-elemental damage. |
| [Ice](magic/ice.md) | 4 | 8 | Ice(1) | Low ice-elemental damage. |
| [Ice2](magic/ice2.md) | 22 | 20 | Ice(2) | Moderate ice-elemental damage. |
| [Ice3](magic/ice3.md) | 52 | 64 | Ice(3) | High ice-elemental damage. |
| [Bolt](magic/bolt.md) | 4 | 8 | Lightning(1) | Low lightning-elemental damage. |
| [Bolt2](magic/bolt2.md) | 22 | 20 | Lightning(2) | Moderate lightning-elemental damage. |
| [Bolt3](magic/bolt3.md) | 52 | 64 | Lightning(3) | High lightning-elemental damage. |
| [Quake](magic/quake.md) | 6 | 11 | Earth(1) | Low earth-elemental damage. |
| [Quake2](magic/quake2.md) | 28 | 24 | Earth(2) | Moderate earth-elemental damage. |
| [Quake3](magic/quake3.md) | 68 | 70 | Earth(3) | High earth-elemental damage. |
| [Bio](magic/bio.md) | 8 | 10 | Poison(1) | Low poison-elemental damage. 48% chance of inflicting Poison. |
| [Bio2](magic/bio2.md) | 36 | 21 | Poison(2) | Moderate poison-elemental damage. 48% chance of inflicting Poison. |
| [Bio3](magic/bio3.md) | 80 | 68 | Poison(3) | High poison-elemental damage. 72% chance of inflicting Poison. |
| [Demi](magic/demi.md) | 14 | 8/32 | Gravity(1) | Gravity-elemental damage equal to 25% of target's current HP. Cannot be reflected. |
| [Demi2](magic/demi2.md) | 33 | 16/32 | Gravity(2) | Gravity-elemental damage equal to 50% of target's current HP. Cannot be reflected. |
| [Demi3](magic/demi3.md) | 48 | 24/32 | Gravity(3) | Gravity-elemental damage equal to 75% of target's current HP. Cannot be reflected. |

### Indirect

Status and support magic - buffs, debuffs and battle manipulation.

| Spell | MP | Power | Materia | Effect |
| --- | ---: | ---: | --- | --- |
| [Sleepel](magic/sleepel.md) | 8 | 0 | Seal(1) | 72% chance of inflicting Sleep on target. |
| [Silence](magic/silence.md) | 24 | 0 | Seal(2) | 60% chance of inflicting Silence on target. |
| [Confu](magic/confu.md) | 18 | 0 | Mystify(1) | 60% chance of inflicting Confuse on target. |
| [Berserk](magic/berserk.md) | 18 | 0 | Mystify(2) | Inflicts Berserk on target. 80% probability on enemies, 100% on allies. |
| [Mini](magic/mini.md) | 10 | 0 | Transform(1) | Inflicts or cures Small on target. 72% probability on enemies, 100% on allies. |
| [Toad](magic/toad.md) | 14 | 0 | Transform(2) | Inflicts or cures Frog on target. 72% probability on enemies, 100% on allies. |
| [Haste](magic/haste.md) | 18 | 0 | Time(1) | Grants Haste to target. |
| [Slow](magic/slow.md) | 20 | 0 | Time(2) | Inflicts Slow on target. Only enemy immunity can prevent it. |
| [Stop](magic/stop.md) | 34 | 0 | Time(3) | 60% chance of inflicting Stop on target. |
| [Barrier](magic/barrier.md) | 16 | 0 | Barrier(1) | Grants Barrier. |
| [MBarrier](magic/mbarrier.md) | 24 | 0 | Barrier(2) | Grants MBarrier. |
| [Reflect](magic/reflect.md) | 30 | 0 | Barrier(3) | Grants Reflect. Cannot be reflected. |
| [Wall](magic/wall.md) | 58 | 0 | Barrier(4) | Grants Barrier and MBarrier. |
| [DeBarrier](magic/debarrier.md) | 12 | 0 | Destruct(1) | Remove Barrier, MBarrier, Reflect, and Shield statuses. Cannot be reflected. |
| [DeSpell](magic/despell.md) | 20 | 0 | Destruct(2) | Remove Haste, Slow, Stop, Regen, Barrier, MBarrier, Reflect, Shield, Death Force, and Resist statuses. Cannot be reflected. |
| [Death](magic/death.md) | 30 | 0 | Destruct(3) | 44% chance of inflicting Death. |
| [Escape](magic/escape.md) | 16 | 0 | Exit(1) | Instantly end battle without reward. Cannot be reflected. |
| [Remove](magic/remove.md) | 99 | 0 | Exit(2) | Remove an enemy from battle and flag as Dead. No Gil will be received from this enemy. Cannot be reflected. |

### Other

High-tier magic. These spells always sort to the bottom of the battle menu regardless of the player's Magic order setting.

| Spell | MP | Power | Materia | Effect |
| --- | ---: | ---: | --- | --- |
| [Comet](magic/comet.md) | 70 | 80 | Comet(1) | Major non-elemental damage. Cannot be reflected. |
| [Comet2](magic/comet2.md) | 110 | 30 | Comet(2) | Moderate non-elemental damage to random enemy target. Four attacks. Cannot be reflected. |
| [Freeze](magic/freeze.md) | 82 | 95 | Contain(1) | High ice-elemental damage. 68% chance of inflicting Stop. |
| [Break](magic/break.md) | 86 | 100 | Contain(2) | High earth-elemental damage. 32% chance of inflicting Petrify. |
| [Tornado](magic/tornado.md) | 90 | 105 | Contain(3) | High wind-elemental damage. 32% chance of inflicting Confusion. |
| [Flare](magic/flare.md) | 100 | 115 | Contain(4) | Heavy fire-elemental damage. |
| [FullCure](magic/fullcure.md) | 99 | 32/32 | Full Cure(2) | Fully restores target's HP. Cannot be reflected. |
| [Shield](magic/shield.md) | 180 | 0 | Shield(2) | Grants Shield status. Cannot be reflected. |
| [Ultima](magic/ultima.md) | 130 | 105 | Ultima(2) | Heavy non-elemental magic damage to all enemies. Cannot be reflected. |

## Summon abilities

| Summon | Ability | MP | Power | Effect |
| --- | --- | ---: | ---: | --- |
| Choco/Mog | [DeathBlow!!](summon/deathblow.md) | 14 | 16 | 15/16 of the time: non-elemental damage to all enemies with a 40% chance of inflicting Stop. |
| Shiva | [Diamond Dust](summon/diamond-dust.md) | 32 | 24 | Ice-elemental damage to all enemies. |
| Ifrit | [Hellfire](summon/hellfire.md) | 34 | 27 | Fire-elemental damage to all enemies. |
| Ramuh | [Judgement Bolt](summon/judgement-bolt.md) | 40 | 30 | Lightning-elemental damage to all enemies. |
| Titan | [Anger of the Land](summon/anger-of-the-land.md) | 46 | 33 | Earth-elemental damage to all enemies. |
| Odin | [Steel Bladed Sword](summon/steel-bladed-sword.md) | 80 | 0 | If the target is not immune to instant death: instantly kills all enemies, with 92% success. |
| Leviathan | [Tidal Wave](summon/tidal-wave.md) | 78 | 75 | Water-elemental damage to all enemies. |
| Bahamut | [Mega Flare](summon/mega-flare.md) | 100 | 65 | High non-elemental damage to all enemies. Ignores Magic Defense. |
| Kujata | [Tetra-Disaster](summon/tetra-disaster.md) | 110 | 100 | Fire-, Ice- and Lightning-elemental damage to all enemies. |
| Alexander | [Judgement](summon/judgement.md) | 120 | 120 | Holy-elemental damage to all enemies. |
| Phoenix | [Phoenix Flame](summon/phoenix-flame.md) | 180 | 60 | High Fire-elemental damage to all enemies, and resurrects dead allies. |
| Neo Bahamut | [Giga Flare](summon/giga-flare.md) | 140 | 80 | High non-elemental damage to all enemies. Ignores Magic Defense. |
| Hades | [Black Cauldron](summon/black-cauldron.md) | 150 | 90 | Non-elemental damage to all enemies, with a 100% chance of inflicting Poison, Confusion, Sleep, Silence, Small, Frog, Slow and Paralyzed on anything not immune. |
| Typhon | [Disintegration](summon/disintegration.md) | 160 | 110 | High Fire, Ice, Lightning and Earth damage to all enemies. Ignores Magic Defense. |
| Bahamut ZERO | [Tera Flare](summon/tera-flare.md) | 180 | 120 | Heavy non-elemental damage to all enemies. Ignores Magic Defense. |
| Knights of Round | [Ultimate End](summon/ultimate-end.md) | 250 | 80 | Heavy non-elemental damage to all enemies, ignoring Magic Defense. The damage is the cumulative sum of thirteen separate hits. |

## Enemy skills

| Skill | MP | Power | Type | Missable | Effect |
| --- | ---: | ---: | --- | --- | --- |
| [Frog Song](enemy-skill/frog-song.md) | 5 | 0 | Magical | No | Inflicts Sleep and Frog on one target. |
| [L4 Suicide](enemy-skill/l4-suicide.md) | 10 | 31/32 | Magical | No | On every member of the target party whose level is divisible by 4, removes 31/32 of current HP and inflicts Mini. |
| [Magic Hammer](enemy-skill/magic-hammer.md) | 3 | 0 | Magical | No | Drains 100 MP from one target and transfers it to the caster. |
| [White Wind](enemy-skill/white-wind.md) | 34 | 0 | Physical | No | Restores HP equal to the caster's current HP and cures Sleep, Poison, Confusion, Silence, Slow, Stop, Frog, Small, Slow-numb, Petrify, Berserk, Paralyzed, Darkness, Death Force and Resist on the whole target party. Cannot be reflected. |
| [Big Guard](enemy-skill/big-guard.md) | 56 | 0 | Magical | No | Grants Haste, Barrier and MBarrier to the whole target party. Cannot be reflected. |
| [Angel Whisper](enemy-skill/angel-whisper.md) | 50 | 32/32 | Magical | No | Restores one target to maximum HP and cures Death, Sleep, Poison, Confusion, Silence, Slow, Stop, Frog, Small, Slow-numb, Petrify, Berserk, Paralysis and Darkness. If the target dies before the ability resolves it does not retarget. Cannot be reflected. |
| [Dragon Force](enemy-skill/dragon-force.md) | 19 | 0 | Magical | No | Raises one target's Defense and Magic Defense to 150%. Cannot be reflected. |
| [Death Force](enemy-skill/death-force.md) | 3 | 0 | Magical | No | Grants Death Force status - immunity to instant death - to one target. Cannot be reflected. |
| [Flame Thrower](enemy-skill/flame-thrower.md) | 10 | 14 | Magical | No | Minor to moderate Fire-elemental damage to one target. |
| [Laser](enemy-skill/laser.md) | 16 | 16/32 | Magical | No | Gravity-elemental damage equal to half the target's current HP, to one target. |
| [Matra Magic](enemy-skill/matra-magic.md) | 8 | 11 | Magical | No | Non-elemental damage to the whole target party. |
| [Bad Breath](enemy-skill/bad-breath.md) | 58 | 0 | Magical | No | Inflicts Sleep, Poison, Confusion, Silence, Frog and Small on the whole target party. Cannot be reflected. |
| [Beta](enemy-skill/beta.md) | 35 | 54 | Magical | No | Major Fire-elemental damage to the whole target party. Cannot be reflected. |
| [Aqualung](enemy-skill/aqualung.md) | 34 | 52 | Magical | No | Major Water-elemental damage to the whole target party. |
| [Trine](enemy-skill/trine.md) | 20 | 34 | Magical | **Yes** | Moderate to major Lightning-elemental damage to the whole target party. Cannot be reflected. |
| [Magic Breath](enemy-skill/magic-breath.md) | 75 | 77 | Magical | No | Major Fire, Ice and Lightning damage to the whole target party. Cannot be reflected. |
| [????](enemy-skill/question-marks.md) | 3 | 0 | Physical | No | Non-elemental physical damage to one target equal to the user's max HP minus their current HP - so the more wounded the caster, the more it hurts. |
| [Goblin Punch](enemy-skill/goblin-punch.md) | 0 | 12 | Physical | No | Physical damage to one target. Damage is multiplied by 8 if the target's level exactly equals the caster's level. |
| [Chocobuckle](enemy-skill/chocobuckle.md) | 3 | 0 | Physical | **Yes** | Damage to one target equal to the number of times the party has fled from battle. |
| [L5 Death](enemy-skill/l5-death.md) | 22 | 0 | Magical | No | Instant death to every member of the target party whose level is a multiple of 5. Cannot be reflected. |
| [Death Sentence](enemy-skill/death-sentence.md) | 10 | 0 | Magical | No | Inflicts Death-sentence on one target - a countdown after which the target dies. Cannot be reflected. |
| [Roulette](enemy-skill/roulette.md) | 6 | 0 | Magical | No | Instant death to one randomly chosen target - which may be an ally. Cannot be reflected. |
| [Shadow Flare](enemy-skill/shadow-flare.md) | 100 | 125 | Magical | No | Non-elemental damage to one target. The highest-power enemy skill in the game. |
| [Pandora's Box](enemy-skill/pandoras-box.md) | 110 | 60 | Magical | **Yes** | Defense-ignoring non-elemental damage to all targets. |

---

Content adapted from the Final Fantasy Wiki, available under CC-BY-SA.
