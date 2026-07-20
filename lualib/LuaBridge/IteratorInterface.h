//==============================================================================
/**
 * @file IteratorInterface.h
 *
 * @brief Defines the iterator interface and Lua wrapper for range-based
 *        iteration in Lua using RefCountedObjectPtr for lifetime management.
 *
 * This file provides a template-based iterator pattern that integrates with
 * LuaBridge's class registration system. It uses RefCountedObjectPtr to ensure
 * proper shared lifetime management between C++ and Lua.
 *
 * @copyright Copyright (c) 2024, LuaBridge authors.
 * @license MIT License (see LICENSE file)
 */
//==============================================================================

#pragma once

#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/RefCountedObject.h>
#include <LuaBridge/RefCountedPtr.h>
#include <LuaBridge/detail/Stack.h>

#include <type_traits>
#include <utility>

namespace luabridge {

//==============================================================================
/**
 * @brief Template interface for iterators with typed key and value.
 *
 * This class inherits from RefCountedObject to enable shared lifetime management
 * via RefCountedObjectPtr. This is essential for objects that are shared between
 * C++ and Lua, as it prevents premature destruction while Lua still holds
 * references.
 *
 * @tparam TKey   The type of the key (index/name) returned by the iterator.
 * @tparam TValue The type of the value returned by the iterator.
 *
 * Example usage:
 * @code
 * class MyIterator : public IteratorInterface<int, std::string> {
 * public:
 *     void next() override {
 *         if (index < data.size()) {
 *             key = index + 1;
 *             value = data[index++];
 *             done = false;
 *         } else {
 *             done = true;
 *         }
 *     }
 * private:
 *     std::vector<std::string> data;
 *     size_t index = 0;
 * };
 * @endcode
 */
template <typename TKey, typename TValue>
struct IteratorInterface : public RefCountedObject
{
    //--------------------------------------------------------------------------
    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     */
    virtual ~IteratorInterface() = default;

    //--------------------------------------------------------------------------
    /**
     * @brief Advances the iterator to the next element.
     *
     * This method must update the `done`, `key`, and `value` members.
     * When the iteration is complete, `done` should be set to `true`.
     *
     * @note This method is called by Lua after each iteration step.
     */
    virtual void next() = 0;

    //--------------------------------------------------------------------------
    /**
     * @brief Resets the iterator to its initial state.
     *
     * The default implementation resets `done` to `false` and value-initializes
     * `key` and `value`. Override this method if custom reset logic is needed.
     */
    virtual void reset()
    {
        done = false;
        key = TKey{};
        value = TValue{};
    }

    //--------------------------------------------------------------------------
    // Member variables
    //--------------------------------------------------------------------------

    /**
     * @brief Indicates whether the iteration has finished.
     *
     * This member should be set to `true` in `next()` when no more elements
     * are available.
     */
    bool done = false;

    /**
     * @brief The current key (index, name, or other identifier).
     *
     * This member is pushed to Lua as the first return value (`k` in `for k,v`).
     */
    TKey key{};

    /**
     * @brief The current value associated with the key.
     *
     * This member is pushed to Lua as the second return value (`v` in `for k,v`).
     */
    TValue value{};
};

//==============================================================================
/**
 * @brief Internal wrapper that holds an iterator and exposes it to Lua.
 *
 * This class is used internally by `Class::addEnumerator()` to manage the
 * lifetime of an iterator and provide the `__gc` metamethod for automatic
 * cleanup. It stores the iterator using RefCountedObjectPtr to ensure proper
 * shared ownership.
 *
 * @tparam TKey   The iterator's key type.
 * @tparam TValue The iterator's value type.
 *
 * @note This class is not intended for direct use by library clients.
 */
template <typename TKey, typename TValue>
struct LuaIteratorWrapper
{
    //--------------------------------------------------------------------------
    /**
     * @brief Constructs a wrapper that takes ownership of an iterator.
     *
     * @param it The iterator to wrap (must not be null).
     */
    explicit LuaIteratorWrapper(RefCountedObjectPtr<IteratorInterface<TKey, TValue> > it)
        : iter(std::move(it))
    {
        // Ensure the iterator is valid
        if (!this->iter) {
            throw std::runtime_error("LuaIteratorWrapper: null iterator");
        }
    }

