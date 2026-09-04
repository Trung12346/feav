rmdir /s /q dependencies
mkdir dependencies
mkdir dependencies\cglm
mkdir dependencies\glfw

git clone https://github.com/recp/cglm dependencies\cglm
git clone https://github.com/glfw/glfw dependencies\glfw

cmake -S dependencies\glfw -B dependencies\glfw\build -G "MinGW Makefiles"
cmake --build dependencies\glfw\build

cmake -S dependencies\cglm -B dependencies\cglm\build -G "MinGW Makefiles"
cmake --build dependencies\cglm\build

rmdir /s /q build

mkdir build

gcc -g -std=c99 ^
src\main.c ^
-I include ^
-I C:\VulkanSDK\1.4.357.0\Include ^
-I dependencies\cglm\include ^
-I dependencies\glfw\include ^
-L dependencies\cglm\build ^
-L dependencies\glfw\build\src ^
-L C:\VulkanSDK\1.4.357.0\Lib ^
-l glfw3 ^
-l cglm.dll ^
-l gdi32 ^
-l vulkan-1 ^
-o build/main.exe