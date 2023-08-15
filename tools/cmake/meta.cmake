function(set_meta_information)

# add some helpful information to library
# get the latest commit hash of the working branch git branch
execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE META_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# get git branch name
execute_process(
        COMMAND git branch --show-current
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE META_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# branch and hash
#add_definitions(-DMETA_GIT_BRANCH="${META_GIT_BRANCH}")
#add_definitions(-DMETA_GIT_HASH="${META_GIT_HASH}")

set_property(GLOBAL PROPERTY META_GIT_BRANCH ${META_GIT_BRANCH})
set_property(GLOBAL PROPERTY META_GIT_HASH ${META_GIT_HASH})

# time stamp
string(TIMESTAMP META_TIMESTAMP [UTC])
#add_definitions(-DMETA_TIMESTAMP="${META_TIMESTAMP} UTC")
set_property(GLOBAL PROPERTY META_TIMESTAMP "${META_TIMESTAMP} UTC")

# compiler
#add_definitions(-DMETA_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")
#add_definitions(-DMETA_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")

set_property(GLOBAL PROPERTY META_COMPILER_ID "${CMAKE_CXX_COMPILER_ID} UTC")
set_property(GLOBAL PROPERTY META_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION} UTC")

endfunction()