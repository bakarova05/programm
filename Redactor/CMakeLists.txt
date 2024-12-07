cmake_minimum_required(VERSION 3.5) # Минимальная версия CMake
project(ObjectDetection) # Имя проекта
find_package(OpenCV REQUIRED) # Поиск пакета OpenCV
add_executable(ObjectDetection main.cpp) # Создание исполняемого файла
target_link_libraries(ObjectDetection ${OpenCV_LIBS}) # Линковка с библиотеками OpenCV