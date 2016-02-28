set(KTIKZPART_DATA_INSTALL_DIR ${DATA_INSTALL_DIR}/ktikzpart)
#add_definitions(-DORGNAME=\\\"Florian_Hackenberger\\\" -DAPPNAME=\\\"ktikz\\\")
#add_definitions(-DKTIKZ_USE_KDE)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../common
)

set(ktikzpart_SRCS
	configdialog.cpp
	configgeneralwidget.cpp
	part.cpp
	../common/templatewidget.cpp
	../common/tikzpreview.cpp
	../common/tikzpreviewthread.cpp
	../common/tikzpreviewcontroller.cpp
	../common/tikzpreviewgenerator.cpp
)

ki18n_wrap_ui(ktikzpart_SRCS
	configgeneralwidget.ui
	../common/templatewidget.ui
)

kconfig_add_kcfg_files(ktikzpart_SRCS ../common/settings.kcfgc)

#kde4_add_plugin(ktikzpart ${ktikzpart_SRCS})
add_library(ktikzpart MODULE ${ktikzpart_SRCS})

target_include_directories(ktikzpart SYSTEM PRIVATE ${POPPLER_INCLUDE_DIRS})
target_link_libraries(ktikzpart KF5::Parts KF5::IconThemes ${POPPLER_LIBRARIES})

kcoreaddons_desktop_to_json(ktikzpart ktikzpart.desktop)

install(TARGETS ktikzpart DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ktikzpart.rc DESTINATION ${KTIKZPART_DATA_INSTALL_DIR})

add_custom_target(uninstalldirs_part)
add_custom_command(TARGET uninstalldirs_part
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${KTIKZPART_DATA_INSTALL_DIR}
	COMMENT "Removing directory ${KTIKZPART_DATA_INSTALL_DIR}"
	VERBATIM)
