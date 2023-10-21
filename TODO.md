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
* Add validation to check that textures are a power of two. Unreal Engine would not generate mips unless your texture resolution is a power of 2. X and Y do not have to match they just need to be power of 2. For example, 2048x2048 and 2048x1024 both worked
* Texture resolutions that are not multiples of 4 will not compress.Nearly all (if not every) game-engine compression format (DXT, ETC, ASTC, etc.) is based on the idea of 4x4 blocks or sometimes 6x6 or 8x8. Bringing in an image with dimensions that are a multiple of 4, like 304x304, results in DXT compression, but 305x305 gives B8G8R8A8(uncompressed). So if you need to have some non-Po2 textures, it seems like restricting both dimensions to multiples of 4 (or 8 or 12 to be really safe) should do it. 
* Add check to ensure that every Blueprint (that inherits from a specific class?) has a Thumbnail specified
* Add check to ensure every parameter in a Material has a group/description specified.
* Add check to unreal that verifies every Niagara system has an "Effect Type" associated with it.
* Add check to unreal that verifies that "Masks" compression setting set for textures of the _AORMT or similar textures
* Add check to unreal that verifies that "Normalmap" compression setting set for textures of the _N or similar textures
* Add check to ensure sRGB set only BaseCOlor textures where expected?
* Add metadata to assets to track origin. (i.e. What license and where was it sourced from)


Add Action that applies a layout for textures based on metadata and types? Maybe something like:
Possible Directory layout:
 `Characters/[CharacterName]/Animations`
 `Characters/[CharacterName]/Materials`
 `Characters/[CharacterName]/Materials/Instances`
 `Characters/[CharacterName]/Materials/Functions`
 `Characters/[CharacterName]/Materials/Layers`
 `Characters/[CharacterName]/Rigs`
 `Characters/[CharacterName]/Textures`
