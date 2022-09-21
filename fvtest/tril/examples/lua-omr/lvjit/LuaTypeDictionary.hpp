/*******************************************************************************
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

#ifndef LUATYPEDICTIONARY_HPP
#define LUATYPEDICTIONARY_HPP

// JitBuilder headers
#include "Jit.hpp"
#include "ilgen/TypeDictionary.hpp"

// Lua headers
#include "luavm.hpp"

namespace Lua { class TypeDictionary; }

/*
** A TypeDictionary for defining JitBuilder representations of Lua VM types
*/
class Lua::TypeDictionary : public TR::TypeDictionary {
public:

   // struct for caching JitBuilder representations of commonly used VM types
   struct LuaTypes {
      TR::IlType* lu_byte;
      TR::IlType* lua_Integer;
      TR::IlType* lua_Unsigned;
      TR::IlType* lua_Number;
      TR::IlType* Instruction;
      TR::IlType* Value;
      TR::IlType* TString;
      TR::IlType* p_TString;
      TR::IlType* TValue;
      TR::IlType* StkId;
      TR::IlType* Proto;
      TR::IlType* LClosure;
      TR::IlType* UpVal;
      TR::IlType* CallInfo;
      TR::IlType* lua_State;
      TR::IlType* TMS;
   };
   
   TypeDictionary();

   LuaTypes getLuaTypes() { return luaTypes; }

   //https://github.com/eclipse/omr/blob/381ec5b6fb/compiler/ilgen/TypeDictionary.hpp
   template <typename T>
     struct is_supported {
        static const bool value =  std::is_arithmetic<T>::value // note: is_arithmetic = is_integral || is_floating_point
                                || std::is_void<T>::value;
     };
     template <typename T>
     struct is_supported<T*> {
        // a pointer type is supported iff the type being pointed to is supported
        static const bool value =  is_supported<T>::value;
     };

   // integral
     template <typename T>
     TR::IlType* toIlType(typename std::enable_if<std::is_integral<T>::value && (sizeof(T) == 1)>::type* = 0) { return Int8; }
     template <typename T>
     TR::IlType* toIlType(typename std::enable_if<std::is_integral<T>::value && (sizeof(T) == 2)>::type* = 0) { return Int16; }
     template <typename T>
     TR::IlType* toIlType(typename std::enable_if<std::is_integral<T>::value && (sizeof(T) == 4)>::type* = 0) { return Int32; }
     template <typename T>
     TR::IlType* toIlType(typename std::enable_if<std::is_integral<T>::value && (sizeof(T) == 8)>::type* = 0) { return Int64; }

     // floating point
     template <typename T>
     TR::IlType* toIlType(typename std::enable_if<std::is_floating_point<T>::value && (sizeof(T) == 4)>::type* = 0) { return Float; }
     template <typename T>
     TR::IlType* toIlType(typename std::enable_if<std::is_floating_point<T>::value && (sizeof(T) == 8)>::type* = 0) { return Double; }

     // void
     template <typename T>
     TR::IlType* toIlType(typename std::enable_if<std::is_void<T>::value>::type* = 0) { return NoType; }

     // pointer
     template <typename T>
     TR::IlType* toIlType(typename std::enable_if<std::is_pointer<T>::value && is_supported<typename std::remove_pointer<T>::type>::value>::type* = 0) {
        return PointerTo(toIlType<typename std::remove_pointer<T>::type>());
     }


private:
   LuaTypes luaTypes;
};

#endif // LUATYPEDICTIONARY_HPP
