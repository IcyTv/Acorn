add_library("ImNodes" STATIC
  "Acorn/vendor/ImNodes/ImNodes.cpp"
  "Acorn/vendor/ImNodes/ImNodes.h"
  "Acorn/vendor/ImNodes/ImNodesEz.cpp"
  "Acorn/vendor/ImNodes/ImNodesEz.h"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("ImNodes" PROPERTIES
    OUTPUT_NAME "ImNodes"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Debug-windows-x86_64/ImNodes"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Debug-windows-x86_64/ImNodes"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Debug-windows-x86_64/ImNodes"
  )
endif()
target_include_directories("ImNodes" PRIVATE
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes>
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
)
target_compile_definitions("ImNodes" PRIVATE
)
target_link_directories("ImNodes" PRIVATE
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("ImNodes"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("ImNodes" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("ImNodes" PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("ImNodes" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("ImNodes" PROPERTIES
    OUTPUT_NAME "ImNodes"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Release-windows-x86_64/ImNodes"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Release-windows-x86_64/ImNodes"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Release-windows-x86_64/ImNodes"
  )
endif()
target_include_directories("ImNodes" PRIVATE
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes>
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
)
target_compile_definitions("ImNodes" PRIVATE
)
target_link_directories("ImNodes" PRIVATE
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("ImNodes"
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("ImNodes" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("ImNodes" PRIVATE
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("ImNodes" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  set_target_properties("ImNodes" PROPERTIES
    OUTPUT_NAME "ImNodes"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Dist-windows-x86_64/ImNodes"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Dist-windows-x86_64/ImNodes"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes/bin/Dist-windows-x86_64/ImNodes"
  )
endif()
target_include_directories("ImNodes" PRIVATE
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes>
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
)
target_compile_definitions("ImNodes" PRIVATE
)
target_link_directories("ImNodes" PRIVATE
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("ImNodes"
)
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  set_target_properties("ImNodes" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("ImNodes" PRIVATE
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  set_target_properties("ImNodes" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()