-- Include conan gennerate script
include("conanbuildinfo.premake.lua")

-- Main Workspace
workspace "StringHasher"
    -- Import conan gennerate config
    conan_basic_setup()

    -- Project
    project "StrHWeb"
        kind "ConsoleApp"
        language "C++"
        targetdir "bin/%{cfg.buildcfg}"
		objdir "bin/%{cfg.buildcfg}/obj/"
		location "src"
        debugdir "app"

        linkoptions { conan_exelinkflags }

        files { "**.h", "**.cpp" }

        filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

		filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
