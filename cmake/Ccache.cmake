find_program(CCACHE_BIN ccache)

if(CCACHE_BIN)
    message(STATUS "Using ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_BIN})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_BIN})
else()
    message(WARNING "Ccache not found")
endif()
