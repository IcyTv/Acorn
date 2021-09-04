add_executable("Sandbox"
      "Sandbox/res/shaders/Textured.shader"
      "Sandbox/res/textures/Checkerboard.png"
      "Sandbox/res/textures/ChernoLogo.png"
      "Sandbox/res/textures/RPGpack_sheet.png"
      "Sandbox/res/textures/RPGpack_sheet_2X.png"
      "Sandbox/res/textures/roguelikeSheet_magenta.png"
      "Sandbox/res/textures/roguelikeSheet_transparent.png"
    "Sandbox/src/ParticleSystem.cpp"
    "Sandbox/src/ParticleSystem.h"
    "Sandbox/src/Sandbox.cpp"
    "Sandbox/src/Sandbox2D.cpp"
    "Sandbox/src/Sandbox2D.h"
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_dependencies("Sandbox"
    "Acorn"
  )
  set_target_properties("Sandbox" PROPERTIES
    OUTPUT_NAME "Sandbox"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Debug-windows-x86_64/Sandbox"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Debug-windows-x86_64/Sandbox"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Debug-windows-x86_64/Sandbox"
  )
endif()
target_include_directories("Sandbox" PRIVATE
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/src>
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor>
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes>
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot>
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/yaml-cpp/include>
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGuizmo>
)
target_compile_definitions("Sandbox" PRIVATE
  $<$<CONFIG:Debug>:AC_PLATFORM_WINDOWS>
  $<$<CONFIG:Debug>:AC_DEBUG>
)
target_link_directories("Sandbox" PRIVATE
  $<$<CONFIG:Debug>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("Sandbox"
  $<$<CONFIG:Debug>:Acorn>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("Sandbox" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("Sandbox" PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-g>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties("Sandbox" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_dependencies("Sandbox"
    "Acorn"
  )
  set_target_properties("Sandbox" PROPERTIES
    OUTPUT_NAME "Sandbox"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Release-windows-x86_64/Sandbox"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Release-windows-x86_64/Sandbox"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Release-windows-x86_64/Sandbox"
  )
endif()
target_include_directories("Sandbox" PRIVATE
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/src>
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor>
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes>
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot>
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/yaml-cpp/include>
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGuizmo>
)
target_compile_definitions("Sandbox" PRIVATE
  $<$<CONFIG:Release>:AC_PLATFORM_WINDOWS>
  $<$<CONFIG:Release>:AC_DEBUG>
)
target_link_directories("Sandbox" PRIVATE
  $<$<CONFIG:Release>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("Sandbox"
  $<$<CONFIG:Release>:Acorn>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("Sandbox" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("Sandbox" PRIVATE
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-O2>
  $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Release)
  set_target_properties("Sandbox" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  add_dependencies("Sandbox"
    "Acorn"
  )
  set_target_properties("Sandbox" PROPERTIES
    OUTPUT_NAME "Sandbox"
    ARCHIVE_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Dist-windows-x86_64/Sandbox"
    LIBRARY_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Dist-windows-x86_64/Sandbox"
    RUNTIME_OUTPUT_DIRECTORY "C:/Users/michael/Programming/game-engine/game-engine/bin/Dist-windows-x86_64/Sandbox"
  )
endif()
target_include_directories("Sandbox" PRIVATE
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/src>
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor>
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGui>
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImNodes>
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImPlot>
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/yaml-cpp/include>
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/Acorn/vendor/ImGuizmo>
)
target_compile_definitions("Sandbox" PRIVATE
  $<$<CONFIG:Dist>:AC_PLATFORM_WINDOWS>
  $<$<CONFIG:Dist>:AC_DIST>
)
target_link_directories("Sandbox" PRIVATE
  $<$<CONFIG:Dist>:C:/Users/michael/Programming/game-engine/game-engine/vcpkg_installed/x64-windows/lib>
)
target_link_libraries("Sandbox"
  $<$<CONFIG:Dist>:Acorn>
)
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  set_target_properties("Sandbox" PROPERTIES LINK_FLAGS "-IGNORE:4099 ")
endif()
target_compile_options("Sandbox" PRIVATE
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:C>>:-m64>
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:C>>:-O3>
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:CXX>>:-m64>
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:CXX>>:-O3>
  $<$<AND:$<CONFIG:Dist>,$<COMPILE_LANGUAGE:CXX>>:-std=c++17>
)
if(CMAKE_BUILD_TYPE STREQUAL Dist)
  set_target_properties("Sandbox" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE False
    INTERPROCEDURAL_OPTIMIZATION False
  )
endif()