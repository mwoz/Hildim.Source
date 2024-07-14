
if not arg or not arg[1] then
    io.stderr:write('Lua2lh: The first parameter must be set and contain the path to the lua file')
    os.exit(false)
    return
end


local filepath = arg[1]
local filename = arg[2]

local cTemplate = os.getenv'Lua2lhTrmplate'
if not cTemplate then
    io.stderr:write('Lua2lh: A users "Lua2lhTrmplate" macro must be set for the current project and bound to an environment variable')
    os.exit(false)
    return
end

local content = string.dump(assert(loadfile(filepath)))
if filename and not filepath:lower():gsub('\\','/'):find(filename:lower()..'$') then filename = nil end

if not filename then
    _,_, filename = filepath:find('([^/\\]*)$')
end

cTemplate = cTemplate:format(filename)

local dump do
    local numtab={}; for i=0,255 do numtab[string.char(i)]=("%3d,"):format(i) end
    function dump(str)
        return (str:gsub(".", numtab):gsub(("."):rep(80), "%0\n"))
    end
end

io.write([=[
/* code automatically generated by Lua2lh.lua -- DO NOT EDIT */
{
/* #include'ing this file in a C program is equivalent to calling
  if (luaL_loadfile(L,"]=]..filename..[=[")==0) lua_call(L, 0, 0);
*/
/* ]=]..filename..[=[ */
static const unsigned char B1[]={
]=]..dump(content)..[=[

};

]=]..cTemplate..[=[

}
]=])

