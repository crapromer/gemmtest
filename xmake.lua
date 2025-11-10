add_rules("mode.debug", "mode.release")
set_project("vecdot_unified")
set_languages("cxx17")

target("vec_dot")
    set_kind("binary")
    add_files("vec_dot.cpp")

    if is_mode("release") then
        add_defines("NDEBUG")
        add_cxflags("-O3", {force = true})
    end

    -- 针对不同平台添加 SIMD 编译参数
    if is_plat("linux", "macosx") then
        if is_arch("x86_64", "i386") then
            add_cxflags("-march=native", "-mfma", "-mavx", {force = true})
        elseif is_arch("arm64", "aarch64", "armv7") then
            add_cxflags("-march=armv8-a+simd", "-mfpu=neon", {force = true})
        end
    elseif is_plat("windows") then
        if is_arch("x64") then
            add_cxflags("/O2", "/arch:AVX", "/fp:fast", {force = true})
        elseif is_arch("arm64") then
            add_cxflags("/O2", {force = true}) -- MSVC 自动支持 NEON
        end
    end
