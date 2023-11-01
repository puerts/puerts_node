pushd ..
call npm install .
popd
pushd cpp
mkdir build64  & pushd build64
cmake -G "Visual Studio 16 2019" -A x64 ..
popd
cmake --build build64 --config Release
copy build64\Release\hello.dll hello.dll
popd
node test.js
