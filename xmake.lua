add_rules("mode.debug", "mode.release")
--add_requires("boost","mysql", "openssl", "libgd", "asio", "zlib", "brotli", "libqrencode", "libpng", "freetype")


set_languages("c++20")
set_encodings("utf-8")

if is_plat("windows") then 
    add_defines("WIN32")
    add_defines("WIN32_LEAN_AND_MEAN")
    add_syslinks("advapi32")
    if is_mode("debug") then
        add_defines("_DISABLE_VECTOR_ANNOTATION")
        add_defines("_DISABLE_STRING_ANNOTATION")
    end
end

if is_plat("mingw") then 
    add_defines("WIN32_LEAN_AND_MEAN")
end

add_requires("openssl","asio","zlib", "brotli")
--mysql
if is_plat("windows") then
    package("mysql")    
        add_configs("shared", {description = "Download shared binaries.", default = true, type = "boolean", readonly = true})
        set_urls("https://dev.mysql.com/get/Downloads/MySQL-8.0/mysql-8.0.35.tar.gz")
        add_versions("8.0.35", "917c5ed38704e99211185ce4be24e33a8c19c91241ed73af4181a6f38d1574c2")

        on_load("windows",function(package) 
            package:add("deps", "boost")
            package:add("deps", "openssl")
            package:add("deps", "zlib")
            package:add("deps", "zstd")
            package:add("deps", "lz4")
        end)

        add_includedirs("include", "include/mysql")

        on_install("mysql", function (package)
            io.gsub("CMakeLists.txt", "ADD_SUBDIRECTORY%(storage/ndb%)", "")
            local configs = {"-DCOMPILATION_COMMENT=XMake",
                             "-DDEFAULT_CHARSET=utf8",
                             "-DDEFAULT_COLLATION=utf8_general_ci",
                             "-DINSTALL_DOCDIR=share/doc/#{name}",
                             "-DINSTALL_INCLUDEDIR=include/mysql",
                             "-DINSTALL_INFODIR=share/info",
                             "-DINSTALL_MANDIR=share/man",
                             "-DINSTALL_MYSQLSHAREDIR=share/mysql",
                             "-DWITH_EDITLINE=bundled",
                             "-DWITH_UNIT_TESTS=OFF",
                             "-DDISABLE_SHARED=" .. (package:config("shared") and "OFF" or "ON"),
                             "-DWITH_LZ4='system'",
                             "-DWITH_ZSTD='system'",
                             "-DWITH_ZLIB='system'",
                             "-DWINDOWS_RUNTIME_MD=" .. (is_plat("windows") and package:config("vs_runtime"):startswith("MD") and "ON" or "OFF"),
                             "-DWITHOUT_SERVER=ON"}
            io.replace("cmake/ssl.cmake","IF(NOT OPENSSL_APPLINK_C)","IF(FALSE AND NOT OPENSSL_APPLINK_C)", {plain = true})
            for _, removelib in ipairs({"icu", "libevent", "re2", "rapidjson", "protobuf", "libedit"}) do
                io.replace("CMakeLists.txt", "MYSQL_CHECK_" .. string.upper(removelib) .. "()\n", "", {plain = true})
                io.replace("CMakeLists.txt", "INCLUDE(" .. removelib .. ")\n", "", {plain = true})
                io.replace("CMakeLists.txt", "WARN_MISSING_SYSTEM_" .. string.upper(removelib) .. "(" .. string.upper(removelib) .. "_WARN_GIVEN)", "# WARN_MISSING_SYSTEM_" .. string.upper(removelib) .. "(" .. string.upper(removelib) .. "_WARN_GIVEN)", {plain = true})
                io.replace("CMakeLists.txt", "SET(" .. string.upper(removelib) .. "_WARN_GIVEN)", "# SET(" .. string.upper(removelib) .. "_WARN_GIVEN)", {plain = true})
            end
            os.rmdir("extra")
            for _, folder in ipairs({"client", "man", "mysql-test", "libbinlogstandalone"}) do
                os.rmdir(folder)
                io.replace("CMakeLists.txt", "ADD_SUBDIRECTORY(" .. folder .. ")\n", "", {plain = true})
            end
            os.rmdir("storage/ndb")
            for _, line in ipairs({"INCLUDE(cmake/boost.cmake)\n", "MYSQL_CHECK_EDITLINE()\n"}) do
                io.replace("CMakeLists.txt", line, "", {plain = true})
            end
            io.replace("libbinlogevents/CMakeLists.txt", "INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libbinlogevents/include)", "MY_INCLUDE_SYSTEM_DIRECTORIES(LZ4)\nINCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libbinlogevents/include)", {plain = true})
            io.replace("cmake/install_macros.cmake", "  INSTALL_DEBUG_SYMBOLS(","  # INSTALL_DEBUG_SYMBOLS(", {plain = true})
            import("package.tools.cmake").install(package, configs)
            if package:is_plat("windows") then
                if package:config("shared") then
                    os.rm(package:installdir(path.join("lib", "mysqlclient.lib")))
                    os.cp(package:installdir(path.join("lib", "libmysql.dll")), package:installdir("bin"))
                else
                    os.rm(package:installdir(path.join("lib", "libmysql.lib")))
                    os.rm(package:installdir(path.join("lib", "libmysql.dll")))
                end
            else
                if package:config("shared") then
                    os.rm(package:installdir(path.join("lib", "*.a")))
                    os.cp(package:installdir(path.join("lib", "*.so.*")), package:installdir("bin"))
                else
                    os.rm(package:installdir(path.join("lib", "*.so.*")))
                end
            end
        end)

    package_end()    
    add_requires("mysql");
