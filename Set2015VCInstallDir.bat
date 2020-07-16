pushd "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\"
call vcvarsall.bat
popd
md build
md bin
md bin\Plugin
cd build
md Plugin
cmake -G "Visual Studio 14 2015" ..
cmake --build .