set_xmakever("2.5.0")
set_languages("c++14")

target("joblib")
    set_kind("shared")
    add_files("*.cpp", "../include/easylogging/easylogging++.cc")
    add_includedirs("../include")