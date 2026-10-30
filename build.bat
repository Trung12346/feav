del build\main.exe

gcc -g -std=c11 -march=native -pthread ^
src\main.c ^
dependencies\ryu\ryu\s2f.c ^
dependencies\ryu\ryu\s2d.c ^
-I include ^
-I C:\VulkanSDK\1.4.357.0\Include ^
-I dependencies\cglm\include ^
-I dependencies\glfw\include ^
-I dependencies\ryu ^
-L dependencies\cglm\build ^
-L dependencies\glfw\build\src ^
-L C:\VulkanSDK\1.4.357.0\Lib ^
-l glfw3 ^
-l cglm.dll ^
-l gdi32 ^
-l vulkan-1 ^
-o build/main.exe