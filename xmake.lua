set_version("0.0.1")
set_xmakever("2.5.1")
set_languages("cxx14")
set_project("joblib")
set_targetdir("xmake-build")

add_rules("mode.release", "mode.debug")
if is_mode("relwithdeb") then 
    set_symbols("debug") 
    set_optimize("faster") 
    set_strip("all")
end

includes("src", "test")