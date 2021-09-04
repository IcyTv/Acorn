add_library("ImPlot" STATIC
  "Acorn/vendor/ImPlot/implot.cpp"
  "Acorn/vendor/ImPlot/implot.h"
  "Acorn/vendor/ImPlot/implot_internal.h"
  "Acorn/vendor/ImPlot/implot_items.cpp"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("ImPlot" PROPERTIES
    OUTPUT_NAME "ImPlot"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Debug-windows-x86_64/ImPlot"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Debug-windows-x86_64/ImPlot"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Debug-windows-x86_64/ImPlot"
  )
endif()
target_include_directories("ImPlot" PRIVATE
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot>
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
)
target_compile_definitions("ImPlot" PRIVATE
)
target_link_directories("ImPlot" PRIVATE
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("ImPlot"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("ImPlot" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("ImPlot" PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("ImPlot" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("ImPlot" PROPERTIES
    OUTPUT_NAME "ImPlot"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Release-windows-x86_64/ImPlot"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Release-windows-x86_64/ImPlot"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Release-windows-x86_64/ImPlot"
  )
endif()
target_include_directories("ImPlot" PRIVATE
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot>
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
)
target_compile_definitions("ImPlot" PRIVATE
)
target_link_directories("ImPlot" PRIVATE
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("ImPlot"
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("ImPlot" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("ImPlot" PRIVATE
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("ImPlot" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  set_target_properties("ImPlot" PROPERTIES
    OUTPUT_NAME "ImPlot"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Dist-windows-x86_64/ImPlot"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Dist-windows-x86_64/ImPlot"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot/bin/Dist-windows-x86_64/ImPlot"
  )
endif()
target_include_directories("ImPlot" PRIVATE
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot>
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
)
target_compile_definitions("ImPlot" PRIVATE
)
target_link_directories("ImPlot" PRIVATE
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("ImPlot"
)
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  set_target_properties("ImPlot" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("ImPlot" PRIVATE
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  set_target_properties("ImPlot" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()