#pragma once

namespace Columbus::Assets
{

	// Definition of "Project"
	// Project would define asset import, have asset database (potentially?), as well as some data-driven info about what we are trying to do

	// Definition of "Level"
	// Project has a list of levels, levels contain all assets, collisions, lighting data and so on to get loaded
	// Level can (and probably should) be stored as GLTF
	// This would allow to modify data without breaking sources (such as creating clusters for geometry, baking lighting and so on)
	// Static geometry is baked into the level, fully. That would make it easy to edit source levels in external software.
	// 
	// All additional entities (lighting, settings, spawners, game-specific things) on the level have their serialisation mechanism.
	// They are saved alongside the level. These conversions are irreversible, so it essentially converts source assets
	// into engine-specific data.
	//
	// Level vs World
	// World is all runtime data that is used for rendering, gameplay, physics, audio.
	// Level is a data-level thing, a definition of what exists here.
	// World can load levels, either replacing the current data fully, or adding it on top (additive loading).
	// World can also load any other data, handles engine systems and lifecycle of things.
	// Level is just data, all Level logic is about managing data.

	// Roadmap:
	// - Create "Project"
	//     + New project
	//     + Project path, project settings, list of levels
	//     + Load project
	//     + Reflect it in the UI
	// - Create "Level"
	//     - Level settings
	//     - New level
	//     - Load level
	//     - Save level
	//     - Import level
	//     - Reimport level
	// - Create "Import"
	//     - GLTF level import base
	//     - Compress textures
	//     - Repack ORM textures

	struct TextureAsset
	{
		char SourcePath[512];
		char ImportedRelativePath[512]; // relative to what? to project source folder? we don't have "projects"
	};

	void ImportLevel(const char* SourcePath, const char* ImportPath);

	// TODO:
	// import materials: repack textures, compress textures, store
	// import levels with geometry

}
