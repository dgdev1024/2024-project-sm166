-- @file premake5.lua

-- Workspace Settings
workspace "project-sm166"

  -- Language and Standard
  language "C++"
  cppdialect "C++20"
  filter { "toolset:gcc" }
    cppdialect "gnu++20"
  filter {}

  -- Extra Warnings + Treat Warnings as Errors
  warnings "Extra"
  flags { "FatalWarnings" }

  -- Build Configuration
  location "./generated"
  configurations { "debug", "release", "distribute" }

  -- Build Macros
  filter { "configurations:debug" }
    defines { "SM166_DEBUG" }
    symbols "On"
  filter { "configurations:release" }
    defines { "SM166_RELEASE" }
    optimize "On"
  filter { "configurations:distribute" }
    defines { "SM166_DISTRIBUTE" }
    optimize "On"
  filter {}

  -- OS Macros
  filter { "system:linux" }
    defines { "SM166_LINUX" }
  filter {}

-- SM166 CPU Emulator Backend
project "sm166"

  -- Project Configuration
  kind "StaticLib"
  location "./generated/sm166"
  targetdir "./build/bin/sm166/%{cfg.buildcfg}"
  objdir "./build/obj/sm166/%{cfg.buildcfg}"

  -- Include Directories
  includedirs {
    "./projects/sm166/include"
  }

  -- Source Files
  files {
    "./projects/sm166/src/**.cpp"
  }

-- SM166 Assembler
project "sm166-asm"

  -- Project Configuration
  kind "ConsoleApp"
  location "./generated/sm166-asm"
  targetdir "./build/bin/sm166-asm/%{cfg.buildcfg}"
  objdir "./build/obj/sm166-asm/%{cfg.buildcfg}"

  -- Include Directories
  includedirs {
    "./projects/sm166/include",
    "./projects/sm166-asm/include"
  }

  -- Source Files
  files {
    "./projects/sm166-asm/src/**.cpp"
  }

  -- Link Libraries
  libdirs {
    "./build/bin/sm166/%{cfg.buildcfg}"
  }

  links {
    "sm166"
  }