    //--------------------------------------------------------------------------
    /**
     * @brief Lua C function that implements the iterator step.
     *
     * This function is called by Lua on each iteration of the `for` loop.
     * It retrieves the wrapper from the upvalue, checks if the iterator
     * is done, and pushes the current key and value to the Lua stack.
     *
     * @param L The Lua state.
     * @return  The number of values pushed to the stack (2 for key and value).
     *
     * @lua_stack_before
     *   - Upvalue 1: `LuaIteratorWrapper*` (the wrapper userdata)
     *
     * @lua_stack_after
     *   - Returns 2 values: `key` and `value`, or `nil, nil` if done.
     */
        static int luaNext(lua_State* L)
    {
        // Try to obtain wrapper either as first argument (when userdata used with __call)
        // or as upvalue (legacy closure mode). This keeps backward compatibility.
        LuaIteratorWrapper* wrapper = nullptr;

        if (lua_type(L, 1) == LUA_TUSERDATA) {
            // Called as userdata.__call(...): userdata is at stack index 1
            wrapper = static_cast<LuaIteratorWrapper*>(lua_touserdata(L, 1));
        }
        else {
            // Called as a closure with userdata upvalue (legacy path)
            void* up = lua_touserdata(L, lua_upvalueindex(1));
            wrapper = static_cast<LuaIteratorWrapper*>(up);
        }

        // Safety check: wrapper or its iterator could be null
        if (!wrapper || !wrapper->iter) {
            lua_pushnil(L);
            lua_pushnil(L);
            return 2;
        }

        IteratorInterface<TKey, TValue>& iter = *wrapper->iter;

        // Advance to the next element for the next call
        if (!iter.done)
            iter.next();

        // If iteration is complete, return nil for both values
        if (iter.done) {
            lua_pushnil(L);
            lua_pushnil(L);
            return 2;
        }
        else {
            // Push the current key and value to Lua using the Stack machinery
            Stack<TKey>::push(L, iter.key);
            Stack<TValue>::push(L, iter.value);
            return 2; // Two return values: key and value
        }
    }

    static void registerMetaTable(lua_State* L)
    {
        static const char* const metaName = "LuaIteratorWrapper";

        // Check if the metatable already exists
        luaL_getmetatable(L, metaName);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);

            // Create a new metatable
            luaL_newmetatable(L, metaName);

            // Set the __gc metamethod
            lua_pushcfunction(L, [](lua_State* L) -> int {
                LuaIteratorWrapper* wrapper = static_cast<LuaIteratorWrapper*>(
                    lua_touserdata(L, 1)
                    );
                if (wrapper) {
                    // Call the destructor to clean up the iterator
                    wrapper->~LuaIteratorWrapper();
                }
                return 0;
                });
            lua_setfield(L, -2, "__gc");

            // Protect the metatable from Lua-side modification
            lua_pushliteral(L, "This metatable is protected");
            lua_setfield(L, -2, "__metatable");

            // Optional: set __tostring for debugging
            lua_pushcfunction(L, [](lua_State* L) -> int {
                lua_pushfstring(L, "LuaIteratorWrapper (%p)", lua_touserdata(L, 1));
                return 1;
                });
            lua_setfield(L, -2, "__tostring");

            // Make userdata callable: set __call to luaNext (C function)
            // When called as userdata(...), luaNext will receive userdata as arg1.
            lua_pushcfunction(L, &LuaIteratorWrapper::luaNext);
            lua_setfield(L, -2, "__call");
        }
        lua_pop(L, 1); // Pop the metatable
    }

    static void pushToLua(lua_State* L, RefCountedObjectPtr<IteratorInterface<TKey, TValue> > it)
    {
        // Ensure the metatable is registered and contains __call, __gc, __tostring
        registerMetaTable(L);

        // Create userdata that holds the wrapper
        LuaIteratorWrapper* wrapper = static_cast<LuaIteratorWrapper*>(
            lua_newuserdatauv(L, sizeof(LuaIteratorWrapper), 0)
            );

        // Construct the wrapper in-place (takes ownership of the iterator)
        new (wrapper) LuaIteratorWrapper(std::move(it));

        // Attach the metatable with __gc and __call
        luaL_getmetatable(L, "LuaIteratorWrapper");
        lua_setmetatable(L, -2);

        // Now the userdata itself is callable (due to __call). Leave userdata on stack.
        // Previous implementation returned a function (closure) that captured userdata as upvalue.
        // Returning callable userdata gives us working __tostring and proper __gc behaviour.
    }

    //----------------------------------------------------------------------
    /**
     * @brief Push a member-function pointer into a userdata upvalue (no memcpy).
     *
     * This constructs the member-function object in-place inside a full
     * userdata. We require the member-function pointer type to be
     * trivially-copyable so no __gc destructor is required.
     */
    template <typename MemFn>
    static void pushMethodUpvalue(lua_State* L, MemFn m)
    {
        static_assert(std::is_trivially_copyable<MemFn>::value,
                      "Member-function pointer must be trivially copyable");

        void* ud = lua_newuserdatauv(L, sizeof(MemFn), 0); // pushes userdata
        new (ud) MemFn(m); // placement-new: construct in userdata
        // no metatable needed for trivial types
    }

    //----------------------------------------------------------------------
    /**
     * @brief Read a member-function pointer previously stored with pushMethodUpvalue().
     */
    template <typename MemFn>
    static MemFn readMethodUpvalue(lua_State* L, int upIndex)
    {
        void* up = lua_touserdata(L, upIndex);
        if (!up)
            return MemFn();
        return *static_cast<MemFn*>(up); // copy from in-place object
    }

    //--------------------------------------------------------------------------
    // Member variables
    //--------------------------------------------------------------------------

    /**
     * @brief The wrapped iterator (managed by RefCountedObjectPtr).
     *
     * The iterator is destroyed when the wrapper is destructed,
     * which happens when Lua's garbage collector collects the userdata.
     * The RefCountedObjectPtr ensures the iterator is kept alive as long as
     * either C++ or Lua holds a reference.
     */
    RefCountedObjectPtr<IteratorInterface<TKey, TValue> > iter;
};

