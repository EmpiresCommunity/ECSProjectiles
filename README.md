# ECSProjectiles
A (someday) Multiplayer ECS-Based Projectile and Ballistics Simulation for UE4

This UE plugin uses an ECS framework (FLECS in our case) to simulate simple ballistic projects that use linetraces to collide with things and Niagara render,
The Projectile code works but the networking section is very early on and can't replicate entities yet. 
You will certainly need modify code if you want to use this for your game.

## Quickstart
After enabling the plugin go to project settings and take note of the new MegaFLECS sections. In here you can add the prepacked ECSmodules to the list of enabled modules.

**Important**: The projectile subsystem needs to have a AECSProjectilesNiagaraManagerBP set in the ECSProjectileDeveloperSettings for it to spawn in the world!
This is what sets the AECSProjectilesNiagaraManagerBP actor to get spawned by the subsystem and the default Niagara Entity ID stuff. 

Now you should be able to call the blueprint library function "SpawnECSBulletNiagaraGrouped" and it should spawn a bullet with the default setup at the given location. 
There is an example in TestBulletSpammer.uasset unless I forgot to hook things up.


# Overview

## MegaFLECS

There are like a few different FLECS integrations for UE4 you can find on here (check out the FLECS github page) and this one isn't really much fancier than the rest:
* split code into ECS modules you can easily enable or disable. (pretty simple for now and really needs dependency management)
* UE4 ScriptStructs paired with each USTRUCTed FLECS component automatically for planned reflection.
* no editor support at all! 

The FLECS world is ticked in a UWorldSubsystem by subscribing itself as a tickable delegate.

## ECSProjectiles modules

### ECSProjectileModule_SimpleSim
Super simple ballistics projectile simulation that makes bullets go forward, fall from gravity, and even ricochet off surfaces. Definitely not done.

### ECSProjectileModule_Niagara
This ECSmodule spawns an actor that holds a Niagara system that accepts bullet position arrays to render. 
There is also some early support for just one Niagara system per bullet but you should probably not do that for anything there is more than 10 of.

For each FECSNiagaraGroup+BulletPositions they are paired with:
* Make an array of a given visual type of bullet's positions
* Stuff their previous frame and current positions into two TArrays and send it to a special Niagara System
* said Niagara system renders them as GPU particles and spawns/kills particles based on size of the positions TArray. 

Ideally each FECSNiagaraGroup "set" of bullets (one for all green bullets, one for all red bullets etc) would have their own FECSNiagaraGroup that they are in a FLECS pair relationship with and set when they spawn. 
This is how SpawnECSBulletNiagaraGrouped works with the current "default" system. Ideally one would have some way of mapping systems to their entity representations to pair them easily in the editor but I didn't get that far. The entire point is to have a small number of NiagaraSystems render hundreds of bullets.




## Why two arrays?
They are unorded and otherwise the Niagara particles would essentially have random velocities from other entities.

## What about the hit FX?
I tried to do the same thing as the regular projectile rendering but for explosions! This would remove the high cost of spawning the explosion effects actors.
It's definitely possible but a bit more complicated of a Niagara system that I haven't figured out completely. 

### ECSProjectileModule_Networking
This part is not currently working but we have a good start. The plan was to make our own DataChannel to send packets over for each entity. 
Entity IDs are not synched but each has an ECSnetworkID component that is used instead. 
Scriptstructs paired up with each USTRUCT using FLECS component definition entity (FLECS uses entities for just about everything internally) are the plan for serialization.


## Why bother?
We think ECS stuff is neat and it makes simulation logic like this very simple along with being extremely fast in comparison.
In tests I have gotten 40k+ bullets running at 16.66ms so I'm pretty happy with that compared to AActor performance.
I think most game projects should still just use regular UE actors for everything but spawning stuff a dozen times per frame is definitely a situation where it's time to get fancy and apply some DOD tactics.

## Why FLECS?
I liked how the C++ API works and it's very easy to integrate because it's written in C internally. 
There are other options out there for C++ ECS frameworks. At least check out ENTT as well if you are interested in others! 

## What about UE5?!
Yeah, it could probably run with almost no changes in the current EAs. The hard part will be making sure the Niagara systems don't get mangled somehow. 
I believe subsystems have their own tickable thing now instead of the setup we have which is cool.




