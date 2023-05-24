add_rules("mode.debug", "mode.release")

add_requires("glfw", {configs = {shared = true}})
add_requires("glad")
add_requires("assimp")
add_requires("freetype")
set_languages("c++20")

target("Game")
    set_kind("binary")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src/", {public  = true})
    add_packages("glfw","glad","assimp","freetype")
