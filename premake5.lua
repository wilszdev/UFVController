-- premake5.lua
workspace "UFV Controller"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "UFV Controller"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "UFV Controller"