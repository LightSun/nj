﻿/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2016, 2017
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 ******************************************************************************/

#include "LuaTypeDictionary.hpp"
#include "luavm.hpp"

#include <type_traits>

//decltype: like auto, used to infer type
#define DEFINE_FIELD_T(type, field) DefineField(#type, #field, toIlType<decltype(type::field)>())
#define DEFINE_FIELD(type, field, ilt)\
    DefineField(#type, #field, ilt)

#define UNION_FIELD_T(type, field) UnionField(#type, #field, toIlType<decltype(type::field)>())

#define STR(s) #s
#define CLOSE_STRUCT(s) \
    CloseStruct(STR(s))
#define DEFINE_STRUCT(s) \
    DefineStruct(STR(s))

#define TString_STR "TString"
#define TValue_STR "TValue"
#define Proto_STR "Proto"
#define next "next"

Lua::TypeDictionary::TypeDictionary() : TR::TypeDictionary() {
   // common lua types
   luaTypes.lu_byte         = toIlType<lu_byte>();
   luaTypes.lua_Integer     = toIlType<lua_Integer>();
   luaTypes.lua_Unsigned    = luaTypes.lua_Integer;
   luaTypes.lua_Number      = toIlType<lua_Number>();
   luaTypes.Instruction     = toIlType<Instruction>();
   luaTypes.TMS             = toIlType<std::underlying_type<TMS>::type>();

   // placeholder and convenience types
   auto pGCObject_t = toIlType<void*>();
   auto pGlobalState_t = toIlType<void*>();      // state of all threads
   auto pUpVal_t = toIlType<void*>();
   auto pLuaLongjmp = toIlType<void*>();
   auto lua_Hook_t = toIlType<void*>();          // is actually `void(*)(lua_State*, lua_Debug*)
   auto pInstruction = PointerTo(luaTypes.Instruction);
   auto lua_CFunction_t = toIlType<void*>();

   // lobject.h types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   // union Value
   luaTypes.Value = DefineUnion("Value");
   UnionField("Value", "gc", pGCObject_t);      // collectable objects
   UNION_FIELD_T(Value, p);                     // light usedata
   UNION_FIELD_T(Value, b);                     // booleans
   UnionField("Value", "f", lua_CFunction_t);   // light C functions
   UNION_FIELD_T(Value, i);                     // integer numbers
   UNION_FIELD_T(Value, n);                     // float numbers
   CloseUnion("Value");

   luaTypes.TString = DefineStruct(TString_STR);
   luaTypes.p_TString = PointerTo(luaTypes.TString);
   DefineField(TString_STR, next, pGCObject_t);             // CommonHeader
   DEFINE_FIELD_T(TString, tt);                          //      |
   DEFINE_FIELD_T(TString, marked);                      //      |
   DEFINE_FIELD_T(TString, extra);                       // reserved words for short strings; "has hash" for longs
   DEFINE_FIELD_T(TString, shrlen);                      // length for short strings
   DEFINE_FIELD_T(TString, hash);
   DEFINE_FIELD(TString, u.hnext, luaTypes.p_TString);   // linked list for hash table
   CLOSE_STRUCT(TString);

   // struct TValue
   luaTypes.TValue = DEFINE_STRUCT(TValue);
   DEFINE_FIELD(TValue, value_, luaTypes.Value);
   DEFINE_FIELD_T(TValue, tt_);
   CLOSE_STRUCT(TValue);

   luaTypes.StkId = PointerTo("TValue"); // stack index

   // struct Proto
   luaTypes.Proto = DEFINE_STRUCT(Proto);
   auto pProto = PointerTo("Proto");
   DEFINE_FIELD_T(Proto, sizep);
   DEFINE_FIELD(Proto, compiledcode, toIlType<void*>());
   DEFINE_FIELD_T(Proto, jitflags);
   DEFINE_FIELD_T(Proto, callcounter);
   CLOSE_STRUCT(Proto);

   // struct LClosure
   luaTypes.LClosure = DEFINE_STRUCT(LClosure);
   DEFINE_FIELD(LClosure, next, pGCObject_t);           // ClosureHeader  CommonHeader
   DEFINE_FIELD(LClosure, tt, luaTypes.lu_byte);        //      |              |
   DEFINE_FIELD(LClosure, marked, luaTypes.lu_byte);    //      |              |
   DEFINE_FIELD(LClosure, nupvalues, luaTypes.lu_byte); //      |
   DEFINE_FIELD(LClosure, gclist, pGCObject_t);         //      |
   DEFINE_FIELD(LClosure, p, pProto);
   DEFINE_FIELD(LClosure, upvals, toIlType<void*>());
   CLOSE_STRUCT(LClosure);

   // lfunc.h types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   luaTypes.UpVal = DEFINE_STRUCT(UpVal);
   DEFINE_FIELD(UpVal, v, PointerTo(luaTypes.TValue));                    // points to stack or to its own value
   DEFINE_FIELD_T(UpVal, refcount);                                       // reference counter
/*
   union {
      struct {                                                            // (when open)
         UpVal* next;                                                     // linked list
         int touched;                                                     // mark to avoid cycles with dead threads
      } open;
*/
      DEFINE_FIELD(UpVal, u.value, luaTypes.TValue); /* TValue value; */  // the value (when closed)
/*
   } u;
*/
   CLOSE_STRUCT(UpVal);

   // lstate.h types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   // struct CallInfo (Information about a call)
   luaTypes.CallInfo = DEFINE_STRUCT(CallInfo);
   TR::IlType* pCallInfo_t = PointerTo("CallInfo");
   DEFINE_FIELD(CallInfo, func, luaTypes.StkId);  // function index in the stack
   DEFINE_FIELD(CallInfo, top, luaTypes.StkId);   // top for this function
   DEFINE_FIELD(CallInfo, previous, pCallInfo_t); // dynamic call link
   DEFINE_FIELD(CallInfo, next, pCallInfo_t);     //       |
/*
   union { // Node: JitBuilder does not currently support defining unions
       struct {   // only for Lua functions
*/
           DEFINE_FIELD(CallInfo, u.l.base, luaTypes.StkId);   /* StkId base; */ // base for this function
           DEFINE_FIELD(CallInfo, u.l.savedpc, pInstruction);  /* const Instruction* savedpc; */
/*
       } l;
       struct {   // only for C functions
           lua_KFunction k;         // continuation in case of yields
           ptrdiff_t old_errfunc;
           lua_KContext ctx;        // context info. in case of yields
       } c;
   } u;
*/
   DEFINE_FIELD_T(CallInfo, extra);
   DEFINE_FIELD(CallInfo, nresults, Int16);
   DEFINE_FIELD(CallInfo, callstatus, luaTypes.lu_byte);
   CLOSE_STRUCT(CallInfo);

   // struct lua_State (per thread state)
   luaTypes.lua_State = DEFINE_STRUCT(lua_State);
   TR::IlType* pLuaState_t = PointerTo("lua_State");
   DEFINE_FIELD(lua_State, next, pGCObject_t);             // CommonHeader
   DEFINE_FIELD(lua_State, tt, luaTypes.lu_byte);          //      |
   DEFINE_FIELD(lua_State, marked, luaTypes.lu_byte);      //      |
   DEFINE_FIELD_T(lua_State, nci);                         // number of items in `ci` list
   DEFINE_FIELD(lua_State, status, luaTypes.lu_byte);
   DEFINE_FIELD(lua_State, top, luaTypes.StkId);           // first free slot in the stack
   DEFINE_FIELD(lua_State, l_G, pGlobalState_t);
   DEFINE_FIELD(lua_State, ci, pCallInfo_t);               // call info for current function
   DEFINE_FIELD(lua_State, oldpc, pInstruction);           // last pc traced
   DEFINE_FIELD(lua_State, stack_last, luaTypes.StkId);    // last free slot in the stack
   DEFINE_FIELD(lua_State, stack, luaTypes.StkId);         // stack base
   DEFINE_FIELD(lua_State, openupval, pUpVal_t);           // list of open upvalues in this stack
   DEFINE_FIELD(lua_State, gclist, pGCObject_t);
   DEFINE_FIELD(lua_State, twups, pLuaState_t);            // list of threads with open upvalues
   DEFINE_FIELD(lua_State, errorJmp, pLuaLongjmp);         // current error recover point
   DEFINE_FIELD(lua_State, base_ci, luaTypes.CallInfo);    // CallInfo for first level (C calling Lua)
   DEFINE_FIELD(lua_State, hook, lua_Hook_t);              // (should be `volatile` ?)
   DEFINE_FIELD_T(lua_State, errfunc);
   DEFINE_FIELD_T(lua_State, stacksize);
   DEFINE_FIELD_T(lua_State, basehookcount);
   DEFINE_FIELD_T(lua_State, hookcount);
   DEFINE_FIELD_T(lua_State, nny);                         // number of non-yieldable calls in stack
   DEFINE_FIELD_T(lua_State, nCcalls);                     // number of nested C calls
   DEFINE_FIELD_T(lua_State, hookmask);
   DEFINE_FIELD(lua_State, allowhook, luaTypes.lu_byte);
   CLOSE_STRUCT(lua_State);
}
