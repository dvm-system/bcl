# Appends 'VALUE' to each variable in a specified list.
#
# Example: bcl_append("-std=c++11" CMAKE_CXX_FLAGS)
function(bcl_append VALUE)
  foreach(ARG ${ARGN})
    set(${ARG} "${${ARG}} ${VALUE}" PARENT_SCOPE)
  endforeach()
endfunction()

# Replaces `OLD` value in the variable `VAR' by `NEW` value.
#
# If `OLD` is not in `VAR', appends `NEW` to `VAR`.
# If the option already is on the variable, do not add it.
#
# Example: bcl_replace(CMAKE_CXX_FLAGS_RELEASE "-O3" "-O2")
function(bcl_replace VAR OLD NEW)
  if("${${VAR}}" MATCHES "(^| )${NEW}($| )")
    set(TMP "")
  else()
    set(TMP "${NEW}")
  endif()
  if("${${VAR}}" MATCHES "(^| )${OLD}($| )")
    string(REGEX REPLACE "(^| )${OLD}($| )" " ${TMP} " ${VAR} "${${VAR}}")
  else()
    set(${VAR} "${${VAR}} ${TMP}")
  endif()
  set(${VAR} "${${VAR}}" PARENT_SCOPE)
endfunction(bcl_replace)

