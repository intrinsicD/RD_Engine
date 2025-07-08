# Define the path to our assets directory relative to the root source directory.
set(RDE_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "RDE Asset Directory: ${RDE_ASSET_DIR}")

# This command takes config.h.in, replaces the @VARIABLES@,
# and generates a real config.h file in the BUILD directory.
configure_file(
        "config.h.in"
        "${CMAKE_BINARY_DIR}/config.h"
)

# Now, we must tell our C++ targets to include the BUILD directory
# so they can find the generated config.h.
# This must be done for any target that needs the asset path.
target_include_directories(AssetSystem PUBLIC
        "${CMAKE_BINARY_DIR}"
)

# In your target definition, you also pass the variable to the template
# Note: We need to use CMAKE_CROSSCOMPILING_EMULATOR to make this work
# in case you cross-compile for other platforms.
# Let's create a full path for development asset path.
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" DEV_ASSET_PATH_CMAKE)

# Configure the template with the correct variable.
# You will need to define this variable in the configure_file call.
# I have to correct the structure slightly. The set() command alone is not enough.
# A cleaner approach is to set it as a cache variable or pass directly.
# Let's go with a cleaner approach for clarity.

# Let's refine the configuration step for better clarity.
# In the main CMakeLists.txt:

# Let's correct the whole process:
set(DEV_ASSET_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Note: configure_file will substitute any variable defined with set()
# Let's adjust the .in file to use DEV_ASSET_PATH

# config.h.in (Corrected)
# #define RDE_DEVELOPMENT_ASSET_PATH "@DEV_ASSET_PATH@"

# Back in CMakeLists.txt
configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

# That's much cleaner.