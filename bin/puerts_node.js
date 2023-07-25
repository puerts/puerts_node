#!/usr/bin/env node

const program = require('commander');
const puerts_node = require('..');
const fs = require('fs');
const path = require('path');

function copyFileSync( source, target ) {
    var targetFile = target;

    if ( fs.existsSync( target ) ) {
        if ( fs.lstatSync( target ).isDirectory() ) {
            targetFile = path.join( target, path.basename( source ) );
        }
    }

    fs.writeFileSync(targetFile, fs.readFileSync(source));
}

function copyFolderRecursiveSync( source, target ) {
    var files = [];

    var targetFolder = path.join( target, path.basename( source ) );
    if ( !fs.existsSync( targetFolder ) ) {
        fs.mkdirSync( targetFolder );
    }

    if ( fs.lstatSync( source ).isDirectory() ) {
        files = fs.readdirSync( source );
        files.forEach( function ( file ) {
            var curSource = path.join( source, file );
            if ( fs.lstatSync( curSource ).isDirectory() ) {
                copyFolderRecursiveSync( curSource, targetFolder );
            } else {
                copyFileSync( curSource, targetFolder );
            }
        } );
    }
}

function gen_cmakelist(project_name) {
    return `
cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 14)

project (${project_name})

set(PUERTS_LIB_ROOT puerts_libs)
set(PUERTS_INCLUDE \${PUERTS_LIB_ROOT}/include)
set(PUERTS_SRC \${PUERTS_LIB_ROOT}/src)

file(GLOB SOURCE_FILES "src/hello_world.cc")

include_directories(
    \${PUERTS_INCLUDE}
    "src"
)

add_library(\${PROJECT_NAME} SHARED \${SOURCE_FILES} \${PUERTS_SRC}/pesapi_adpt.c)

target_compile_definitions (\${PROJECT_NAME} PRIVATE BUILDING_PES_EXTENSION) 

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

void Init() {
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
        copyFolderRecursiveSync(path.join(__dirname, '..', 'puerts_libs'), project_name);
        fs.writeFileSync(path.join(project_name, 'CMakeLists.txt'), gen_cmakelist(project_name), {encoding: "utf8", flag: "w"});
        fs.writeFileSync(path.join(project_name, 'src', 'hello_world.cc'), gen_helloworld_cc(project_name), {encoding: "utf8", flag: "w"});
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
