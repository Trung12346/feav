rmdir /s /q dependencies
mkdir dependencies
mkdir dependencies\cglm
mkdir dependencies\glfw
mkdir dependencies\ryu

git clone https://github.com/recp/cglm dependencies\cglm
git clone https://github.com/glfw/glfw dependencies\glfw
git clone https://github.com/ulfjack/ryu dependencies\ryu

rustup target add x86_64-pc-windows-gnu
cargo build --release --target x86_64-pc-windows-gnu --manifest-path include\layout_engine\Cargo.toml

cmake -S dependencies\glfw -B dependencies\glfw\build -G "MinGW Makefiles"
cmake --build dependencies\glfw\build

cmake -S dependencies\cglm -B dependencies\cglm\build -G "MinGW Makefiles"
cmake --build dependencies\cglm\build

rmdir /s /q build

mkdir build

gcc -g -std=c11 -march=native -pthread ^
src\main.c ^
dependencies\ryu\ryu\s2f.c ^
dependencies\ryu\ryu\s2d.c ^
-I include ^
-I %VULKAN_SDK%\Include ^
-I dependencies\cglm\include ^
-I dependencies\glfw\include ^
-I dependencies\ryu ^
-L dependencies\cglm\build ^
-L dependencies\glfw\build\src ^
-L include\layout_engine\target\x86_64-pc-windows-gnu\release ^
-L %VULKAN_SDK%\Lib ^
-l glfw3 ^
-l cglm.dll ^
-l gdi32 ^
-l vulkan-1 ^
-l feav_layout ^
-l ntdll ^
-l ws2_32 ^
-l userenv ^
-l bcrypt ^
-l advapi32 ^
-l kernel32 ^
-o build/main.exe