end


-- if !is_plat("windows") then
--     add_requires("openssl","mysql","asio","zlib", "brotli")
-- end   

add_defines("DEBUG")

add_includedirs("common")
add_includedirs("controller")
add_includedirs("vendor/httpcli")
add_includedirs("viewsrc/include")
add_includedirs("websockets/include")
add_includedirs("vendor/httpserver/include")
add_includedirs("vendor/httpserver/include/utility")
add_includedirs("orm")
add_includedirs("models/cms/include")
add_includedirs("models")
add_includedirs("controller/include")
add_includedirs("controller/include/admin")
add_includedirs("libs")
add_includedirs("libs/department")
add_includedirs("libs/img")
add_includedirs("libs/types")
add_includedirs("libs/ipdata")
add_includedirs("libs/markdown")
add_includedirs("include", "include/mysql")

for _, dir in ipairs(os.dirs("$(buildir)/libs/**")) do
    add_includedirs(dir)
end

for _, dir in ipairs(os.dirs("$(buildir)/orm/**")) do
    add_includedirs(dir)
end

for _, dir in ipairs(os.dirs("$(buildir)/controller/**")) do
    add_includedirs(dir)
end

for _, dir in ipairs(os.dirs("$(buildir)/vendor/**")) do
    add_includedirs(dir)
end

target("paozhu_pre")
    set_kind("binary")
    add_files("vendor/httpcli/autopickmethod.cpp")
    add_files("vendor/httpserver/src/md5.cpp")

target("paozhu")
    if is_mode("debug") then
        set_policy("build.sanitizer.address", true)
    end
    set_kind("binary")
    add_deps("paozhu_pre")
--    add_packages("boost")
    add_packages("asio")
    add_packages("openssl")
    add_packages("zlib")
--    add_packages("libgd")
--    add_packages("libqrencode")
--    add_packages("libpng")
--    add_packages("freetype")
    add_packages("mysql")
    add_packages("brotli")
    add_files("models/**.cpp")
    add_files("vendor/httpserver/**.cpp")
    add_files("common/**.cpp")
    add_files("controller/src/**.cpp")
    add_files("viewsrc/view/**.cpp")
    add_files("libs/**.cpp")
    add_files("test/test.cpp")

    on_load(function (target)
        if is_plat("linux", "macosx") then
            target:add("links", "pthread")
        elseif is_plat("windows") then
            target:add("links", "ws2_32")
            if os.isdir("include") then
                target:add({includedirs = "./include"})
            end 
            if os.isdir("lib") then
                for _, linkdir in ipairs({"./lib"}) do
                    if os.isdir(linkdir) then
                        target:add("linkdirs", linkdir)
                    end
                end
            end 
        end
    end)


    after_build(function (target)
        if is_mode("debug") then
            cprint("${red}enable asan also need copy dll from VS install dir. \nsuch as from \"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.38.33130\\bin\\Hostx64\\x64\\clang_rt.asan_dynamic-x86_64.dll\"")
        end
    end)

target("paozhu_empty")
    set_kind("binary")
    add_files("vendor/httpcli/autoemptymethod.cpp")
    add_deps("paozhu_pre")

target("paozhu_cli")
    set_kind("binary")
    add_files("vendor/httpcli/http_cli.cpp")

    add_packages("mysql")
    on_load(function (target)
        if is_plat("windows") then
            target:add("links", "ws2_32")
            if os.isdir("include") then
                target:add({includedirs = "./include"})
            end    
            if os.isdir("lib") then
                for _, linkdir in ipairs({"./lib"}) do
                    if os.isdir(linkdir) then
                        target:add("linkdirs", linkdir)
                    end
                end
            end 
        end
    end)