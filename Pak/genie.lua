--
dofile("config.lua");

BUILD_DIR = path.getabsolute("./build")

solution "Pak tools"
	location(BUILD_DIR)
	
	configurations {
		"Debug",
		"Release"
	}

	platforms {
		"x32",
		"x64"
	}
	
	language "C++"
	
	configuration {"Debug"}
		targetsuffix "_debug"
		flags {
			"Symbols",
			"FullSymbols"
		}
		defines {
			"DEBUG",
			"CONF_DEBUG"
		}
		links {
		}
	
	configuration {"Release"}
		flags {
			"Optimize",
			"Symbols",
			"NoBufferSecurityCheck"
		}
		defines {
			"NDEBUG",
			"CONF_RELEASE",
		}
		links {
		}
	
	configuration {}
		flags {
			"Cpp14"
		}
	
	targetdir("../build")
	
	includedirs {
		"src",
	}
	
	links {
		"kernel32",
		"user32",
		"winmm",
		"advapi32"
	}
	
	flags {
		"NoExceptions",
		"NoRTTI",
		"EnableSSE",
		"EnableSSE2",
		"EnableAVX",
		"EnableAVX2",
		"EnableMinimalRebuild",
		"StaticRuntime",
	}
	
	defines {
		"_CRT_SECURE_NO_DEPRECATE",
		"_CRT_NONSTDC_NO_DEPRECATE",
		"WIN32_LEAN_AND_MEAN",
		"NOMINMAX",
    }
	
	-- disable exception related warnings
	buildoptions{ "/wd4577", "/wd4530" }
	

project "pak"
	kind "ConsoleApp"

	configuration {}

	includedirs {
		"src",
	}
	
	files {
		"src/**.h",
		"src/**.c",
		"src/**.cpp",
	}