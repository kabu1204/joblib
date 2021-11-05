set_xmakever("2.5.0")
set_languages("c++14")

target("test")
    set_kind("binary")
    add_files("*.cpp")
    add_deps("joblib")
    add_includedirs("../")