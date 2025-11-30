# SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)
project(StackAndConquer
    VERSION 0.12.0
    DESCRIPTION "Challenging tower conquest board game"
    LANGUAGES C CXX
)
set(PROJECT_COPYRIGHT "(C) 2015-present Thorsten Roth")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MAIN_VERSION Qt6)
set(QT_MINIMUM_VERSION 6.0)
    
find_package(${QT_MAIN_VERSION} ${QT_MINIMUM_VERSION}
    REQUIRED Core Gui Widgets Qml SvgWidgets LinguistTools
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(WINDOWS ON)
#elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
#    set(MACOS ON)
endif()
    
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(WINDOWS)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/data/win.rc")
    qt_add_executable(stackandconquer ${app_icon_resource_windows})
    set_target_properties(stackandconquer PROPERTIES
        OUTPUT_NAME StackAndConquer
        WIN32_EXECUTABLE TRUE
    )
#elseif(MACOS)
#    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
#    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon.icns")
#    set_source_files_properties(${app_icon_macos} PROPERTIES
#           MACOSX_PACKAGE_LOCATION "Resources")
#
#    # TODO: To be checked how to bundle ressources (boards)
#    qt_add_executable(stackandconquer MACOSX_BUNDLE ${app_icon_macos})
else()
    qt_add_executable(stackandconquer)
endif()

target_compile_features(stackandconquer PUBLIC cxx_std_11)
target_compile_definitions(stackandconquer
    PRIVATE
        APP_NAME="${PROJECT_NAME}"
        APP_VERSION="${PROJECT_VERSION}"
        APP_COPY="${PROJECT_COPYRIGHT}"
        APP_DESC="${PROJECT_DESCRIPTION}"
        QT_NO_FOREACH
        $<$<CONFIG:DEBUG>:QT_DISABLE_DEPRECATED_BEFORE=0x061000>
)

if(MSVC)
    target_compile_options(stackandconquer
        PRIVATE
            $<$<CONFIG:DEBUG>:/W4 /permissive->
    )
else()
    target_compile_options(stackandconquer
        PRIVATE
            $<$<CONFIG:DEBUG>:-Wall -Wextra -pedantic>
    )
endif()

target_link_libraries(stackandconquer PRIVATE
    ${QT_MAIN_VERSION}::Core
    ${QT_MAIN_VERSION}::Gui
    ${QT_MAIN_VERSION}::Widgets
    ${QT_MAIN_VERSION}::Qml
    ${QT_MAIN_VERSION}::SvgWidgets
)

set(HEADERS
    stackandconquer.h
    game.h
    board.h
    player.h
    settings.h
    settingsdialog.h
    opponentjs.h
    generateboard.h
)

set(SOURCES
    main.cpp
    stackandconquer.cpp
    game.cpp
    board.cpp
    player.cpp
    settings.cpp
    settingsdialog.cpp
    opponentjs.cpp
    generateboard.cpp
)

set(FORMS
    stackandconquer.ui
    settingsdialog.ui
)

set(RESOURCES
    data/data.qrc
)

set(TRANSLATIONS
    lang/stackandconquer_de.ts
    lang/stackandconquer_en.ts
    lang/stackandconquer_it.ts
    lang/stackandconquer_ja_JP.ts
    lang/stackandconquer_nl.ts
    lang/stackandconquer_pl.ts
    lang/stackandconquer_ru.ts
)

qt_add_translation(QM_FILES ${TRANSLATIONS})
add_custom_target(translations_${PROJECT_NAME} ALL DEPENDS ${QM_FILES})
foreach(qm_file ${QM_FILES})
    get_filename_component(qm_filename "${qm_file}" NAME)
    set_source_files_properties(
        "${qm_file}"
        PROPERTIES QT_RESOURCE_ALIAS "${qm_filename}"
    )
endforeach()
qt_add_resources(stackandconquer translations_stack
    PREFIX "/"
    FILES ${QM_FILES}
)

target_sources(stackandconquer PRIVATE ${HEADERS} ${SOURCES} ${FORMS} ${RESOURCES})

if(LINUX)
    include(GNUInstallDirs)
    install(TARGETS stackandconquer DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES data/unix/com.github.elth0r0.stackandconquer.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES data/unix/com.github.elth0r0.stackandconquer.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(DIRECTORY data/boards DESTINATION ${CMAKE_INSTALL_DATADIR}/stackandconquer)
    install(DIRECTORY data/cpu DESTINATION ${CMAKE_INSTALL_DATADIR}/stackandconquer)
    install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
    add_subdirectory(man)
endif()
