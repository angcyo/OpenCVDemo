
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

#指定需要CMAKE的最小版本 https://blog.csdn.net/b2259909/article/details/58591898
cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("opencv_bmp")

#设置生成的so动态库最后输出的路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../outLibs/${ANDROID_ABI})

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

#引入文件夹 https://juejin.cn/post/6844903906099593229
include_directories(include)

#添加动态链接库 https://juejin.cn/post/6844903906099593229
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../nativeLibs/${ANDROID_ABI}/libopencv_java3.so)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#当前cmakelists.txt所在目录的所有.c .cpp源文件
AUX_SOURCE_DIRECTORY(. SRC_LIST)

#生成so库，直接使用变量代表那些.c文件集合
#使用opencv.cpp文件生成共享库opencv_bmp https://juejin.cn/post/6844903906099593229
add_library( # Sets the name of the library.
        opencv_bmp

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${SRC_LIST} #[[opencv.cpp]])

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

# 在ndk中查找log库 取别名log-lib https://juejin.cn/post/6844903906099593229
find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

#设置 target 需要链接的库 https://juejin.cn/post/6844903906099593229
target_link_libraries( # Specifies the target library.
        opencv_bmp
        lib_opencv

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})
