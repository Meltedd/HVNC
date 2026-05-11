cmake_minimum_required(VERSION 3.15...3.30)
project(HVNC LANGUAGES CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(MSVC)
  # /EHsc set per target
  string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  # Default /O2 /Ob2 collides with target optimization (D9025)
  string(REPLACE "/O2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  # CMP0092 NEW drops /W3 from defaults
  add_compile_options(/W3)
endif()

add_executable(Server WIN32
  Server/Main.cpp
  Server/Server.cpp
  Server/ControlWindow.cpp
)
target_compile_definitions(Server PRIVATE UNICODE _UNICODE)
target_link_libraries(Server PRIVATE ws2_32)
if(MSVC)
  # <iostream> uses exceptions (C4530)
  target_compile_options(Server PRIVATE
    /EHsc
    $<$<CONFIG:Release>:/O2>
  )
  set_target_properties(Server PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

add_executable(Client
  Client/Main.cpp
  Client/HiddenDesktop.cpp
  common/Api.cpp
  common/HTTP.cpp
  common/Panel.cpp
  common/Utils.cpp
)
target_link_libraries(Client PRIVATE gdiplus)
if(MSVC)
  target_compile_options(Client PRIVATE
    $<$<CONFIG:Release>:/O1 /Ob0 /Oi>
  )
endif()
