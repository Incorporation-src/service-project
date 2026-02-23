cmake_minimum_required(VERSION 3.10)
project(microservices)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SQLite3 REQUIRED)

# ==================== User Service ====================
set(USER_SERVICE_SOURCES
    user_service/src/main.cpp
    user_service/src/user_database.cpp
)

add_executable(user_service ${USER_SERVICE_SOURCES})

target_include_directories(user_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/user_service/include
    ${SQLITE3_INCLUDE_DIRS}
)

target_link_libraries(user_service PRIVATE
    pthread
    sqlite3
)

# ==================== Order Service ====================
set(ORDER_SERVICE_SOURCES
    order_service/src/main.cpp
    order_service/src/order_database.cpp
)

add_executable(order_service ${ORDER_SERVICE_SOURCES})

target_include_directories(order_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/order_service/include
    ${SQLITE3_INCLUDE_DIRS}
)

target_link_libraries(order_service PRIVATE
    pthread
    sqlite3
)