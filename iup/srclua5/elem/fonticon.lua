------------------------------------------------------------------------------
-- FontIcon class
------------------------------------------------------------------------------
local ctrl = {
    nick = "fonticon",
    parent = iup.WIDGET,
    subdir = "elem",
    creation = "S",
    callback = {}
}

function iup.SetFontIcon(name, data)
    local h = iup.GetHandle(name)
    if h then iup.Destroy(h) end
    h = iup.fonticon{data = data}
    if h then
         iup.SetHandle(name, h)
         return true
    else
        return false
    end
end

function ctrl.createElement(class, param)
    return iup.FontIcon(param.data)
end


iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iupWidget")
