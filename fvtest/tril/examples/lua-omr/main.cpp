#include "lua.hpp"
#include <string>
#include <iostream>
#include <functional>

#define CALL_LUA(L, func)\
{int s = func(L);\
if(s){\
    cout << "CALL_LUA >> error: "<< lua_tostring(L, -1) << endl;\
}}
#define PRINT_RESULT(s)\
{if(s){\
    cout << "CALL_LUA >> error: "<< lua_tostring(L, -1) << endl;\
}}

using String = std::string;
using CString = const std::string&;
using namespace std;

static String getFileDir(CString file);

int main(int argc, char* argv[]){
    fprintf(stderr, "exe: %s\n", argv[0]);
    String dir = argc > 1 ? argv[1] : getFileDir(argv[0]);

    lua_State * L = luaL_newstate();
    luaL_openlibs(L);
    //luaopen_libtorch(L);
    // set path
    String script;
    script.reserve(256);
    script += "package.path=\"";
    script += dir.data();
    script += "/?.lua;\"..package.path;"
              "print('package.path = ', package.path)"
              ";print('package.cpath = ', package.cpath)"
            ;

    if(luaL_dostring(L, script.data())){
        cout << "error: "<< lua_tostring(L, -1) << endl;
    }else{
        cout << "lua do string success." << endl;
    }
    //luaT_getinnerparent(ls, "torch.DiskFile");
    String lua_file = dir + "/testAll.lua";
    CALL_LUA(L, [lua_file](lua_State * L){
        return luaL_dofile(L, lua_file.data());
    });
    lua_close(L);
    return 0;
}


static String getFileDir(CString file){
    int pos;
#if defined(_WIN32) || defined(WIN32)
    pos = file.rfind("/");
    if(pos < 0){
        pos = file.rfind("\\");
    }
#else
    pos = file.rfind("/");
#endif
    return file.substr(0, pos);
}
