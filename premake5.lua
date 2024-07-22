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
  configurations { "debug", "release", "distribute", "edebug" }

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
  filter { "configurations:edebug" }
    defines { "SM166_DEBUG", "SM166_ERROR_DEBUG" }
    symbols "On"
  filter {}

  -- OS Macros
  filter { "system:linux" }
    defines { "SM166_LINUX" }
  filter {}

  filter { "configurations:edebug", "toolset:gcc" }
    defines { "_GLIBCXX_DEBUG" }
  filter { }

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

-- SM Boy Emulator Backend
project "sm166-boy"

  -- Project Configuration
  kind "StaticLib"
  location "./generated/sm166-boy"
  targetdir "./build/bin/sm166-boy/%{cfg.buildcfg}"
  objdir "./build/obj/sm166-boy/%{cfg.buildcfg}"

  -- Include Directories
  includedirs {
    "./projects/sm166/include",
    "./projects/sm166-boy/include"
  }

  -- Source Files
  files {
    "./projects/sm166-boy/src/**.cpp"
  }

-- SM Boy Emulator Frontend
project "smboy"

  -- Project Configuration
  kind "ConsoleApp"
  location "./generated/smboy"
  targetdir "./build/bin/smboy/%{cfg.buildcfg}"
  objdir "./build/obj/smboy/%{cfg.buildcfg}"

  -- Precompiled Headers
  pchheader "./projects/smboy/include/stdafx.hpp"
  pchsource "./projects/smboy/src/stdafx.cpp"

  -- Include Directories
  includedirs {
    "./projects/sm166/include",
    "./projects/sm166-boy/include",
    "./projects/smboy/include"
  }

  -- Source Files
  files {
    "./projects/smboy/src/**.cpp"
  }

  -- Link Libraries
  libdirs {
    "./build/bin/sm166/%{cfg.buildcfg}",
    "./build/bin/sm166-boy/%{cfg.buildcfg}"
  }

  links {
    "sm166-boy", "sm166", "pthread"
  }

  filter { "configurations:debug" }
    links { "sfml-audio-d", "sfml-graphics-d", "sfml-window-d", "sfml-system-d" }
  filter { "not configurations:debug" }
    links { "sfml-audio", "sfml-graphics", "sfml-window", "sfml-system" }
  filter {}


-- SM166 Assembler
project "smasm"

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
