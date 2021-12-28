ifeq ($(OS), Windows_NT)
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmakewin.mak TEC_UNAME=$(TEC_UNAME)
else
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmake.mak
endif

.PHONY: do_all cd cdpdf cdgl cdim cdcontextplus cdlua5 cdluapdf5 cdluagl5 cdluacontextplus5 cdluaim5
do_all:        cd cdpdf cdgl cdim cdcontextplus cdlua5 cdluapdf5 cdluagl5 cdluacontextplus5 cdluaim5

cd:
	@$(TECMAKE_CMD)
  
cdpdf:
	@$(TECMAKE_CMD) MF=cdpdf
cdgl:
	@$(TECMAKE_CMD) MF=cdgl
cdim:
	@$(TECMAKE_CMD) MF=cdim
  
cdcontextplus:
	@$(TECMAKE_CMD) MF=cdcontextplus
cddirect2d:
	@$(TECMAKE_CMD) MF=cddirect2d
cdcairo:
	@$(TECMAKE_CMD) MF=cdcairo
  
# Building for the default (USE_LUA51) 
# or for the defined at the environment
  
cdlua5:
	@$(TECMAKE_CMD) MF=cdlua5 
cdluapdf5:
	@$(TECMAKE_CMD) MF=cdluapdf5 
cdluagl5:
	@$(TECMAKE_CMD) MF=cdluagl5 
cdluacontextplus5:
	@$(TECMAKE_CMD) MF=cdluacontextplus5 
cdluadirect2d5:
	@$(TECMAKE_CMD) MF=cdluadirect2d5 
cdluaim5:
	@$(TECMAKE_CMD) MF=cdluaim5 
