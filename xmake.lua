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
        set_urls("https://github.com/xmake-mirror/mysql/releases/download/$(version)/mysql_$(version)_x64.zip")
        add_versions("8.0.31", "26312cfa871c101b7a55cea96278f9d14d469455091c4fd3ffaaa67a2d1aeea5")
        package:add("deps", "boost")
        package:add("deps", "openssl")
        package:add("deps", "zlib")
        package:add("deps", "zstd")
        package:add("deps", "lz4");

        add_includedirs("include", "include/mysql")

        on_install("mysql", function (package)
            os.cp("lib", package:installdir())
            os.cp("include", package:installdir())
            os.cp("lib/libmysql.dll", package:installdir("bin"))
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