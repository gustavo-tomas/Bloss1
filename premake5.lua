-- Workspace -----------------------------------------------------------------------------------------------------------
workspace "bloss1"
    architecture "x86_64"
    toolset "gcc"
    language "C++"
    cppdialect "C++17"
    configurations { "debug", "release" }

    -- Change backends here
    defines { "_GLFW", "_OPENGL", "WITH_ALSA" }

    -- Run vendor premakes here
    include "vendor/soloud"
    include "vendor/glfw"

-- Engine --------------------------------------------------------------------------------------------------------------
project "bloss1"
    location "bloss1"
    kind "ConsoleApp"

    targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("bin/build/%{prj.name}")

    pchheader "%{prj.name}/src/pch.hpp"

    files { "%{prj.name}/src/**.hpp", "%{prj.name}/src/**.cpp" }

    -- @TODO: Don't forget to add all dependencies to the vendor folder
    includedirs { "%{prj.name}/src", "vendor/", "/usr/include/freetype2" }

    linkoptions
    {
        "-lGL", "-lGLEW",
        "-lassimp",
        "-lfreetype",
        "-lavcodec", "-lavformat", "-lavutil", "-lswscale",
        "-lasound" -- linux only
    }

    -- Links vendor libraries here
    links { "soloud", "glfw" }

    filter "system:linux"
        pic "On"

    filter "configurations:debug"
        buildoptions { "-Wall", "-Wextra", "-O0", "-fsanitize=address", "-fno-omit-frame-pointer" }
        linkoptions { "-fsanitize=address" }

        defines { "_DEBUG" }
        symbols "On" -- '-g'
        runtime "Debug"

    filter "configurations:release"
        buildoptions { "-O3" } -- Zoooom
        defines { "_RELEASE" }
        symbols "Off"
        runtime "Release"
