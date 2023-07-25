# puerts_node

export your c++ API for node

## Installation

You can install `puerts_node` using `npm`:

``` bash
npm install -g puerts_node
```

## Example

### C++ code to be call

``` c++
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
```

### Export C++ API

``` c++
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

//hello_world is module name, will use in js later.
PESAPI_MODULE(hello_world, Init)
```

ps: Above C++ example project can be generate by below command line:

``` bash
puerts_node init hello_world
```

Compile the generated addon project:

``` bash
cd hello_world
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Calling addon in JavaScript

``` javascript
const puerts_node = require("puerts_node");

let hello_world = puerts_node('path/to/hello_world');
const HelloWorld = hello_world.HelloWorld;

const obj = new HelloWorld(101);
obj.Foo((x, y) => x > y);

HelloWorld.Bar("hello");

HelloWorld.StaticField = 999;
obj.Field = 888;
obj.Foo((x, y) => x > y);
```

## TypeScript support

### Generate typescript declaration file (index.d.ts) for a puerts addon

``` bash
puerts_node gen_dts path\to\hello_world -t typing
```

Add typing directory to tsconfig.json/compilerOptions/typeRoots.

### Calling addon in TypeScript

``` typescript
import {load} from "puerts_node";
import * as HelloWorldModlue from 'hello_world'

let hello_world = load<typeof HelloWorldModlue>('path/to/hello_world');

const HelloWorld = hello_world.HelloWorld;

const obj = new HelloWorld(101);

obj.Foo((x, y) => x > y);

HelloWorld.Bar("hello");

HelloWorld.StaticField = 999;
obj.Field = 888;

obj.Foo((x, y) => x > y);

```

## Features

The following C++ features are supported: constructors, inheritance, member variables, member functions, static functions, static variables, function overloading (constructors/static functions/member functions)

Supports the generation of TypeScript declarations

For more examples, see: https://github.com/puerts/puerts_node/tree/master/test
