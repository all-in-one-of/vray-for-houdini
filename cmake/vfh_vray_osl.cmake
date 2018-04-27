#
# Copyright (c) 2015-2018, Chaos Software Ltd
#
# V-Ray For Houdini
#
# ACCESSIBLE SOURCE CODE WITHOUT DISTRIBUTION OF MODIFICATION LICENSE
#
# Full license text: https://github.com/ChaosGroup/vray-for-houdini/blob/master/LICENSE
#

find_package(VRayOSL)

macro(use_vray_osl)
	if(NOT VRayOSL_FOUND)
		message(FATAL_ERROR "V-Ray OSL NOT found!\n"
							"To specify V-Ray SDK search path, use one of the following options:\n"
							"-DSDK_PATH=<VFH dependencies location>\n"
							)
	endif()
	message_array("Using OSL" VRayOSL_INCLUDES)

	include_directories(${VRayOSL_INCLUDES})
	link_directories(${VRayOSL_LIBRARIES})
endmacro()

macro(link_with_vray_osl _name)
	if(APPLE)
		set(VRAY_OSL_LIBS vrayoslquery_s
		                  vrayoslexec_s
		                  vrayoslcomp_s
		                  vrayopenimageio_s)
	else()
		set(VRAY_OSL_LIBS vrayosl
		                  vrayopenimageio)
	endif()

	target_link_libraries(${_name} ${VRAY_OSL_LIBS})
endmacro()
