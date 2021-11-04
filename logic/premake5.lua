project "logic"
	kind "SharedLib"
	language "C"
	cdialect "gnu99"

	targetdir "../"
	objdir "obj"

	staticruntime "on"

	files {
		"src/**.h",
		"src/**.c"
	}

	includedirs {
		"src",
		"../core/src"
	}

	links {
		"core"
	}

	defines {
		"EXPORT_SYMBOLS"
	}

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "on"
		runtime "debug"

	filter "configurations:release"
		defines { "RELEASE" }
		optimize "on"
		runtime "release"
