package("interception")
    add_links("interception")

    on_load(function (package)
        local linkdir = os.scriptdir()
        if package:is_arch("x64", "x86_64") then 
            linkdir = path.join(linkdir, "x64")
        elseif package:is_arch("x86") then
            linkdir = path.join(linkdir,"x86")
        else
            raise("Unsupported architecture: " .. package:arch() .. ". Only x64 and x86 are supported.")
        end
        package:add("linkdirs", linkdir)
        package:add("includedirs", os.scriptdir())
    end)

    on_fetch(function (package)
        local result = {}
        result.includedirs = package:get("includedirs")
        result.linkdirs = package:get("linkdirs")
        result.links = package:get("links")
        result.ldflags = package:get("ldflags")
        return result
    end)