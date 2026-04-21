/*
 * Tencent is pleased to support the open source community by making Puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * Puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

#pragma once

#include "Binding.hpp"
#include "pesapi.h"
#include <iostream>

namespace PUERTS_NAMESPACE
{
namespace internal
{
class AutoValueScope
{
public:
    AutoValueScope(struct pesapi_ffi* apis, pesapi_env_ref env_holder) : apis_(apis)
    {
        scope = apis_->open_scope(env_holder);
    }

    ~AutoValueScope()
    {
        apis_->close_scope(scope);
    }

    struct pesapi_ffi* apis_;
    pesapi_scope scope;
};
}    // namespace internal
namespace pesapi_impl
{
static inline void REPORT_EXCEPTION(const char* MSG)
{
    std::cout << "call function throw: " << MSG << std::endl;
}

class Object
{
public:
    Object(PApiCallbackEnv env, pesapi_value value) : apis_(env.apis)
    {
        env_holder = apis_->create_env_ref(env.env);
        value_holder = apis_->create_value_ref(env.env, value, 0);
    }

    Object(const Object& InOther) : apis_(InOther.apis_)
    {
        env_holder = apis_->duplicate_env_ref(InOther.env_holder);
        value_holder = apis_->duplicate_value_ref(InOther.value_holder);
    }

    ~Object()
    {
        if (apis_)
        {
            apis_->release_value_ref(value_holder);
            apis_->release_env_ref(env_holder);
        }
    }

    template <typename T>
    T Get(const char* key) const
    {
        internal::AutoValueScope ValueScope(apis_, env_holder);
        auto env = apis_->get_env_from_ref(env_holder);
        auto object = apis_->get_value_from_ref(env, value_holder);

        auto value = apis_->get_property(env, object, key);
        if (!apis_->is_undefined(env, value))
        {
            return pesapi_impl::Converter<T>::toCpp(PApiCallbackEnv{apis_, env}, value);
        }
        return {};
    }

    template <typename T>
    void Set(const char* key, T val) const
    {
        internal::AutoValueScope ValueScope(apis_, env_holder);
        auto env = apis_->get_env_from_ref(env_holder);
        auto object = apis_->get_value_from_ref(env, value_holder);

        apis_->set_property(env, object, key, pesapi_impl::Converter<T>::toScript(PApiCallbackEnv{apis_, env}, val));
    }

    bool IsValid() const
    {
        internal::AutoValueScope ValueScope(apis_, env_holder);
        auto env = apis_->get_env_from_ref(env_holder);
        auto val = apis_->get_value_from_ref(env, value_holder);
        return val && apis_->is_object(env, val);
    }

    void operator=(const Object& obj)
    {
        apis_ = obj.apis_;
        env_holder = apis_->duplicate_env_ref(obj.env_holder);
        value_holder = apis_->duplicate_value_ref(obj.value_holder);
    }

    struct pesapi_ffi* apis_;
    pesapi_env_ref env_holder;
    pesapi_value_ref value_holder;

    friend struct pesapi_impl::Converter<Object>;
};

class Function : public Object
{
public:
    Function(PApiCallbackEnv env, pesapi_value value) : Object(env, value)
    {
    }

    template <typename... Args>
    void Action(Args... cppArgs) const
    {
        internal::AutoValueScope ValueScope(apis_, env_holder);
        auto env = apis_->get_env_from_ref(env_holder);
        auto object = apis_->get_value_from_ref(env, value_holder);

        auto _un_used = invokeHelper(env, object, cppArgs...);

        if (apis_->has_caught(ValueScope.scope))
        {
            REPORT_EXCEPTION(apis_->get_exception_as_string(ValueScope.scope, true));
        }
    }

    template <typename Ret, typename... Args>
    Ret Func(Args... cppArgs) const
    {
        internal::AutoValueScope ValueScope(apis_, env_holder);
        auto env = apis_->get_env_from_ref(env_holder);
        auto object = apis_->get_value_from_ref(env, value_holder);

        auto ret = invokeHelper(env, object, cppArgs...);

        if (apis_->has_caught(ValueScope.scope))
        {
            REPORT_EXCEPTION(apis_->get_exception_as_string(ValueScope.scope, true));
            return {};
        }
        else
        {
            return pesapi_impl::Converter<Ret>::toCpp(PApiCallbackEnv{apis_, env}, ret);
        }
    }

    bool IsValid() const
    {
        internal::AutoValueScope ValueScope(apis_, env_holder);
        auto env = apis_->get_env_from_ref(env_holder);
        auto val = apis_->get_value_from_ref(env, value_holder);
        return val && apis_->is_function(env, val);
    }

    template <typename T>
    void SetWeakAndOwnBy(const T* Owner)
    {
        internal::AutoValueScope ValueScope(apis_, env_holder);
        auto env = apis_->get_env_from_ref(env_holder);

        auto owner = apis_->native_object_to_value(env, StaticTypeId<T>::get(), Owner, false);
        auto val = apis_->get_value_from_ref(env, value_holder);
        if (apis_->set_owner(env, val, owner))
        {
            apis_->set_ref_weak(env, value_holder);
        }
    }

private:
    template <typename... Args>
    auto invokeHelper(pesapi_env env, pesapi_value func, Args... CppArgs) const
    {
        pesapi_value argv[sizeof...(Args)]{pesapi_impl::Converter<Args>::toScript(PApiCallbackEnv{apis_, env}, CppArgs)...};
        return apis_->call_function(env, func, nullptr, sizeof...(Args), argv);
    }

    auto invokeHelper(pesapi_env env, pesapi_value func) const
    {
        return apis_->call_function(env, func, nullptr, 0, nullptr);
    }

    friend struct pesapi_impl::Converter<Function>;
};

}    // namespace pesapi_impl

template <>
struct ScriptTypeName<pesapi_impl::Object>
{
    static constexpr auto value()
    {
        return internal::Literal("object");
    }
};

template <>
struct ScriptTypeName<pesapi_impl::Function>
{
    static constexpr auto value()
    {
        return internal::Literal("Function");
    }
};

namespace pesapi_impl
{
template <>
struct Converter<Object>
{
    static pesapi_value toScript(PApiCallbackEnv env, Object value)
    {
        return env.apis->get_value_from_ref(env.env, value.value_holder);
    }

    static Object toCpp(PApiCallbackEnv env, pesapi_value value)
    {
        return Object(env, value);
    }

    static bool accept(PApiCallbackEnv env, pesapi_value value)
    {
        return env.apis->is_object(env.env, value);
    }
};

template <>
struct Converter<Function>
{
    static pesapi_value toScript(PApiCallbackEnv env, Function value)
    {
        return env.apis->get_value_from_ref(env.env, value.value_holder);
    }

    static Function toCpp(PApiCallbackEnv env, pesapi_value value)
    {
        return Function(env, value);
    }

    static bool accept(PApiCallbackEnv env, pesapi_value value)
    {
        return env.apis->is_function(env.env, value);
    }
};

#include "StdFunctionConverter.hpp"
}    // namespace pesapi_impl

}    // namespace PUERTS_NAMESPACE
