
set(module_name "filesystem") # We conveniently store the name of our module/folder, so we could change it at one place
set(library_name "module_${module_name}") # Modules are static libraries and tied together to the main project

declare_module(${module_name} .) # Get the source lists of all files in this folder (and register us as module)
add_lib(${module_name}) # Our module is **a static lib** which gets linked into other modules or apps.

# Acquire our dependencies for this module
# (NOLINK is used since we're only a module/static library, no app)
# (library_name is used instead of the module_name, since we require it for this subproject)
config_sdl(${library_name} NOLINK)
config_boost_filesystem(${library_name} NOLINK)
config_rapidjson(${library_name} NOLINK)


# This function is used to bind this module into another module/application
function(config_filesystem targ)
  message(STATUS "Configuring ${targ} with ${module_name}")
  
  if (";${ARGN};" MATCHES ";NOLINK;") # Some argument passing, just copy that.
    set(NOLINK "NOLINK")
  endif()
  target_link_libs(${targ} ${library_name} ${NOLINK}) # Tell the requiring module that it needs to link with our static lib

  # Also tell the other module that it needs our dependencies:
  config_sdl(${targ} ${NOLINK}) # we need sdl since we havent refractored cube.hpp good enough yet
  config_boost_filesystem(${targ} ${NOLINK})
  config_rapidjson(${targ} ${NOLINK})

endfunction()
