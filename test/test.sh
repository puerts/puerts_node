cd ..
npm install .
cd -
cd  cpp
mkdir build
cd build
cmake ..
make
cd ../..
node test.js
