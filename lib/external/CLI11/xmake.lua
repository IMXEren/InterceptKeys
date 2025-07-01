-- https://github.com/CLIUtils/CLI11/commit/7a7ba4b9cacc9a9d49cc078f2340f4817ca7b677
package("CLI11")
    set_kind("library", {headeronly = true})

    on_load(function (package)
        package:add("includedirs", os.scriptdir())
    end)

    on_fetch(function (package)
        local result = {}
        result.includedirs = package:get("includedirs")
        return result
    end)