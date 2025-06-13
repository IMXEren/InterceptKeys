set_languages("c++17")
add_rules("mode.debug", "mode.release")
add_defines("UNICODE", "_UNICODE")
if is_mode("debug") then
    add_defines("_DEBUG")
end

includes("lib")
add_requires("interception")
add_requires("quill v9.0.3")

-- add_defines("BUILD_AS_SERVICE") -- Define this to build as a service else it will build as a console application
-- add_defines("ENABLE_LOGGING")   -- Define this to enable logging with log file rotation

option("service", {default = true, description = "Build as a Windows service executable"})
option("logging", {default = false, description = "Enable logging with log file rotation"})

if has_config("service") then
    add_defines("BUILD_AS_SERVICE")
end
if has_config("logging") then 
    add_defines("ENABLE_LOGGING")
end

set_runtimes(is_mode("debug") and "MDd" or "MD")
if is_mode("release") then
    add_cxflags("/GL", {force = true})
    add_ldflags("/LTCG", {force = true})
end

target("InterceptKeys")
    set_kind("static")
    add_files("InterceptKeys.cpp")
    add_files("logger.cpp")
    add_files("mapped_entries.cpp")
    add_packages("interception")
    add_packages("quill", {public = true})

target("Service")
    set_values("windows.subsystem", "console")
    add_ldflags("/SUBSYSTEM:CONSOLE")
    add_rules("win.sdk.application")
    add_files("Service.cpp")

    add_deps("InterceptKeys")

    after_build(function (target)
        local arch = is_arch("x86_64", "x64") and "x64" or "x86"
        local mode = is_mode("release") and "Release" or "Debug"
        os.cp(target:targetfile(), "$(scriptdir)/" .. arch .. "/" .. mode .. "/InterceptKeys.exe")
    end)

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

