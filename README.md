# Micro and Macro AI

### Introduction
The concept of a micro- and macro-AI is not new. It was first used in Left 4 Dead as a dynamic system for pacing and difficulty . The macro AI, also known as the Director AI, spawns enemies depending on several factors, such as: player health, the player’s current ammo, the player’s skill, location and current situation.
The micro AI deals with the separate NPC’s spawned by the macro AI. Each type of NPC often has its own micro AI, ranging from a zombie simply chasing the player, to an enemy trying to outflank a player.
In this research project, I will be focusing on the implementation of, and more extensively, the synergy between, a micro and macro AI in a small game. 

### The Game
I shall make a very simplified version of the game Alien Isolation. The player will wander around the map, and he will flee when he sees the Alien.
The Alien will systematically search the map, based on the input it receives from the macro AI.
This will be a priority system according to its situation: if the Alien sees the player, it will hunt the player. If the Alien is nowhere near the player, the Director will systematically direct it towards the player.
The macro AI will also make the Alien go away from the player according to several factors, such as how close the Alien is to the player, and whether the player can run away

### The Framework
For this research project, I decided not to use the Unity or the Unreal Engine, but the framework provided to us by Professors Vandaele and Geens, with some slight adjustments, since their framework only provided the necessities for the course. As such, some small features were added, such as:
* The ability to rotate navigation colliders 90 degrees in the navigation mesh.
* Some collision detection functionality, such as ray-casting.
* An **InvertedBehaviorConditional** node for the behaviour tree. This is easier and cleaner than having to implement **IsXConditionMet** and **IsXConditionNotMet** behaviours.

![RotateNavigationColliders](https://github.com/Rhidian12/GPP_Framework/master/RotateNavigationColliders.png)
