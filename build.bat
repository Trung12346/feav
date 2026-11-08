del build\main.exe

cargo build --release --target x86_64-pc-windows-gnu --manifest-path include\layout_engine\Cargo.toml

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