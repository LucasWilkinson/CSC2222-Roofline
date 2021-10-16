#
#   Call `add_subdirectory(third_party)`, NOTE will set THIRD_PARTY_FOLDER in the parent scope
#   Then call one of the following to add the corresponding third-party dependency to the target:
#       `add_third_party_dependency_plot(<target>)`
#

set(THIRD_PARTY_FOLDER "third_party" PARENT_SCOPE)

function(add_third_party_dependency_hopscotch TARGET)
    target_include_directories(${TARGET} PUBLIC "${THIRD_PARTY_FOLDER}/hopscotch/include")
    # TODO: [Lucas] Cleanup
    target_sources(${TARGET} PUBLIC "${THIRD_PARTY_FOLDER}/hopscotch/src/rw_roofline.cpp")
    target_sources(${TARGET} PUBLIC "${THIRD_PARTY_FOLDER}/hopscotch/src/common.cpp")
endfunction()

function(add_third_party_dependency_tables_and_graphs TARGET)
    target_include_directories(${TARGET} PUBLIC "${THIRD_PARTY_FOLDER}/Tables-and-Graphs/include")
endfunction()