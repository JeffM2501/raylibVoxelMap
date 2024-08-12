# Raylib Voxel Map example
[WIP] Using chunks to show a voxel world


# Method of operation
This is an extension of the simple voxel mesher found in https://github.com/raylib-extras/examples-cpp/tree/main/voxel_mesher

The app generates a simple world using a perlin noise map and meshes the voxels as needed in threads, and then uploads them to the GPU in the main thread.

## Voxel_lib
All the code for the voxel world is part of the voxel_lib library. It contains the voxel data, world chunks and meshing system, as well as threading systems to generate and mesh chunks async from the main thread.

## Lighting_system
This is a C++ version of rlights.h that supports attenuation and spotlights


# TODO
 * Visibility Grid
 * Dynamic Mesh Load/Unload
 * Floating Origin
 * Skybox
 * Water
 * Better World Generation
 * Trees


# License
Copyright (c) 2020-2024 Jeffery Myers

This software is provided "as-is", without any express or implied warranty. In no event 
will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you 
  wrote the original software. If you use this software in a product, an acknowledgment 
  in the product documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be misrepresented
  as being the original software.

  3. This notice may not be removed or altered from any source distribution.
