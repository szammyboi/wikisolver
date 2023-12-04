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

    defines { "CPR_FORCE_WINSSL_BACKEND", "CURL_STATICLIB"}

    --symbols "on"
    buildoptions {"-Werror", "-Wuninitialized"}

    includedirs 
    {
        "vendor/curl/include",
        "vendor/cpr/include",
    }

    files
    {
        "vendor/cpr/cpr/**.cpp"
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
    buildoptions {--[["-Werror",]] "-Wuninitialized", "-Wextra", "-march=native"}

    includedirs 
    {
        "vendor/curl/include",
        "vendor/cpr/include",
        "vendor/cpr/include/custom",
        "vendor/json/include",
        "vendor/levenshtein-sse",
        "src/cpr"
    }

    -- normaliz, wdldap32
    links
    {
        "Ws2_32",
        "Crypt32",
        "bcrypt",
        "cpr",
        "curl"
    }

    files 
    {
        "src/**.cpp",
        "src/**.h",
    }
