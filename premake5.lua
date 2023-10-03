-- Workspace -----------------------------------------------------------------------------------------------------------
workspace "bloss1"
    architecture "x86_64"
    toolset "gcc"
    language "C++"
    cppdialect "C++17"
    configurations { "debug", "profile", "release" }

    -- Change backends here
    defines { "_GLFW", "_OPENGL", "_SOLOUD", "WITH_ALSA" }

    -- Run vendor premakes here
    include "vendor/soloud"
    include "vendor/glfw"
    include "vendor/assimp"
    include "vendor/imgui"

-- Engine --------------------------------------------------------------------------------------------------------------
project "bloss1"
    location "bloss1"
    kind "ConsoleApp"

    targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("bin/build/%{prj.name}")

    pchheader "%{prj.name}/src/pch.hpp"

    files { "%{prj.name}/src/**.hpp", "%{prj.name}/src/**.cpp" }

    -- @TODO: Don't forget to add all dependencies to the vendor folder
    includedirs 
    { 
        "%{prj.name}/src",
        "vendor/glfw/include",
        "vendor/assimp/include",
        "vendor/glm",
        "vendor/imgui",
        "vendor/",
        "/usr/include/freetype2"
    }

    linkoptions
    {
        "-lGL", "-lGLEW",
        "-lfreetype",
        "-lavcodec", "-lavformat", "-lavutil", "-lswscale",
        "-lasound" -- linux only
    }

    -- Links vendor libraries here
    links { "soloud", "glfw", "assimp", "imgui" }

    filter "system:linux"
        pic "On"

    filter "configurations:debug"
        buildoptions { "-Wall", "-Wextra", "-fsanitize=address", "-fno-omit-frame-pointer" }
        linkoptions { "-fsanitize=address" }

        defines { "_DEBUG", "BLS_PROFILE" }
        symbols "On" -- '-g'
        optimize "Off" -- '-O0'
        runtime "Debug"

    filter "configurations:profile"
        defines { "_PROFILE", "BLS_PROFILE" }
        symbols "Off"
        optimize "On" -- 'O2'
        runtime "Release"

    filter "configurations:release"
        defines { "_RELEASE" }
        symbols "Off"
        optimize "Full" -- '-O3'
        runtime "Release"
