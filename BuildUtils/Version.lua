
local vCur = ''

local fname = arg[3]..'HildimVer.h'
local f = io.input(fname)
if f then
    vCur = f:read('*a')
    f:close()
end

local ver = arg[1]
local IUPver  = arg[2]
local t = {}

for d in ver:gmatch('(%d+)') do
    t[#t + 1] = d
end

for i = #t + 1, 4 do
    t[i] = '0'
end

local verWord = table.concat(t, ',')


local vTempl = [[#define VERSION_HILDIM "!ver!"

#ifdef IsHildiMApp
#define FILEDESCRIPTIONE FILEDESCRIPTION
#else
#define FILEDESCRIPTIONE FILEDESCRIPTION  "for Hildim " VERSION_HILDIM
#endif

#ifdef x64
#define VERSION_BIT " x64"
#else
#define VERSION_BIT " x32"
#endif
#ifdef Debug
#define _DEBUG
#define VERSIONEX VERSION_BIT " Debug"
#else
#define VERSIONEX VERSION_BIT
#endif


#define VERSION_IUP "!IUPver!"
#define VERSION_HILDIM_W !verWord!
#define FILEDESCRIPTIONEX FILEDESCRIPTIONE VERSIONEX
]]

vTempl = vTempl:gsub('!ver!', ver):gsub('!IUPver!', IUPver):gsub('!verWord!', verWord)

if vTempl == vCur:gsub('\r\n', '\n') then
    print('Current version: '..ver)
else
    local f = io.output(fname)
    if f then
        f:write(vTempl)
        f:flush()
        f:close()
        print('Version '..ver..' set')
    else
        print'Error when openi HildimVer.h for write varsion'
    end
end



