---@diagnostic disable: lowercase-global, undefined-global

require "ninja"
require "export-compile-commands"

workspace "WikipediaSolver"
    configurations {"default", "test"}
    targetdir "build"
    architecture "x86_64"
    flags "MultiProcessorCompile"

project "curl"
	language "C"
	kind "StaticLib"
	targetname "curl"
    staticruntime "On"

    outputdir = "%{cfg.buildcfg}"

    targetdir ("build/bin/" .. outputdir)
    objdir ("build/bin-int/" .. outputdir)

	includedirs { "vendor/curl/include", "vendor/curl/lib" }
    buildoptions '-MP'
	defines { 
        "CURL_STATICLIB",
        "BUILDING_LIBCURL",
        "USE_IPV6",
        "HTTP_ONLY",
        "USE_WINDOWS_SSPI",
        "USE_SCHANNEL",
        "_WINSOCK_DEPRECATED_NO_WARNINGS",
        "USE_WINDOWS_SSPI",
        "USE_WIN32_IDN",
        "WANT_IDN_PROTOTYPES"
    }

	files "vendor/curl/lib/**.c"

    links { 
        "Normaliz", 
	 	"Ws2_32", 
	 	"Crypt32",
	 	"Wldap32",
    }

project "cpr"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    outputdir = "%{cfg.buildcfg}"

    targetdir ("build/bin/" .. outputdir)
    objdir ("build/bin-int/" .. outputdir)

    defines { "CPR_FORCE_WINSSL_BACKEND", "BUILDING_LIBCURL", "CURL_STATICLIB"}

    --symbols "on"
    buildoptions {"-Werror", "-Wuninitialized"}

    includedirs 
    {
        "vendor/curl/include",
        "vendor/cpr/include",
        "vendor/cpr/include/custom",
    }

    files
    {
        "vendor/cpr/cpr/**.cpp"
    }

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

    defines {"YAML_CPP_STATIC_DEFINE"}

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

    defines {"YAML_CPP_STATIC_DEFINE", "BUILDING_LIBCURL"}

    --symbols "on"
    buildoptions {"-Werror", "-Wuninitialized"}

    includedirs 
    {
        "vendor/yaml-cpp/include",
        "vendor/curl/include",
        "vendor/cpr/include",
        "vendor/cpr/include/custom",
        "vendor/json/include"
    }

    -- normaliz, wdldap32
    links
    {
        --"yaml-cpp",
        "Ws2_32",
        "Crypt32",
        "bcrypt",
        "cpr",
        "curl",
    }

    files 
    {
        "src/**.cpp",
        "src/**.h",
    }
