#ifdef _DEBUG

#include "lstate.h"
#include "lobject.h"

// Ёкспортируема€ функци€, чтобы линкер не удалил еЄ.
// Ќикакой логики времени выполнени€ Ч только использование типа дл€ генерации отладинфо.
extern "C" __declspec(dllexport) void __force_debug_lua_state_type_for_debug()
{
    volatile size_t dummy = sizeof(lua_State);
    (void)dummy;
    dummy = sizeof(CClosure);
    (void)dummy;
    dummy = sizeof(LClosure);
    (void)dummy;
    dummy = sizeof(Udata);
    dummy = sizeof(LClosure);
    (void)dummy;
    Udata t; 
    LClosure l;
    CClosure c;
//    void(t);
//    void(l);
//    void(c);
}
#endif

