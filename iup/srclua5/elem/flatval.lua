------------------------------------------------------------------------------
-- Val class 
------------------------------------------------------------------------------
local ctrl = {
  nick = "flatval",
  parent = iup.WIDGET,
  subdir = "elem",
  funcname = "FlatVal",
  creation = "S",
  callback = {
    valuechanging_cb = "n",
  },
}

function ctrl.createElement(class, param)
   return iup.FlatVal(param[1])
end

iup.RegisterWidget(ctrl)
iup.SetClass(ctrl, "iupWidget")
