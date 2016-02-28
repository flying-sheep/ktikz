file(GLOB app_icons *-apps-ktikz.png)
ecm_install_icons(
	ICONS ${app_icons}
	DESTINATION ${ICON_INSTALL_DIR})
