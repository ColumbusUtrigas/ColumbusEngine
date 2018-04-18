#pragma once

#include <Core/Templates/TypeTraits.h>

namespace Columbus
{

	template<typename>
	struct IsFunction : FalseType { };
	 
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...)> : TrueType {};
	 
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......)> : TrueType {};
	 
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) const> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) volatile> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) const volatile> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) const> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) volatile> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) const volatile> : TrueType {};
	 
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) &> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) const &> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) volatile &> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) const volatile &> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) &> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) const &> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) volatile &> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) const volatile &> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) &&> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) const &&> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) volatile &&> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args...) const volatile &&> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) &&> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) const &&> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) volatile &&> : TrueType {};
	template<typename Ret, typename... Args>
	struct IsFunction<Ret(Args......) const volatile &&> : TrueType {};

}












