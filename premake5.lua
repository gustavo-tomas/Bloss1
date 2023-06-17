-- Workspace -----------------------------------------------------------------------------------------------------------
workspace "bloss1"
    architecture "x86_64"
    toolset "gcc"
    language "C++"
    cppdialect "C++17"
    configurations { "debug", "release" }

-- Engine --------------------------------------------------------------------------------------------------------------
project "bloss1"
    location "bloss1"
    kind "ConsoleApp"

    targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("bin/build/%{prj.name}")

    pchheader "%{prj.name}/src/pch.hpp"

    files { "%{prj.name}/src/**.hpp", "%{prj.name}/src/**.cpp" }

    includedirs { "%{prj.name}/src" }

    linkoptions { "-lglfw", "-lGL", "-lGLEW" }
    
    filter "system:linux"
        pic "On"

    filter "configurations:debug"
        buildoptions { "-Wall", "-Wextra" }
        defines { "_DEBUG", "_OPENGL" }
        symbols "On" -- '-g'
        runtime "Debug"

    filter "configurations:release"
        buildoptions { "-O3" } -- Zoooom
        defines { "_RELEASE" }
        runtime "Release"
