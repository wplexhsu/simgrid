find_library(HAVE_NS3_LIB
    NAME ns3
    HINTS
    $ENV{LD_LIBRARY_PATH}
    $ENV{HOME}
    PATH_SUFFIXES lib64 lib lib64/ns3 lib/ns3
    PATHS
    /opt
    /opt/local
    /opt/csw
    /sw
    /usr
    ${ns3_path}
)

find_path(HAVE_CORE_MODULE_H
	NAME ns3/core-module.h
    HINTS
    $ENV{HOME}
    PATH_SUFFIXES include include/ns3
    PATHS
    /opt
    /opt/local
    /opt/csw
    /sw
    /usr
    ${ns3_path}
)

message(STATUS "Looking for core-module.h")
if(HAVE_CORE_MODULE_H)
message(STATUS "Looking for core-module.h - found")
else(HAVE_CORE_MODULE_H)
message(STATUS "Looking for core-module.h - not found")
endif(HAVE_CORE_MODULE_H)
mark_as_advanced(HAVE_CORE_MODULE_H)

message(STATUS "Looking for lib ns3")
if(HAVE_NS3_LIB)
message(STATUS "Looking for lib ns3 - found")
else(HAVE_NS3_LIB)
message(STATUS "Looking for lib ns3 - not found")
endif(HAVE_NS3_LIB)
mark_as_advanced(HAVE_NS3_LIB)

if(HAVE_NS3_LIB AND HAVE_CORE_MODULE_H)
	set(HAVE_NS3 1)
	string(REPLACE "/libns3.${LIB_EXE}" ""  HAVE_NS3_LIB "${HAVE_NS3_LIB}")
	string(REGEX MATCH "${HAVE_NS3_LIB}" operation "$ENV{LD_LIBRARY_PATH}")
	
	if(NOT operation)
	
		message(STATUS "Warning: To use NS-3 don't forget to set LD_LIBRARY_PATH with:	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HAVE_NS3_LIB}")
		set(HAVE_NS3 0)
		
	else(NOT operation)
	
		string(REGEX MATCH "-L${HAVE_NS3_LIB} " operation1 "${CMAKE_C_FLAGS}")
		if(NOT operation1)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-L${HAVE_NS3_LIB} ")
		endif(NOT operation1)
	
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-I${HAVE_CORE_MODULE_H} -L${HAVE_NS3_LIB} ")
	endif(NOT operation)		

endif(HAVE_NS3_LIB AND HAVE_CORE_MODULE_H)