rmdir /s /q dependencies
mkdir dependencies
mkdir dependencies\cglm
mkdir dependencies\glfw
mkdir dependencies\ryu

git clone https://github.com/recp/cglm dependencies\cglm
git clone https://github.com/glfw/glfw dependencies\glfw
git clone https://github.com/ulfjack/ryu dependencies\ryu

cmake -S dependencies\glfw -B dependencies\glfw\build -G "MinGW Makefiles"
cmake --build dependencies\glfw\build

cmake -S dependencies\cglm -B dependencies\cglm\build -G "MinGW Makefiles"
cmake --build dependencies\cglm\build

rmdir /s /q build

mkdir build

gcc -g -std=c11 -march=native -pthread -Wall -Wextra ^
src\main.c ^
-I include ^
-I C:\VulkanSDK\1.4.357.0\Include ^
-I dependencies\cglm\include ^
-I dependencies\glfw\include ^
-I dependencies\ryu\ryu ^
-L dependencies\cglm\build ^
-L dependencies\glfw\build\src ^
-L C:\VulkanSDK\1.4.357.0\Lib ^
-l glfw3 ^
-l cglm.dll ^
-l gdi32 ^
-l vulkan-1 ^
-o build/main.exe