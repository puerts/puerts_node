/*
 * Tencent is pleased to support the open source community by making Puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * Puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

#include "pesapi.h"
#include <node.h>
#include "CppObjectMapper.h"
#include "JSClassRegister.h"

#ifdef WIN32
#include "win32-dlfcn.h"
#else
#include <dlfcn.h>
#endif

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <sstream>
#include <regex>

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#ifndef MSVC_PRAGMA
#if !defined(__clang__) && defined(_MSC_VER)
	#define MSVC_PRAGMA(Pragma) __pragma(Pragma)
#else
	#define MSVC_PRAGMA(...)
#endif
#endif

static std::map<std::string, void*> GHandlers;

MSVC_PRAGMA(warning(push))
MSVC_PRAGMA(warning(disable : 4191))
static pesapi_func_ptr funcs[] = {(pesapi_func_ptr) &pesapi_create_null, (pesapi_func_ptr) &pesapi_create_undefined,
    (pesapi_func_ptr) &pesapi_create_boolean, (pesapi_func_ptr) &pesapi_create_int32, (pesapi_func_ptr) &pesapi_create_uint32,
    (pesapi_func_ptr) &pesapi_create_int64, (pesapi_func_ptr) &pesapi_create_uint64, (pesapi_func_ptr) &pesapi_create_double,
    (pesapi_func_ptr) &pesapi_create_string_utf8, (pesapi_func_ptr) &pesapi_create_binary, (pesapi_func_ptr) &pesapi_get_value_bool,
    (pesapi_func_ptr) &pesapi_get_value_int32, (pesapi_func_ptr) &pesapi_get_value_uint32,
    (pesapi_func_ptr) &pesapi_get_value_int64, (pesapi_func_ptr) &pesapi_get_value_uint64,
    (pesapi_func_ptr) &pesapi_get_value_double, (pesapi_func_ptr) &pesapi_get_value_string_utf8,
    (pesapi_func_ptr) &pesapi_get_value_binary, (pesapi_func_ptr) &pesapi_is_null, (pesapi_func_ptr) &pesapi_is_undefined,
    (pesapi_func_ptr) &pesapi_is_boolean, (pesapi_func_ptr) &pesapi_is_int32, (pesapi_func_ptr) &pesapi_is_uint32,
    (pesapi_func_ptr) &pesapi_is_int64, (pesapi_func_ptr) &pesapi_is_uint64, (pesapi_func_ptr) &pesapi_is_double,
    (pesapi_func_ptr) &pesapi_is_string, (pesapi_func_ptr) &pesapi_is_object, (pesapi_func_ptr) &pesapi_is_function,
    (pesapi_func_ptr) &pesapi_is_binary, (pesapi_func_ptr) &pesapi_create_native_object,
    (pesapi_func_ptr) &pesapi_get_native_object_ptr, (pesapi_func_ptr) &pesapi_get_native_object_typeid,
    (pesapi_func_ptr) &pesapi_is_native_object, (pesapi_func_ptr) &pesapi_create_ref, (pesapi_func_ptr) &pesapi_get_value_ref,
    (pesapi_func_ptr) &pesapi_update_value_ref, (pesapi_func_ptr) &pesapi_is_ref, (pesapi_func_ptr) &pesapi_get_args_len,
    (pesapi_func_ptr) &pesapi_get_arg, (pesapi_func_ptr) &pesapi_get_env, (pesapi_func_ptr) &pesapi_get_this,
    (pesapi_func_ptr) &pesapi_get_holder, (pesapi_func_ptr) &pesapi_get_userdata,
    (pesapi_func_ptr) &pesapi_get_constructor_userdata, (pesapi_func_ptr) &pesapi_add_return,
    (pesapi_func_ptr) &pesapi_throw_by_string, (pesapi_func_ptr) &pesapi_hold_env, (pesapi_func_ptr) &pesapi_get_env_from_holder,
    (pesapi_func_ptr) &pesapi_duplicate_env_holder, (pesapi_func_ptr) &pesapi_release_env_holder,
    (pesapi_func_ptr) &pesapi_open_scope, (pesapi_func_ptr) &pesapi_has_caught, (pesapi_func_ptr) &pesapi_get_exception_as_string,
    (pesapi_func_ptr) &pesapi_close_scope, (pesapi_func_ptr) &pesapi_hold_value, (pesapi_func_ptr) &pesapi_duplicate_value_holder,
    (pesapi_func_ptr) &pesapi_release_value_holder, (pesapi_func_ptr) &pesapi_get_value_from_holder,
    (pesapi_func_ptr) &pesapi_get_property, (pesapi_func_ptr) &pesapi_set_property, (pesapi_func_ptr) &pesapi_get_property_uint32,
    (pesapi_func_ptr) &pesapi_set_property_uint32, (pesapi_func_ptr) &pesapi_call_function, (pesapi_func_ptr) &pesapi_eval,
    (pesapi_func_ptr) &pesapi_alloc_type_infos, (pesapi_func_ptr) &pesapi_set_type_info,
    (pesapi_func_ptr) &pesapi_create_signature_info, (pesapi_func_ptr) &pesapi_alloc_property_descriptors,
    (pesapi_func_ptr) &pesapi_set_method_info, (pesapi_func_ptr) &pesapi_set_property_info, (pesapi_func_ptr) &pesapi_define_class,
    (pesapi_func_ptr) &pesapi_class_type_info};
MSVC_PRAGMA(warning(pop))

EXTERN_C_START
int pesapi_load_addon(const char* path, const char* module_name)
{
    return -1;
}

extern const char* GPesapiModuleName;
EXTERN_C_END

static void ThrowException(v8::Isolate* Isolate, const char* Message)
{
    auto ExceptionStr = v8::String::NewFromUtf8(Isolate, Message, v8::NewStringType::kNormal).ToLocalChecked();
    Isolate->ThrowException(v8::Exception::Error(ExceptionStr));
}

struct DeclarationGenerator
{
    std::map<std::string, std::vector<std::string>> module_to_classes;
    
    void GenArguments(const puerts::CFunctionInfo* Type, std::stringstream& ss)
    {
        for (unsigned int i = 0; i < Type->ArgumentCount(); i++)
        {
            if (i != 0)
                ss << ", ";
            auto argInfo = Type->Argument(i);

            ss << "p" << i;

            if (i >= Type->ArgumentCount() - Type->DefaultCount())
            {
                ss << "?";
            }

            ss << ": ";

            if (strcmp(argInfo->Name(), "cstring") != 0 && !argInfo->IsUEType() && !argInfo->IsObjectType() && argInfo->IsPointer())
            {
                ss << "ArrayBuffer";
            }
            else
            {
                bool IsReference = argInfo->IsRef();
                bool IsNullable = !IsReference && argInfo->IsPointer();
                if (IsNullable)
                {
                    ss << "$Nullable<";
                }
                if (IsReference)
                {
                    ss << "$Ref<";
                }

                const puerts::CTypeInfo* TypeInfo = Type->Argument(i);
                ss << TypeInfo->Name();

                if (IsNullable)
                {
                    ss << ">";
                }
                if (IsReference)
                {
                    ss << ">";
                }
            }
        }
    }
    
    void GenClassName(const char* name, std::stringstream& ss)
    {
        const char *pp = strchr(name, '.');
        while(pp)
        {
            const char* tmp = strchr(pp + 1, '.');
            if (tmp)
            {
                pp = tmp;
            }
            else //no next .
            {
                break;
            }
        }
        if (pp)
        {
            ss << (pp + 1);
        }
        else
        {
            ss << name;
        }
    }
    
    void GenClass(const puerts::JSClassDefinition* ClassDefinition)
    {
        std::stringstream Output;
        
        Output << "    class " ;
        GenClassName(ClassDefinition->ScriptName, Output);
        if (ClassDefinition->SuperTypeId)
        {
            Output << " extends ";
            GenClassName(puerts::FindClassByID(ClassDefinition->SuperTypeId)->ScriptName, Output);
        }
        Output << " {\n";

        std::set<std::string> AddedFunctions;

        puerts::NamedFunctionInfo* ConstructorInfo = ClassDefinition->ConstructorInfos;
        while (ConstructorInfo && ConstructorInfo->Name && ConstructorInfo->Type)
        {
            std::stringstream Tmp;
            Tmp << "        constructor(";
            GenArguments(ConstructorInfo->Type, Tmp);
            Tmp << ");\n";
            if (AddedFunctions.find(Tmp.str()) == AddedFunctions.end())
            {
                AddedFunctions.emplace(Tmp.str());
                Output << Tmp.str();
            }
            ++ConstructorInfo;
        }

        puerts::NamedPropertyInfo* PropertyInfo = ClassDefinition->PropertyInfos;
        while (PropertyInfo && PropertyInfo->Name && PropertyInfo->Type)
        {
            Output << "        " << PropertyInfo->Name << ": " << PropertyInfo->Type->Name()
                   << ";\n";
            ++PropertyInfo;
        }

        puerts::NamedPropertyInfo* VariableInfo = ClassDefinition->VariableInfos;
        while (VariableInfo && VariableInfo->Name && VariableInfo->Type)
        {
            int Pos = VariableInfo - ClassDefinition->VariableInfos;
            Output << "        static " << (ClassDefinition->Variables[Pos].Setter ? "" : "readonly ") << VariableInfo->Name << ": "
                   << VariableInfo->Type->Name() << ";\n";
            ++VariableInfo;
        }

        puerts::NamedFunctionInfo* FunctionInfo = ClassDefinition->FunctionInfos;
        while (FunctionInfo && FunctionInfo->Name && FunctionInfo->Type)
        {
            std::stringstream Tmp;
            Tmp << "        static " << FunctionInfo->Name;
            if (FunctionInfo->Type->Return())
            {
                Tmp << "(";
                GenArguments(FunctionInfo->Type, Tmp);
                const puerts::CTypeInfo* ReturnType = FunctionInfo->Type->Return();
                Tmp << ") :" << ReturnType->Name() << ";\n";
            }
            else
            {
                Tmp << FunctionInfo->Type->CustomSignature() << ";\n";
            }
            if (AddedFunctions.find(Tmp.str()) == AddedFunctions.end())
            {
                AddedFunctions.emplace(Tmp.str());
                Output << Tmp.str();
            }
            ++FunctionInfo;
        }

        puerts::NamedFunctionInfo* MethodInfo = ClassDefinition->MethodInfos;
        while (MethodInfo && MethodInfo->Name && MethodInfo->Type)
        {
            std::stringstream Tmp;
            Tmp << "        " << MethodInfo->Name;
            if (MethodInfo->Type->Return())
            {
                Tmp << "(";
                GenArguments(MethodInfo->Type, Tmp);
                const puerts::CTypeInfo* ReturnType = MethodInfo->Type->Return();
                Tmp << ") :" << ReturnType->Name() << ";\n";
            }
            else
            {
                Tmp << MethodInfo->Type->CustomSignature() << ";\n";
            }
            if (AddedFunctions.find(Tmp.str()) == AddedFunctions.end())
            {
                AddedFunctions.emplace(Tmp.str());
                Output << Tmp.str();
            }
            ++MethodInfo;
        }

        Output << "    }\n\n";
        
        std::string moudle_name(ClassDefinition->ScriptName, strchr(ClassDefinition->ScriptName, '.') - ClassDefinition->ScriptName);
        
        if (module_to_classes.find(moudle_name) == module_to_classes.end())
        {
            module_to_classes[moudle_name] = std::vector<std::string>();
        }
        
        module_to_classes[moudle_name].push_back(Output.str());
        
        //std::cout << moudle_name << "_" << Output.str();
    }
    
    std::string GetOutput()
    {
        std::stringstream Output;
        for (const auto & pair : module_to_classes) {
            Output << "declare module \"" << pair.first << "\" {\n";
            Output << "    import {$Ref, $Nullable, cstring} from \"puerts\"\n\n";
            for (int i = 0; i < pair.second.size(); i++) {
                Output << pair.second[i];
            }
            Output << "}\n";
        }
        return Output.str();
    }
};

NODE_MODULE_INIT(/* exports, module, context */) {
    auto isolate = context->GetIsolate();

    //TODO: 多addon有两个问题要解决，一个是目前注册JSClassRegister注册数据是放在静态变量，多addon每个addon一份并不共享，其次是FCppObjectMapper如何保持不冲突
    puerts::FCppObjectMapper *cppObjectMapper = new puerts::FCppObjectMapper();
    cppObjectMapper->Initialize(isolate, context);
    isolate->SetData(MAPPER_ISOLATE_DATA_POS, static_cast<puerts::ICppObjectMapper*>(cppObjectMapper));
      
    exports->Set(context, v8::String::NewFromUtf8(isolate, "load").ToLocalChecked(), v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value>& info) {
        if (!info[0]->IsString()) {
            ThrowException(info.GetIsolate(), "#0 argument expect a string");
            return;
        }
        std::string path = *(v8::String::Utf8Value(info.GetIsolate(), info[0]));

        if (GHandlers.find(path) != GHandlers.end()) {
            ThrowException(info.GetIsolate(), "loaded!");
            return;
        }

        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            std::stringstream ss;
            ss << "dlopen fail for " << path << ", error: " << dlerror();
            auto str = ss.str();
            ThrowException(info.GetIsolate(), str.c_str());
            return;
        }
        
        std::string EntryName = STRINGIFY(PESAPI_MODULE_INITIALIZER(dynamic));
        auto Init = (const char* (*)(pesapi_func_ptr*))(uintptr_t)dlsym(handle, EntryName.c_str()); 

        if (Init)  {
            const char* mn = Init(nullptr);
            GPesapiModuleName = mn;
            Init(funcs);
            GPesapiModuleName = nullptr;
            GHandlers[path] = handle;
            info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), mn, v8::NewStringType::kNormal).ToLocalChecked());
        } else {
            ThrowException(info.GetIsolate(), "can find entry");
            dlclose(handle);
        }
    })->GetFunction(context).ToLocalChecked()).Check();

    exports->Set(context, v8::String::NewFromUtf8(isolate, "loadType").ToLocalChecked(), v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value>& info) {
        auto pom = static_cast<puerts::FCppObjectMapper*>((v8::Local<v8::External>::Cast(info.Data()))->Value());
        pom->LoadCppType(info);
    }, v8::External::New(isolate, cppObjectMapper))->GetFunction(context).ToLocalChecked()).Check();

    exports->Set(context, v8::String::NewFromUtf8(isolate, "declaration").ToLocalChecked(), v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value>& info) {
        DeclarationGenerator dg;
        
        puerts::ForeachRegisterClass(
            [&](const puerts::JSClassDefinition* ClassDefinition)
            {
                if (ClassDefinition->TypeId && ClassDefinition->ScriptName)
                {
                    dg.GenClass(ClassDefinition);
                }
            });
            
        //std::cout << ;
        info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), dg.GetOutput().c_str(), v8::NewStringType::kNormal).ToLocalChecked());
    })->GetFunction(context).ToLocalChecked()).Check();
}
