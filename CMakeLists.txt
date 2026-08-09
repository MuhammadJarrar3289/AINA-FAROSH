cmake_minimum_required(VERSION 3.16)
project(UrduPoetryStudio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Quick Gui Widgets)
# For PDF export:
find_package(Qt6 6.2 REQUIRED COMPONENTS PrintSupport)
find_package(Freetype REQUIRED)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/Backend.cpp
    src/FontInspector.cpp
    src/ProjectManager.cpp
)

qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
)

qt_add_qml_module(${PROJECT_NAME}
    URI UrduPoetryStudio
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/PoetryEditor.qml
        qml/Theme.qml
        qml/FontManager.qml
)

# Resources (fonts, qml)
qt_add_resources(RESOURCES
    resources.qrc
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Widgets Qt6::PrintSupport Freetype::Freetype
)

# Copy sample_poem.txt and fonts folder to build dir for convenience
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/sample_poem.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
