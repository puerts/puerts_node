#!/usr/bin/env node

const program = require('commander');
const puerts_node = require('..');
const fs = require('fs');
const path = require('path');

function gen_cmakelist(project_name) {
    return `
cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 14)

project (${project_name})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if ( NOT WIN32 OR CYGWIN )
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif ( )

execute_process(
    COMMAND node -p "require('puerts').include_dir"
    OUTPUT_VARIABLE PUERTS_INCLUDE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY \${CMAKE_CURRENT_SOURCE_DIR}
)
string(REPLACE "\\\\" "/" PUERTS_INCLUDE \${PUERTS_INCLUDE})

execute_process(
    COMMAND node -p "require('puerts').src_dir"
    OUTPUT_VARIABLE PUERTS_SRC
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY \${CMAKE_CURRENT_SOURCE_DIR}
)
string(REPLACE "\\\\" "/" PUERTS_SRC \${PUERTS_SRC})

file(GLOB SOURCE_FILES "src/${project_name}.cc")

include_directories(
    \${PUERTS_INCLUDE}
    "src"
)

add_library(\${PROJECT_NAME} SHARED \${SOURCE_FILES} \${PUERTS_SRC}/pesapi_adpt.c)

target_compile_definitions (\${PROJECT_NAME} PRIVATE BUILDING_PES_EXTENSION) 
set_target_properties(\${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY \${PROJECT_SOURCE_DIR})

`;
}

function gen_helloworld_cc(project_name) {
    return `#include <functional>
#include <string>
#include <iostream>
#include "Binding.hpp"

class HelloWorld
{
public:
    HelloWorld(int p) {
        Field = p;
    }

    void Foo(std::function<bool(int, int)> cmp) {
        bool ret = cmp(Field, StaticField);
        std::cout << "Foo, Field: " << Field << ", StaticField: " << StaticField << ", compare result:" << ret << std::endl;
    }
    
    static int Bar(std::string str) {
        std::cout << "Bar, str:" << str << std::endl;
        return  StaticField + 1;
    }
    
    int Field;
    
    static int StaticField;
};

int HelloWorld::StaticField = 0;

// module declaration begin

UsingCppType(HelloWorld);

static void Init() {
    puerts::DefineClass<HelloWorld>()
        .Constructor<int>()
        .Method("Foo", MakeFunction(&HelloWorld::Foo))
        .Function("Bar", MakeFunction(&HelloWorld::Bar))
        .Property("Field", MakeProperty(&HelloWorld::Field))
        .Variable("StaticField", MakeVariable(&HelloWorld::StaticField))
        .Register();
}

PESAPI_MODULE(${project_name}, Init)

// module declaration end
`
}

program
	.command('init <project_name>')
    .description('init a puerts addon project')
	.action(function (project_name) {
        fs.mkdirSync(path.join(project_name, 'src'), { recursive: true });
        fs.writeFileSync(path.join(project_name, 'CMakeLists.txt'), gen_cmakelist(project_name), {encoding: "utf8", flag: "w"});
        fs.writeFileSync(path.join(project_name, 'src', `${project_name}.cc`), gen_helloworld_cc(project_name), {encoding: "utf8", flag: "w"});
        console.log(`${project_name} inited`);
	})

program
	.command('gen_dts <dll_file>')
    .description('generate typescript declaration file (index.d.ts) for a puerts addon')
    .option('-t, --type_root <type_root>', 'type root for typescript project')
	.action(function (dll_file, opt) {
		console.log('gen_dts ' + dll_file, opt.type_root);
        const module = puerts_node.load(dll_file);
        console.log(`${dll_file} loaded`);
        const dts_file_dir = path.join(opt.type_root, module.__name);
        fs.mkdirSync(dts_file_dir, { recursive: true });
        const dts_file_path = path.join(dts_file_dir, 'index.d.ts');
        fs.writeFileSync(dts_file_path, puerts_node.declaration(), {encoding: "utf8", flag: "w"});
        console.log(`${dts_file_path} written`);
	})

program.parse(process.argv);
