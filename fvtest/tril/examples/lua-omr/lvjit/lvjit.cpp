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

// Lua Vermelha headers
#include "luavm.hpp"
#include "LuaTypeDictionary.hpp"
#include "LuaFunctionBuilder.hpp"

// JitBuilder headers
#include "Jit.hpp"

// OMR headers
#include "ilgen/TypeDictionary.hpp"

typedef int32_t (IncOrDecFunction)(int32_t*);

extern bool internal_initializeJit();
extern int32_t internal_compileMethodBuilder(TR::MethodBuilder * methodBuilder, void ** entryPoint);
extern void internal_shutdownJit();

int luaJ_initJit() {
   return internal_initializeJit() ? 0 : 1;
}

void luaJ_stopJit() {
   internal_shutdownJit();
}

lua_JitFunction luaJ_invokejit(Proto* p) {
   Lua::TypeDictionary types;
   uint8_t* entry = nullptr;
   Lua::FunctionBuilder f(p, &types);

   uint32_t rc = internal_compileMethodBuilder(&f, (void**)&entry);

   if (rc == 0) {
      return (lua_JitFunction)entry;
   }
   else {
      return nullptr;
   }
}

int luaJ_compile(Proto* p) {
   lua_JitFunction f = luaJ_invokejit(p);

   if (f) {
      p->compiledcode = f;
      return 1;
   }
   else {
      p->compiledcode = NULL;
      return 0;
   }
}

unsigned int luaJ_initcallcounter() {
   return 100;
}
