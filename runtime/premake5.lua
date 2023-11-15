---@diagnostic disable: lowercase-global, undefined-global

require "ninja"
require "export-compile-commands"

workspace "COP3530"
    configurations {"default", "test"}
    targetdir "build"
    architecture "x86_64"
    flags "MultiProcessorCompile"

project "yaml-cpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    outputdir = "%{cfg.buildcfg}"

    targetdir ("build/bin/" .. outputdir)
    objdir ("build/bin-int/" .. outputdir)

    --symbols "on"
    buildoptions {"-Werror", "-Wuninitialized"}

    defines "YAML_CPP_STATIC_DEFINE"

    includedirs 
    {
        "vendor/yaml-cpp/include",
    }

    files 
    { 
        "vendor/yaml-cpp/src/**.cpp",
        "vendor/yaml-cpp/src/**.h",
    }

    -- should be 14 probs
project "runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    outputdir = "%{cfg.buildcfg}"

    targetdir ("build/bin/" .. outputdir)
    objdir ("build/bin-int/" .. outputdir)

    defines "YAML_CPP_STATIC_DEFINE"
    
    symbols "on"
    buildoptions {"-Werror", "-Wuninitialized"}

    includedirs 
    {
        "vendor/yaml-cpp/include",
    }

    links 
    {
        "yaml-cpp"
    }

    files 
    { 
        "src/**.cpp",
        "src/**.h",
    }
