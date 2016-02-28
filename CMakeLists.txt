project(ktikz)
cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)

set (QT_MIN_VERSION "5.4.0")
set(KF5_DEP_VERSION "5.10.0")
set(POPPLER_VERSION "0.41.0")

set(KTIKZ_VERSION "0.10")

add_definitions(-DORGNAME=\"Florian_Hackenberger\" -DAPPNAME=\"ktikz\")
add_definitions(-DAPPVERSION=\"${KTIKZ_VERSION}\")
add_definitions(-DKTIKZ_TIKZ_DOCUMENTATION_DEFAULT=\"/usr/share/doc/texmf/pgf/pgfmanual.pdf.gz\")
if(WIN32 AND NOT UNIX)
	add_definitions(-DKTIKZ_TEMPLATE_EDITOR_DEFAULT=\"notepad\")
else(WIN32 AND NOT UNIX)
	add_definitions(-DKTIKZ_TEMPLATE_EDITOR_DEFAULT=\"kwrite\")
endif(WIN32 AND NOT UNIX)

# we need some parts of the ECM CMake helpers
find_package (ECM 1.1.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
# include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
# include(ECMGenerateHeaders)
include(ECMAddAppIcon) 
# include(GenerateExportHeader)

include(CMakePackageConfigHelpers)
include(FindPkgConfig)
include(FeatureSummary)
# include(WriteBasicConfigVersionFile)
include (CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets Xml LinguistTools)
pkg_search_module(POPPLER REQUIRED poppler-qt5>=${POPPLER_VERSION})

if(BUILD_TESTING)
  find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
endif()

# Load the frameworks we need
find_package(KF5 "${KF5_DEP_VERSION}" REQUIRED COMPONENTS
  Config
#   Crash
  I18n
  JobWidgets
  KIO
  Parts
  TextEditor
#   WindowSystem
  XmlGui
  IconThemes
)

# some optional deps, to make compiling on some OSes easier
find_package(KF5 "${KF5_DEP_VERSION}" COMPONENTS DocTools)

# use sane compile flags
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
#   -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
#   -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
)

add_subdirectory(app)
add_subdirectory(part)

ecm_optional_add_subdirectory(po)

# docs, if doc tools around
if (KF5DocTools_FOUND)
    ecm_optional_add_subdirectory(doc)
endif ()

# tell about our features (and what is missing)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)



# Install text/x-pgf mimetype

find_package(SharedMimeInfo)
install(FILES common/qtikz.xml DESTINATION ${XDG_MIME_INSTALL_DIR} RENAME ktikz.xml)
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

# Make packages

