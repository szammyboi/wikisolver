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

project "glfw"
	kind "StaticLib"
	language "C"

	outputdir = "%{cfg.buildcfg}"

    targetdir ("build/bin/" .. outputdir)
    objdir ("build/bin-int/" .. outputdir)

	files
	{
		"vendor/glfw/include/GLFW/glfw3.h",
		"vendor/glfw/include/GLFW/glfw3native.h",
		"vendor/glfw/src/**.c",
	}

    systemversion "latest"
    staticruntime "On"

    defines
    {
        "_GLFW_WIN32",
        "_CRT_SECURE_NO_WARNINGS"
    }

project "glad"
    kind "StaticLib"
    language "C"
    staticruntime "off"

    outputdir = "%{cfg.buildcfg}"

    targetdir ("build/bin/" .. outputdir)
    objdir ("build/bin-int/" .. outputdir)

    files
    {
        "vendor/glad/include/glad/gl.h",
        "vendor/glad/include/KHR/khrplatform.h",
        "vendor/glad/src/gl.c"
    }

    includedirs
    {
        "vendor/glad/include"
    }

    systemversion "latest"

project "imgui"
	kind "StaticLib"
	language "C++"

	outputdir = "%{cfg.buildcfg}"

    targetdir ("build/bin/" .. outputdir)
    objdir ("build/bin-int/" .. outputdir)

    includedirs 
    {
        "vendor/imgui",
        "vendor/glad/include",
        "vendor/glfw/include"
    }

	files
	{
        "vendor/imgui/*.cpp",
		"vendor/imgui/backends/imgui_impl_glfw.cpp",
		"vendor/imgui/backends/imgui_impl_opengl3.cpp",
        "vendor/imgui/misc/cpp/**.cpp"
	}

    defines 
    {
        "_IMGUI_WIN32",
		"_CRT_SECURE_NO_WARNINGS",
        "IMGUI_IMPL_OPENGL_LOADER_GLAD"
    }

    systemversion "latest"
    cppdialect "C++20"
    staticruntime "On"

    -- should be 14 probs
project "runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    outputdir = "%{cfg.buildcfg}"

    targetdir ("build/bin/" .. outputdir)
    objdir ("build/bin-int/" .. outputdir)

    defines {"BUILDING_LIBCURL"}

    --symbols "on"
    buildoptions {"-Werror", "-Wuninitialized", "-Wextra", "-march=native", "-Wno-return-type", "-Wno-sign-compare", "-Wno-missing-field-initializers"}

    includedirs 
    {
        "vendor/curl/include",
        "vendor/cpr/include",
        "vendor/cpr/include/custom",
        "vendor/json/include",
        "vendor/levenshtein-sse",
        "src/cpr",
        "vendor/glfw/include",
        "vendor/glad/include",
        "vendor/imgui",
        "vendor/imgui/backends",
        "vendor/imgui/misc/cpp"
    }

    -- normaliz, wdldap32
    links
    {
        "Ws2_32",
        "Crypt32",
        "bcrypt",
        "cpr",
        "curl",
        "opengl32",
        "gdi32",
        "imgui",
        "glad",
        "glfw",
    }

    files 
    {
        "src/**.cpp",
        "src/**.h",
    }
