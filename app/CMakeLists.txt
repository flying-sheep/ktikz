set(KTIKZ_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/ktikz)
set(KTIKZ_TRANSLATIONS_INSTALL_DIR ${KTIKZ_DATA_INSTALL_DIR}/locale)
set(KTIKZ_TEMPLATES_INSTALL_DIR ${KTIKZ_DATA_INSTALL_DIR}/templates)
add_definitions(-DKTIKZ_TRANSLATIONS_INSTALL_DIR=\"${KTIKZ_TRANSLATIONS_INSTALL_DIR}\")
add_definitions(-DKTIKZ_TEMPLATES_INSTALL_DIR=\"${KTIKZ_TEMPLATES_INSTALL_DIR}\")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../common
	${CMAKE_CURRENT_SOURCE_DIR}/../common/utils
)

set(ktikz_SRCS
	configappearancewidget.cpp
	configeditorwidget.cpp
	configgeneralwidget.cpp
	configdialog.cpp
	editgotolinewidget.cpp
	editindentdialog.cpp
	editreplacewidget.cpp
	editreplacecurrentwidget.cpp
	ktikzapplication.cpp
	loghighlighter.cpp
	logtextedit.cpp
	main.cpp
	mainwindow.cpp
	tikzcommandinserter.cpp
	tikzcommandwidget.cpp
	tikzdocumentationcontroller.cpp
	tikzeditor.cpp
	tikzeditorhighlighter.cpp
	tikzeditorview.cpp
	../common/templatewidget.cpp
	../common/tikzpreview.cpp
	../common/tikzpreviewthread.cpp
	../common/tikzpreviewcontroller.cpp
	../common/tikzpreviewgenerator.cpp
)
ki18n_wrap_ui(ktikz_UI_FILES
	configappearancewidget.ui
	configeditorwidget.ui
	configgeneralwidget.ui
	editgotolinewidget.ui
	editindentdialog.ui
	editreplacewidget.ui
	../common/templatewidget.ui
)
kconfig_add_kcfg_files(ktikz_SRCS ../common/settings.kcfgc)
qt5_add_resources(ktikz_SRCS ktikz.qrc)

file(GLOB ktikz_TS_FILES qtikz_*.ts)
qt5_add_translation(ktikz_QM_FILES ${ktikz_TS_FILES})

ecm_add_app_icon(ktikz_SRCS ICONS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-ktikz.png") # add icons for win32 and mac; this command assumes that the first arg is of the form <appname>_SRCS

add_executable(ktikz ${ktikz_SRCS} ${ktikz_UI_FILES} ${ktikz_QM_FILES})
target_include_directories(ktikz SYSTEM PRIVATE ${POPPLER_INCLUDE_DIRS})
target_link_libraries(ktikz KF5::XmlGui KF5::IconThemes KF5::TextEditor ${POPPLER_LIBRARIES})

install(TARGETS ktikz DESTINATION ${BIN_INSTALL_DIR})
install(FILES ktikzui.rc DESTINATION ${KXMLGUI_INSTALL_DIR})
install(FILES ../common/ktikz.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES ktikz.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES ${ktikz_QM_FILES} DESTINATION ${KTIKZ_TRANSLATIONS_INSTALL_DIR})
install(FILES ../examples/template_example.pgs ../examples/template_example2.pgs ../examples/beamer-example-template.pgs DESTINATION ${KTIKZ_TEMPLATES_INSTALL_DIR})

add_subdirectory(icons)

add_custom_target(uninstalldirs_app)
add_custom_command(TARGET uninstalldirs_app
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${KTIKZ_TRANSLATIONS_INSTALL_DIR}
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${KTIKZ_TEMPLATES_INSTALL_DIR}
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${KTIKZ_DATA_INSTALL_DIR}
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${DATA_INSTALL_DIR}/ktikz
	COMMENT "Removing directories ${KTIKZ_TRANSLATIONS_INSTALL_DIR} ${KTIKZ_TEMPLATES_INSTALL_DIR} ${KTIKZ_DATA_INSTALL_DIR} ${DATA_INSTALL_DIR}/ktikz"
	VERBATIM)
