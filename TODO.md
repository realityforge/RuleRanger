# TODO

Shorthand notes of where to go next with this experiment:

* Replace all the different ruby variants accomplishing the same behaviour
* Add a mechanism for adding an exclude `DataTable` that will will list assets that will ignore either particular rules or all rules?
* Enhance NameConventionRename action so that it can optionally warn on unknown assets (IsAsset=> true)
* Add an Action that renames according to a regex (primarily so can transform new to old naming conventions. Make
  it's priority higher than naming).
* Use regex matcher and regex renamer action to fix `_Inst` suffix on material instances. Actually lets just add rules
  to transform "old" conventions (i.e. SK_Mannequin_PhysicsAsset -> PHYS_Mannequin, SK_Mannequin_Skeleton -> 
  SKEL_Mannequin, etc). Make sure this runs before NameConventionRename action
* Material functions that are Material Layers are prefixed with ML not MF!
* Retargeters should be named as RTG_\[Source\]_To_\[Target\]
* Make a rule for handling texture extensions
  * Base Color: _BC
  * Ambient Occlusion: _AO
  * Roughness: _R
  * Specular: _S
  * Metallic: _M
  * Emissive: _E
  * Mask: _Mask
  * Flow Map: _F
  * Height: _H
  * Displacement: _D
  * Light Map: _L
  * Alpha/Opacity: _A
  * Packed: Use texture type in RGBA order. e.g., ARMH (Ambient Occlusion, Roughness, Metallic, Height)
* Add rules for "Level (World Partition)" => prefix: `L_`, suffix: `_WP`
* Add ability to add skip rules for assets in a user friendly way without changing RuleSet assets.
* Add a log of every action that resulted in change applied in a run
