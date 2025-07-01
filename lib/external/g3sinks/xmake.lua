package("g3sinks")
    set_homepage("https://github.com/KjellKod/g3sinks")
    set_description("G3Sinks is a collection of sinks for g3log, the asynchronous logging library for C++11 and later.")
    set_license("Unlicense")
    
    add_urls("https://github.com/KjellKod/g3sinks/archive/refs/tags/$(version).tar.gz",
             "https://github.com/KjellKod/g3sinks.git")

    add_versions("2.2", "fe69ab0e3570024c5610a9482eb059d6e13a86ac78d7c6727e321fb312d7ec61")

    add_configs("logrotate", {description = "Enable log rotation sink", default = true, type = "boolean"})
    add_configs("syslog", {description = "Enable syslog sink", default = false, type = "boolean"})

    add_deps("g3log")
    add_deps("zlib")

    on_install("windows", "linux", "macosx", "iphoneos", function (package)
        if not (package:config("logrotate") or package:config("syslog")) then
            raise("At least one of logrotate or syslog sinks must be enabled.")
        end

        local configs = {
            "-DCHOICE_BUILD_DEBUG=OFF",
            "-DCHOICE_BUILD_EXAMPLES=OFF",
            "-DCHOICE_SINK_SNIPPETS=OFF",
            "-DCHOICE_INSTALL_G3SINKS=ON",
        }
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release"))
        table.insert(configs, "-DCHOICE_BUILD_STATIC=" .. (package:config("shared") and "OFF" or "ON"))
        table.insert(configs, "-DCHOICE_SINK_LOGROTATE=".. (package:config("logrotate") and "ON" or "OFF"))
        table.insert(configs, "-DCHOICE_SINK_SYSLOG=" .. (package:config("syslog") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            #include <g3log/g3log.hpp>
            #include <g3log/logworker.hpp>
            #include <g3sinks/LogRotate.h>
            #include <memory>
            void test() {
                using namespace g3;
                std::unique_ptr<LogWorker> logworker{ LogWorker::createLogWorker() };
                auto sinkHandle = logworker->addSink(std::make_unique<LogRotate>("logPrefix", "logDirectory"), &LogRotate::save);
                initializeLogging(logworker.get());
            }
        ]]}, {configs = {languages = "c++17"}}))
    end)