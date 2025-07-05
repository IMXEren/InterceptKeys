set_version("0.1.1", {build = "%Y%m%d%H%M"})
set_allowedplats("windows")
set_allowedarchs("x86", "x64", "x86_64")
set_languages("c++17")
add_rules("mode.debug", "mode.release")
add_defines("UNICODE", "_UNICODE")
if is_mode("debug") then
    add_defines("_DEBUG")
end

option("vs_build")
    set_description("Copy the built executable and interception.dll to the Visual Studio output directory")
    set_default(true)
option_end()
option("dist")
    set_description("Create a distribution package with the built executable, interception.dll etc.")
    set_default(false)
option_end()

includes("lib")
add_requires("interception")
add_requires("CLI11")
add_requires("toml++")
add_requires("fmt 11.2.0")
add_requires("quill v10.0.1")

set_runtimes(is_mode("debug") and "MDd" or "MD")
if is_mode("release") then
    add_cxflags("/GL", {force = true})
    add_ldflags("/LTCG", {force = true})
end

local interception_shared_lib = "interception.dll"
target("InterceptKeys")
    set_kind("static")
    add_files("InterceptKeys.cpp")
    add_files("logger.cpp")
    add_includedirs("$(builddir)/config")

    add_packages("interception")
    add_packages("CLI11", {public = true})
    add_packages("toml++", {public = true})
    add_packages("quill", {public = true})
    add_packages("fmt")
    -- Instead of using {public = true}, directly add packages due to no support for inheritance of cxxflags
    -- ref: https://github.com/xmake-io/xmake/issues/5653#issuecomment-2368107908

    after_build(function (target)
        interception_shared_lib = path.join(target:pkg("interception"):get("linkdirs"), interception_shared_lib)
    end)

target("Service")
    set_values("windows.subsystem", "console")
    add_rules("win.sdk.application")
    add_files("Service.cpp")
    add_deps("InterceptKeys")
    add_packages("fmt")

    add_configfiles("config.h.in")
    set_configdir("$(builddir)/config")
    add_includedirs("$(builddir)/config")

    after_build(function (target)
        if has_config("vs_build") then
            local arch = is_arch("x86_64", "x64") and "x64" or "x86"
            local mode = is_mode("release") and "Release" or "Debug"
            local vs_outdir = path.join(os.scriptdir(), arch, mode)
            os.cp(target:targetfile(), path.join(vs_outdir, "InterceptKeys.exe"))
            os.cp(interception_shared_lib, vs_outdir)
        end

        if has_config("dist") then
            local arch = is_arch("x86_64", "x64") and "x64" or "x86"
            local mode = is_mode("release") and "Release" or "Debug"
            local outname = "InterceptKeys_" .. mode:lower() .. "_" .. arch
            local outdir = path.join(os.tmpdir(), outname)

            os.mkdir(outdir)

            -- Copy artifacts
            local exe_path = path.join(outdir, "InterceptKeys.exe")
            os.cp(target:targetfile(), exe_path)
            os.cp(interception_shared_lib, path.join(outdir, "interception.dll"))
            os.cp("mapping.toml", path.join(outdir, "mapping.toml.example"))
            os.cp("README.md", outdir)
            os.cp("docs", outdir)

            -- Write config file
            local config_txt = string.format(
                "mode=%s\narch=%s\n",
                mode, arch
            )
            io.writefile(path.join(outdir, ".BUILD_CONFIG"), config_txt)

            local archive_path = path.join(os.scriptdir(), "dist", outname .. ".7z")
            local oldcwd = os.cd(outdir)
            os.execv("7z", {"a", "-mx9", archive_path, "*"})
            os.cd(oldcwd)

            print("✓ Packed: " .. archive_path)
            os.rm(outdir, {force = true}) -- Clean up temp directory
        end
    end)

task("build_dist")
    set_menu {
        usage = "xmake build_all_variants",
        description = "Build all combinations of arch, mode, and config options"
    }

    on_run(function ()

        local modes = {"debug", "release"}
        local archs = {"x86", "x64"}

        for _, mode in ipairs(modes) do
            for _, arch in ipairs(archs) do
                local config = {
                    mode = mode,
                    arch = arch,
                }

                print("==> Building: mode=" .. mode .. " arch=" .. arch)

                -- set config
                os.vrunv("xmake", {"f", "-y", "-m", mode, "-a", arch,
                                    "--vs_build=n", "--dist=y"})

                -- build
                os.vrun("xmake build")
            end
        end
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

