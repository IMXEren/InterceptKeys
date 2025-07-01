-- https://github.com/marzer/tomlplusplus/commit/708fff700f36ab3c2ab107b984ec9f3b8be5f055
package("toml++")
    set_kind("library", {headeronly = true})

    on_load(function (package)
        package:add("includedirs", os.scriptdir())
    end)

    on_fetch(function (package)
        local result = {}
        result.includedirs = package:get("includedirs")
        return result
    end)