//==============================================================================
/**
 * @brief Convenience alias for an iterator that only yields values (no keys).
 *
 * This is useful for iterators where the key is not needed (e.g., simple
 * value sequences). The key type is `int` and is automatically incremented
 * starting from 1, mimicking Lua's `ipairs`.
 *
 * @tparam TValue The type of the values yielded by the iterator.
 */
template <typename TValue>
using ValueIterator = IteratorInterface<int, TValue>;

//==============================================================================
/**
 * @brief Convenience alias for a string-keyed iterator.
 *
 * This is useful for iterating over associative containers like `std::map`
 * or `std::unordered_map` with string keys.
 *
 * @tparam TValue The type of the values yielded by the iterator.
 */
template <typename TValue>
using StringKeyIterator = IteratorInterface<std::string, TValue>;

namespace detail {

    template <typename TKey, typename TValue>
    struct Stack< RefCountedObjectPtr< IteratorInterface<TKey, TValue> > >
    {
        using IterPtr = RefCountedObjectPtr< IteratorInterface<TKey, TValue> >;

        static void push(lua_State* L, IterPtr const& it)
        {
            // Используем уже реализованный метод: создаёт userdata + closure
            LuaIteratorWrapper<TKey, TValue>::pushToLua(L, it);
        }

        static IterPtr get(lua_State* L, int index)
        {
            // Если на стеке функция (closure), то извлекаем upvalue 1 (userdata wrapper)
            if (lua_type(L, index) == LUA_TFUNCTION)
            {
                if (lua_getupvalue(L, index, 1) == nullptr)
                {
                    luaL_error(L, "expected iterator closure with wrapper upvalue");
                    return IterPtr();
                }

                // Upvalue теперь на стеке (вверху). Получим raw userdata pointer
                void* raw = lua_touserdata(L, -1);

                // Сравним метатаблицы, чтобы убедиться, что это наш LuaIteratorWrapper
                bool ok = false;
                if (raw != nullptr)
                {
                    // Получаем метатаблицу этого userdata
                    if (lua_getmetatable(L, -1))
                    {
                        // Получаем регистриованную метатаблицу с именем "LuaIteratorWrapper"
                        luaL_getmetatable(L, "LuaIteratorWrapper");
                        ok = (lua_rawequal(L, -1, -2) != 0);
                        lua_pop(L, 2); // pop registry meta + userdata meta
                    }
                }

                IterPtr ret;
                if (ok)
                {
                    auto wrapper = static_cast<LuaIteratorWrapper<TKey, TValue>*>(raw);
                    ret = wrapper->iter; // копируем RefCountedObjectPtr (инкремент refcount)
                }
                else
                {
                    lua_pop(L, 1); // pop upvalue
                    luaL_error(L, "expected LuaIteratorWrapper upvalue");
                    return IterPtr();
                }

                lua_pop(L, 1); // pop upvalue
                return ret;
            }

            // Если сразу userdata (вдруг кто-то передаёт wrapper напрямую)
            if (lua_type(L, index) == LUA_TUSERDATA)
            {
                void* raw = lua_touserdata(L, index);
                if (raw != nullptr)
                {
                    // Проверяем метатаблицу
                    if (lua_getmetatable(L, index))
                    {
                        luaL_getmetatable(L, "LuaIteratorWrapper");
                        bool ok = (lua_rawequal(L, -1, -2) != 0);
                        lua_pop(L, 2);
                        if (ok)
                        {
                            auto wrapper = static_cast<LuaIteratorWrapper<TKey, TValue>*>(raw);
                            return wrapper->iter;
                        }
                    }
                }
            }

            luaL_error(L, "expected iterator (closure returned by getIterator())");
            return IterPtr();
        }

        static bool isInstance(lua_State* L, int index)
        {
            int t = lua_type(L, index);

            if (t == LUA_TFUNCTION)
            {
                if (lua_getupvalue(L, index, 1) == nullptr)
                    return false;

                bool ok = false;
                if (lua_getmetatable(L, -1))
                {
                    luaL_getmetatable(L, "LuaIteratorWrapper");
                    ok = (lua_rawequal(L, -1, -2) != 0);
                    lua_pop(L, 2);
                }

                lua_pop(L, 1); // pop upvalue
                return ok;
            }

            if (t == LUA_TUSERDATA)
            {
                if (lua_getmetatable(L, index))
                {
                    luaL_getmetatable(L, "LuaIteratorWrapper");
                    bool ok = (lua_rawequal(L, -1, -2) != 0);
                    lua_pop(L, 2);
                    return ok;
                }
            }

            return false;
        }
    };

} // namespace detail
} // namespace luabridge