const puerts_node = require('..');
const {$ref, $unref} = puerts_node;

const hello = puerts_node.load("hello");

const TestClass = hello["TestClass"];

//static function
console.log(TestClass.Add(12, 34));

TestClass.Overload();
TestClass.Overload(1);
TestClass.Overload(1, 2);
TestClass.Overload("hello", 2);

try {
    TestClass.Overload("hello");
} catch {
    console.log("caught function overload error");
}

let obj = new TestClass();
obj = new TestClass(8, 9);

//method
obj.OverloadMethod()
obj.OverloadMethod(1024)
obj.OverloadMethod(4294967295)
obj.OverloadMethod(1024n)

try {
    obj.OverloadMethod("hello");
} catch {
    console.log("caught method overload error");
}

//property
console.log(obj.X, obj.Y)
obj.X = 96
obj.Y = 97
console.log(obj.X, obj.Y)
obj.OverloadMethod()

//base method
obj.Foo(888);

//pass object
TestClass.PrintInfo(obj.GetSelf())

//static variable
console.log(TestClass.StaticInt)
TestClass.StaticInt = 789
console.log(TestClass.StaticInt)
TestClass.PrintInfo(obj.GetSelf())

//ref & pointer
let r = $ref(999);
let ret = obj.Ref(r);
console.log("$unref:" + $unref(r) + ", ret:" + ret);
obj.ConstRef(999);

r = $ref(888);
ret = obj.Ptr(r);
console.log("$unref:" + $unref(r) + ", ret:" + ret);

let sr = $ref("ts msg for ref");
obj.StrRef(sr);
console.log("$unref<string>:" + $unref(sr));

console.log(obj.CStr("ts str"));

sr = $ref("ts msg for ptr");
obj.StrPtr(sr);
console.log("$unref<string>:" + $unref(sr));

//js object
const AdvanceTestClass = hello["AdvanceTestClass"];
let obj2  = new AdvanceTestClass(100);

obj2.StdFunctionTest((x, y) => {
    console.log('x=' + x + ",y=" + y);
    return x + y;
})

try {
    new (AdvanceTestClass)(1, 2, 3);
} catch (e) {
    console.log("caught constructor overload error");
}

console.log(puerts_node.declaration());
