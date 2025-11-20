cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(antibox_cmake C CXX)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM"
     OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
     OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_C_FLAGS "-O1")
    set(CMAKE_C_FLAGS_DEBUG "-O1")
    set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O2")
    set(CMAKE_CXX_FLAGS "-O1")
    set(CMAKE_CXX_FLAGS_DEBUG "-O1")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2")
    set(CMAKE_CXX_STANDARD 20)
elseif(UNIX)
    # Set default flags for GCC/Clang on Linux
    set(CMAKE_C_FLAGS "-DNDEBUG -O2")
    set(CMAKE_C_FLAGS_DEBUG "")       # Debugging symbols for debug builds
    set(CMAKE_C_FLAGS_RELEASE "-O2")    # Optimization for release builds

    set(CMAKE_CXX_FLAGS "-DNDEBUG -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "")     # Debugging symbols for debug builds
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")  # Optimization for release builds

    set(CMAKE_CXX_STANDARD 20)          # Enable C++20 standard
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################





set(PROJECT_NAME antibox_cmake)
#set_target_properties(antibox_cmake OUTPUT_NAME "btf_game")

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "Libraries/include/antibox/core/default.frag"
    "Libraries/include/antibox/core/default.vert"
)
source_group("" FILES ${no_group_source_files})

set(Header_Files
    "dat/Chunk.h"
    "dat/cosmetic.h"
    "dat/crafting.h"
    "dat/entities.h"
    "dat/filemanager.h"
    "dat/glyphmanager.h"
    "dat/Game.h"
    "dat/inventory.h"
    "dat/items.h"
    "dat/map.h"
    "dat/uiscreen.h"
    "Libraries/include/antibox/core/antibox.h"
    "Libraries/include/antibox/core/app.h"
    "Libraries/include/antibox/core/audio.h"
    "Libraries/include/antibox/core/engine.h"
    "Libraries/include/antibox/core/imguiwindow.h"
    "Libraries/include/antibox/core/log.h"
    "Libraries/include/antibox/core/mathlib.h"
    "Libraries/include/antibox/core/window.h"
    "Libraries/include/antibox/graphics/3d/camera.h"
    "Libraries/include/antibox/graphics/framebuffer.h"
    "Libraries/include/antibox/graphics/helpers.h"
    "Libraries/include/antibox/graphics/mesh.h"
    "Libraries/include/antibox/graphics/rendercommands.h"
    "Libraries/include/antibox/graphics/shader.h"
    "Libraries/include/antibox/graphics/sprite.h"
    "Libraries/include/antibox/graphics/texture.h"
    "Libraries/include/antibox/graphics/vertex.h"
    "Libraries/include/antibox/managers/factory.h"
    "Libraries/include/antibox/managers/rendermanager.h"
    "Libraries/include/antibox/objects/3d/model.h"
    "Libraries/include/antibox/objects/gameobject.h"
    "Libraries/include/antibox/objects/scene.h"
    "Libraries/include/antibox/objects/tokenizer.h"
    "Libraries/include/antibox/tools/noise.h"
    "Libraries/include/EGL/eglplatform.h"
    "Libraries/include/glad/glad.h"
    "Libraries/include/glad/egl.h"
    "Libraries/include/glad/gles1.h"
    "Libraries/include/glad/gles2.h"
    "Libraries/include/glad/glsc2.h"
    "Libraries/include/glad/glx.h"
    "Libraries/include/glad/vulkan.h"
    "Libraries/include/glad/vulkansc.h"
    "Libraries/include/glad/wgl.h"
    "Libraries/include/glm/common.hpp"
    "Libraries/include/glm/detail/_features.hpp"
    "Libraries/include/glm/detail/_fixes.hpp"
    "Libraries/include/glm/detail/_noise.hpp"
    "Libraries/include/glm/detail/_swizzle.hpp"
    "Libraries/include/glm/detail/_swizzle_func.hpp"
    "Libraries/include/glm/detail/_vectorize.hpp"
    "Libraries/include/glm/detail/compute_common.hpp"
    "Libraries/include/glm/detail/compute_vector_relational.hpp"
    "Libraries/include/glm/detail/func_common.inl"
    "Libraries/include/glm/detail/func_common_simd.inl"
    "Libraries/include/glm/detail/func_exponential.inl"
    "Libraries/include/glm/detail/func_exponential_simd.inl"
    "Libraries/include/glm/detail/func_geometric.inl"
    "Libraries/include/glm/detail/func_geometric_simd.inl"
    "Libraries/include/glm/detail/func_integer.inl"
    "Libraries/include/glm/detail/func_integer_simd.inl"
    "Libraries/include/glm/detail/func_matrix.inl"
    "Libraries/include/glm/detail/func_matrix_simd.inl"
    "Libraries/include/glm/detail/func_packing.inl"
    "Libraries/include/glm/detail/func_packing_simd.inl"
    "Libraries/include/glm/detail/func_trigonometric.inl"
    "Libraries/include/glm/detail/func_trigonometric_simd.inl"
    "Libraries/include/glm/detail/func_vector_relational.inl"
    "Libraries/include/glm/detail/func_vector_relational_simd.inl"
    "Libraries/include/glm/detail/qualifier.hpp"
    "Libraries/include/glm/detail/setup.hpp"
    "Libraries/include/glm/detail/type_float.hpp"
    "Libraries/include/glm/detail/type_half.hpp"
    "Libraries/include/glm/detail/type_half.inl"
    "Libraries/include/glm/detail/type_mat2x2.hpp"
    "Libraries/include/glm/detail/type_mat2x2.inl"
    "Libraries/include/glm/detail/type_mat2x3.hpp"
    "Libraries/include/glm/detail/type_mat2x3.inl"
    "Libraries/include/glm/detail/type_mat2x4.hpp"
    "Libraries/include/glm/detail/type_mat2x4.inl"
    "Libraries/include/glm/detail/type_mat3x2.hpp"
    "Libraries/include/glm/detail/type_mat3x2.inl"
    "Libraries/include/glm/detail/type_mat3x3.hpp"
    "Libraries/include/glm/detail/type_mat3x3.inl"
    "Libraries/include/glm/detail/type_mat3x4.hpp"
    "Libraries/include/glm/detail/type_mat3x4.inl"
    "Libraries/include/glm/detail/type_mat4x2.hpp"
    "Libraries/include/glm/detail/type_mat4x2.inl"
    "Libraries/include/glm/detail/type_mat4x3.hpp"
    "Libraries/include/glm/detail/type_mat4x3.inl"
    "Libraries/include/glm/detail/type_mat4x4.hpp"
    "Libraries/include/glm/detail/type_mat4x4.inl"
    "Libraries/include/glm/detail/type_mat4x4_simd.inl"
    "Libraries/include/glm/detail/type_quat.hpp"
    "Libraries/include/glm/detail/type_quat.inl"
    "Libraries/include/glm/detail/type_quat_simd.inl"
    "Libraries/include/glm/detail/type_vec1.hpp"
    "Libraries/include/glm/detail/type_vec1.inl"
    "Libraries/include/glm/detail/type_vec2.hpp"
    "Libraries/include/glm/detail/type_vec2.inl"
    "Libraries/include/glm/detail/type_vec3.hpp"
    "Libraries/include/glm/detail/type_vec3.inl"
    "Libraries/include/glm/detail/type_vec4.hpp"
    "Libraries/include/glm/detail/type_vec4.inl"
    "Libraries/include/glm/detail/type_vec4_simd.inl"
    "Libraries/include/glm/exponential.hpp"
    "Libraries/include/glm/ext.hpp"
    "Libraries/include/glm/ext/matrix_clip_space.hpp"
    "Libraries/include/glm/ext/matrix_clip_space.inl"
    "Libraries/include/glm/ext/matrix_common.hpp"
    "Libraries/include/glm/ext/matrix_common.inl"
    "Libraries/include/glm/ext/matrix_double2x2.hpp"
    "Libraries/include/glm/ext/matrix_double2x2_precision.hpp"
    "Libraries/include/glm/ext/matrix_double2x3.hpp"
    "Libraries/include/glm/ext/matrix_double2x3_precision.hpp"
    "Libraries/include/glm/ext/matrix_double2x4.hpp"
    "Libraries/include/glm/ext/matrix_double2x4_precision.hpp"
    "Libraries/include/glm/ext/matrix_double3x2.hpp"
    "Libraries/include/glm/ext/matrix_double3x2_precision.hpp"
    "Libraries/include/glm/ext/matrix_double3x3.hpp"
    "Libraries/include/glm/ext/matrix_double3x3_precision.hpp"
    "Libraries/include/glm/ext/matrix_double3x4.hpp"
    "Libraries/include/glm/ext/matrix_double3x4_precision.hpp"
    "Libraries/include/glm/ext/matrix_double4x2.hpp"
    "Libraries/include/glm/ext/matrix_double4x2_precision.hpp"
    "Libraries/include/glm/ext/matrix_double4x3.hpp"
    "Libraries/include/glm/ext/matrix_double4x3_precision.hpp"
    "Libraries/include/glm/ext/matrix_double4x4.hpp"
    "Libraries/include/glm/ext/matrix_double4x4_precision.hpp"
    "Libraries/include/glm/ext/matrix_float2x2.hpp"
    "Libraries/include/glm/ext/matrix_float2x2_precision.hpp"
    "Libraries/include/glm/ext/matrix_float2x3.hpp"
    "Libraries/include/glm/ext/matrix_float2x3_precision.hpp"
    "Libraries/include/glm/ext/matrix_float2x4.hpp"
    "Libraries/include/glm/ext/matrix_float2x4_precision.hpp"
    "Libraries/include/glm/ext/matrix_float3x2.hpp"
    "Libraries/include/glm/ext/matrix_float3x2_precision.hpp"
    "Libraries/include/glm/ext/matrix_float3x3.hpp"
    "Libraries/include/glm/ext/matrix_float3x3_precision.hpp"
    "Libraries/include/glm/ext/matrix_float3x4.hpp"
    "Libraries/include/glm/ext/matrix_float3x4_precision.hpp"
    "Libraries/include/glm/ext/matrix_float4x2.hpp"
    "Libraries/include/glm/ext/matrix_float4x2_precision.hpp"
    "Libraries/include/glm/ext/matrix_float4x3.hpp"
    "Libraries/include/glm/ext/matrix_float4x3_precision.hpp"
    "Libraries/include/glm/ext/matrix_float4x4.hpp"
    "Libraries/include/glm/ext/matrix_float4x4_precision.hpp"
    "Libraries/include/glm/ext/matrix_int2x2.hpp"
    "Libraries/include/glm/ext/matrix_int2x2_sized.hpp"
    "Libraries/include/glm/ext/matrix_int2x3.hpp"
    "Libraries/include/glm/ext/matrix_int2x3_sized.hpp"
    "Libraries/include/glm/ext/matrix_int2x4.hpp"
    "Libraries/include/glm/ext/matrix_int2x4_sized.hpp"
    "Libraries/include/glm/ext/matrix_int3x2.hpp"
    "Libraries/include/glm/ext/matrix_int3x2_sized.hpp"
    "Libraries/include/glm/ext/matrix_int3x3.hpp"
    "Libraries/include/glm/ext/matrix_int3x3_sized.hpp"
    "Libraries/include/glm/ext/matrix_int3x4.hpp"
    "Libraries/include/glm/ext/matrix_int3x4_sized.hpp"
    "Libraries/include/glm/ext/matrix_int4x2.hpp"
    "Libraries/include/glm/ext/matrix_int4x2_sized.hpp"
    "Libraries/include/glm/ext/matrix_int4x3.hpp"
    "Libraries/include/glm/ext/matrix_int4x3_sized.hpp"
    "Libraries/include/glm/ext/matrix_int4x4.hpp"
    "Libraries/include/glm/ext/matrix_int4x4_sized.hpp"
    "Libraries/include/glm/ext/matrix_projection.hpp"
    "Libraries/include/glm/ext/matrix_projection.inl"
    "Libraries/include/glm/ext/matrix_relational.hpp"
    "Libraries/include/glm/ext/matrix_relational.inl"
    "Libraries/include/glm/ext/matrix_transform.hpp"
    "Libraries/include/glm/ext/matrix_transform.inl"
    "Libraries/include/glm/ext/matrix_uint2x2.hpp"
    "Libraries/include/glm/ext/matrix_uint2x2_sized.hpp"
    "Libraries/include/glm/ext/matrix_uint2x3.hpp"
    "Libraries/include/glm/ext/matrix_uint2x3_sized.hpp"
    "Libraries/include/glm/ext/matrix_uint2x4.hpp"
    "Libraries/include/glm/ext/matrix_uint2x4_sized.hpp"
    "Libraries/include/glm/ext/matrix_uint3x2.hpp"
    "Libraries/include/glm/ext/matrix_uint3x2_sized.hpp"
    "Libraries/include/glm/ext/matrix_uint3x3.hpp"
    "Libraries/include/glm/ext/matrix_uint3x3_sized.hpp"
    "Libraries/include/glm/ext/matrix_uint3x4.hpp"
    "Libraries/include/glm/ext/matrix_uint3x4_sized.hpp"
    "Libraries/include/glm/ext/matrix_uint4x2.hpp"
    "Libraries/include/glm/ext/matrix_uint4x2_sized.hpp"
    "Libraries/include/glm/ext/matrix_uint4x3.hpp"
    "Libraries/include/glm/ext/matrix_uint4x3_sized.hpp"
    "Libraries/include/glm/ext/matrix_uint4x4.hpp"
    "Libraries/include/glm/ext/matrix_uint4x4_sized.hpp"
    "Libraries/include/glm/ext/quaternion_common.hpp"
    "Libraries/include/glm/ext/quaternion_common.inl"
    "Libraries/include/glm/ext/quaternion_common_simd.inl"
    "Libraries/include/glm/ext/quaternion_double.hpp"
    "Libraries/include/glm/ext/quaternion_double_precision.hpp"
    "Libraries/include/glm/ext/quaternion_exponential.hpp"
    "Libraries/include/glm/ext/quaternion_exponential.inl"
    "Libraries/include/glm/ext/quaternion_float.hpp"
    "Libraries/include/glm/ext/quaternion_float_precision.hpp"
    "Libraries/include/glm/ext/quaternion_geometric.hpp"
    "Libraries/include/glm/ext/quaternion_geometric.inl"
    "Libraries/include/glm/ext/quaternion_relational.hpp"
    "Libraries/include/glm/ext/quaternion_relational.inl"
    "Libraries/include/glm/ext/quaternion_transform.hpp"
    "Libraries/include/glm/ext/quaternion_transform.inl"
    "Libraries/include/glm/ext/quaternion_trigonometric.hpp"
    "Libraries/include/glm/ext/quaternion_trigonometric.inl"
    "Libraries/include/glm/ext/scalar_common.hpp"
    "Libraries/include/glm/ext/scalar_common.inl"
    "Libraries/include/glm/ext/scalar_constants.hpp"
    "Libraries/include/glm/ext/scalar_constants.inl"
    "Libraries/include/glm/ext/scalar_int_sized.hpp"
    "Libraries/include/glm/ext/scalar_integer.hpp"
    "Libraries/include/glm/ext/scalar_integer.inl"
    "Libraries/include/glm/ext/scalar_packing.hpp"
    "Libraries/include/glm/ext/scalar_packing.inl"
    "Libraries/include/glm/ext/scalar_relational.hpp"
    "Libraries/include/glm/ext/scalar_relational.inl"
    "Libraries/include/glm/ext/scalar_uint_sized.hpp"
    "Libraries/include/glm/ext/scalar_ulp.hpp"
    "Libraries/include/glm/ext/scalar_ulp.inl"
    "Libraries/include/glm/ext/vector_bool1.hpp"
    "Libraries/include/glm/ext/vector_bool1_precision.hpp"
    "Libraries/include/glm/ext/vector_bool2.hpp"
    "Libraries/include/glm/ext/vector_bool2_precision.hpp"
    "Libraries/include/glm/ext/vector_bool3.hpp"
    "Libraries/include/glm/ext/vector_bool3_precision.hpp"
    "Libraries/include/glm/ext/vector_bool4.hpp"
    "Libraries/include/glm/ext/vector_bool4_precision.hpp"
    "Libraries/include/glm/ext/vector_common.hpp"
    "Libraries/include/glm/ext/vector_common.inl"
    "Libraries/include/glm/ext/vector_double1.hpp"
    "Libraries/include/glm/ext/vector_double1_precision.hpp"
    "Libraries/include/glm/ext/vector_double2.hpp"
    "Libraries/include/glm/ext/vector_double2_precision.hpp"
    "Libraries/include/glm/ext/vector_double3.hpp"
    "Libraries/include/glm/ext/vector_double3_precision.hpp"
    "Libraries/include/glm/ext/vector_double4.hpp"
    "Libraries/include/glm/ext/vector_double4_precision.hpp"
    "Libraries/include/glm/ext/vector_float1.hpp"
    "Libraries/include/glm/ext/vector_float1_precision.hpp"
    "Libraries/include/glm/ext/vector_float2.hpp"
    "Libraries/include/glm/ext/vector_float2_precision.hpp"
    "Libraries/include/glm/ext/vector_float3.hpp"
    "Libraries/include/glm/ext/vector_float3_precision.hpp"
    "Libraries/include/glm/ext/vector_float4.hpp"
    "Libraries/include/glm/ext/vector_float4_precision.hpp"
    "Libraries/include/glm/ext/vector_int1.hpp"
    "Libraries/include/glm/ext/vector_int1_sized.hpp"
    "Libraries/include/glm/ext/vector_int2.hpp"
    "Libraries/include/glm/ext/vector_int2_sized.hpp"
    "Libraries/include/glm/ext/vector_int3.hpp"
    "Libraries/include/glm/ext/vector_int3_sized.hpp"
    "Libraries/include/glm/ext/vector_int4.hpp"
    "Libraries/include/glm/ext/vector_int4_sized.hpp"
    "Libraries/include/glm/ext/vector_integer.hpp"
    "Libraries/include/glm/ext/vector_integer.inl"
    "Libraries/include/glm/ext/vector_packing.hpp"
    "Libraries/include/glm/ext/vector_packing.inl"
    "Libraries/include/glm/ext/vector_relational.hpp"
    "Libraries/include/glm/ext/vector_relational.inl"
    "Libraries/include/glm/ext/vector_uint1.hpp"
    "Libraries/include/glm/ext/vector_uint1_sized.hpp"
    "Libraries/include/glm/ext/vector_uint2.hpp"
    "Libraries/include/glm/ext/vector_uint2_sized.hpp"
    "Libraries/include/glm/ext/vector_uint3.hpp"
    "Libraries/include/glm/ext/vector_uint3_sized.hpp"
    "Libraries/include/glm/ext/vector_uint4.hpp"
    "Libraries/include/glm/ext/vector_uint4_sized.hpp"
    "Libraries/include/glm/ext/vector_ulp.hpp"
    "Libraries/include/glm/ext/vector_ulp.inl"
    "Libraries/include/glm/fwd.hpp"
    "Libraries/include/glm/geometric.hpp"
    "Libraries/include/glm/glm.hpp"
    "Libraries/include/glm/gtc/bitfield.hpp"
    "Libraries/include/glm/gtc/bitfield.inl"
    "Libraries/include/glm/gtc/color_space.hpp"
    "Libraries/include/glm/gtc/color_space.inl"
    "Libraries/include/glm/gtc/constants.hpp"
    "Libraries/include/glm/gtc/constants.inl"
    "Libraries/include/glm/gtc/epsilon.hpp"
    "Libraries/include/glm/gtc/epsilon.inl"
    "Libraries/include/glm/gtc/integer.hpp"
    "Libraries/include/glm/gtc/integer.inl"
    "Libraries/include/glm/gtc/matrix_access.hpp"
    "Libraries/include/glm/gtc/matrix_access.inl"
    "Libraries/include/glm/gtc/matrix_integer.hpp"
    "Libraries/include/glm/gtc/matrix_inverse.hpp"
    "Libraries/include/glm/gtc/matrix_inverse.inl"
    "Libraries/include/glm/gtc/matrix_transform.hpp"
    "Libraries/include/glm/gtc/matrix_transform.inl"
    "Libraries/include/glm/gtc/noise.hpp"
    "Libraries/include/glm/gtc/noise.inl"
    "Libraries/include/glm/gtc/packing.hpp"
    "Libraries/include/glm/gtc/packing.inl"
    "Libraries/include/glm/gtc/quaternion.hpp"
    "Libraries/include/glm/gtc/quaternion.inl"
    "Libraries/include/glm/gtc/quaternion_simd.inl"
    "Libraries/include/glm/gtc/random.hpp"
    "Libraries/include/glm/gtc/random.inl"
    "Libraries/include/glm/gtc/reciprocal.hpp"
    "Libraries/include/glm/gtc/reciprocal.inl"
    "Libraries/include/glm/gtc/round.hpp"
    "Libraries/include/glm/gtc/round.inl"
    "Libraries/include/glm/gtc/type_aligned.hpp"
    "Libraries/include/glm/gtc/type_precision.hpp"
    "Libraries/include/glm/gtc/type_precision.inl"
    "Libraries/include/glm/gtc/type_ptr.hpp"
    "Libraries/include/glm/gtc/type_ptr.inl"
    "Libraries/include/glm/gtc/ulp.hpp"
    "Libraries/include/glm/gtc/ulp.inl"
    "Libraries/include/glm/gtc/vec1.hpp"
    "Libraries/include/glm/gtx/associated_min_max.hpp"
    "Libraries/include/glm/gtx/associated_min_max.inl"
    "Libraries/include/glm/gtx/bit.hpp"
    "Libraries/include/glm/gtx/bit.inl"
    "Libraries/include/glm/gtx/closest_point.hpp"
    "Libraries/include/glm/gtx/closest_point.inl"
    "Libraries/include/glm/gtx/color_encoding.hpp"
    "Libraries/include/glm/gtx/color_encoding.inl"
    "Libraries/include/glm/gtx/color_space.hpp"
    "Libraries/include/glm/gtx/color_space.inl"
    "Libraries/include/glm/gtx/color_space_YCoCg.hpp"
    "Libraries/include/glm/gtx/color_space_YCoCg.inl"
    "Libraries/include/glm/gtx/common.hpp"
    "Libraries/include/glm/gtx/common.inl"
    "Libraries/include/glm/gtx/compatibility.hpp"
    "Libraries/include/glm/gtx/compatibility.inl"
    "Libraries/include/glm/gtx/component_wise.hpp"
    "Libraries/include/glm/gtx/component_wise.inl"
    "Libraries/include/glm/gtx/dual_quaternion.hpp"
    "Libraries/include/glm/gtx/dual_quaternion.inl"
    "Libraries/include/glm/gtx/easing.hpp"
    "Libraries/include/glm/gtx/easing.inl"
    "Libraries/include/glm/gtx/euler_angles.hpp"
    "Libraries/include/glm/gtx/euler_angles.inl"
    "Libraries/include/glm/gtx/extend.hpp"
    "Libraries/include/glm/gtx/extend.inl"
    "Libraries/include/glm/gtx/extended_min_max.hpp"
    "Libraries/include/glm/gtx/extended_min_max.inl"
    "Libraries/include/glm/gtx/exterior_product.hpp"
    "Libraries/include/glm/gtx/exterior_product.inl"
    "Libraries/include/glm/gtx/fast_exponential.hpp"
    "Libraries/include/glm/gtx/fast_exponential.inl"
    "Libraries/include/glm/gtx/fast_square_root.hpp"
    "Libraries/include/glm/gtx/fast_square_root.inl"
    "Libraries/include/glm/gtx/fast_trigonometry.hpp"
    "Libraries/include/glm/gtx/fast_trigonometry.inl"
    "Libraries/include/glm/gtx/float_notmalize.inl"
    "Libraries/include/glm/gtx/functions.hpp"
    "Libraries/include/glm/gtx/functions.inl"
    "Libraries/include/glm/gtx/gradient_paint.hpp"
    "Libraries/include/glm/gtx/gradient_paint.inl"
    "Libraries/include/glm/gtx/handed_coordinate_space.hpp"
    "Libraries/include/glm/gtx/handed_coordinate_space.inl"
    "Libraries/include/glm/gtx/hash.hpp"
    "Libraries/include/glm/gtx/hash.inl"
    "Libraries/include/glm/gtx/integer.hpp"
    "Libraries/include/glm/gtx/integer.inl"
    "Libraries/include/glm/gtx/intersect.hpp"
    "Libraries/include/glm/gtx/intersect.inl"
    "Libraries/include/glm/gtx/io.hpp"
    "Libraries/include/glm/gtx/io.inl"
    "Libraries/include/glm/gtx/log_base.hpp"
    "Libraries/include/glm/gtx/log_base.inl"
    "Libraries/include/glm/gtx/matrix_cross_product.hpp"
    "Libraries/include/glm/gtx/matrix_cross_product.inl"
    "Libraries/include/glm/gtx/matrix_decompose.hpp"
    "Libraries/include/glm/gtx/matrix_decompose.inl"
    "Libraries/include/glm/gtx/matrix_factorisation.hpp"
    "Libraries/include/glm/gtx/matrix_factorisation.inl"
    "Libraries/include/glm/gtx/matrix_interpolation.hpp"
    "Libraries/include/glm/gtx/matrix_interpolation.inl"
    "Libraries/include/glm/gtx/matrix_major_storage.hpp"
    "Libraries/include/glm/gtx/matrix_major_storage.inl"
    "Libraries/include/glm/gtx/matrix_operation.hpp"
    "Libraries/include/glm/gtx/matrix_operation.inl"
    "Libraries/include/glm/gtx/matrix_query.hpp"
    "Libraries/include/glm/gtx/matrix_query.inl"
    "Libraries/include/glm/gtx/matrix_transform_2d.hpp"
    "Libraries/include/glm/gtx/matrix_transform_2d.inl"
    "Libraries/include/glm/gtx/mixed_product.hpp"
    "Libraries/include/glm/gtx/mixed_product.inl"
    "Libraries/include/glm/gtx/norm.hpp"
    "Libraries/include/glm/gtx/norm.inl"
    "Libraries/include/glm/gtx/normal.hpp"
    "Libraries/include/glm/gtx/normal.inl"
    "Libraries/include/glm/gtx/normalize_dot.hpp"
    "Libraries/include/glm/gtx/normalize_dot.inl"
    "Libraries/include/glm/gtx/number_precision.hpp"
    "Libraries/include/glm/gtx/number_precision.inl"
    "Libraries/include/glm/gtx/optimum_pow.hpp"
    "Libraries/include/glm/gtx/optimum_pow.inl"
    "Libraries/include/glm/gtx/orthonormalize.hpp"
    "Libraries/include/glm/gtx/orthonormalize.inl"
    "Libraries/include/glm/gtx/perpendicular.hpp"
    "Libraries/include/glm/gtx/perpendicular.inl"
    "Libraries/include/glm/gtx/polar_coordinates.hpp"
    "Libraries/include/glm/gtx/polar_coordinates.inl"
    "Libraries/include/glm/gtx/projection.hpp"
    "Libraries/include/glm/gtx/projection.inl"
    "Libraries/include/glm/gtx/quaternion.hpp"
    "Libraries/include/glm/gtx/quaternion.inl"
    "Libraries/include/glm/gtx/range.hpp"
    "Libraries/include/glm/gtx/raw_data.hpp"
    "Libraries/include/glm/gtx/raw_data.inl"
    "Libraries/include/glm/gtx/rotate_normalized_axis.hpp"
    "Libraries/include/glm/gtx/rotate_normalized_axis.inl"
    "Libraries/include/glm/gtx/rotate_vector.hpp"
    "Libraries/include/glm/gtx/rotate_vector.inl"
    "Libraries/include/glm/gtx/scalar_multiplication.hpp"
    "Libraries/include/glm/gtx/scalar_relational.hpp"
    "Libraries/include/glm/gtx/scalar_relational.inl"
    "Libraries/include/glm/gtx/spline.hpp"
    "Libraries/include/glm/gtx/spline.inl"
    "Libraries/include/glm/gtx/std_based_type.hpp"
    "Libraries/include/glm/gtx/std_based_type.inl"
    "Libraries/include/glm/gtx/string_cast.hpp"
    "Libraries/include/glm/gtx/string_cast.inl"
    "Libraries/include/glm/gtx/texture.hpp"
    "Libraries/include/glm/gtx/texture.inl"
    "Libraries/include/glm/gtx/transform.hpp"
    "Libraries/include/glm/gtx/transform.inl"
    "Libraries/include/glm/gtx/transform2.hpp"
    "Libraries/include/glm/gtx/transform2.inl"
    "Libraries/include/glm/gtx/type_aligned.hpp"
    "Libraries/include/glm/gtx/type_aligned.inl"
    "Libraries/include/glm/gtx/type_trait.hpp"
    "Libraries/include/glm/gtx/type_trait.inl"
    "Libraries/include/glm/gtx/vec_swizzle.hpp"
    "Libraries/include/glm/gtx/vector_angle.hpp"
    "Libraries/include/glm/gtx/vector_angle.inl"
    "Libraries/include/glm/gtx/vector_query.hpp"
    "Libraries/include/glm/gtx/vector_query.inl"
    "Libraries/include/glm/gtx/wrap.hpp"
    "Libraries/include/glm/gtx/wrap.inl"
    "Libraries/include/glm/integer.hpp"
    "Libraries/include/glm/mat2x2.hpp"
    "Libraries/include/glm/mat2x3.hpp"
    "Libraries/include/glm/mat2x4.hpp"
    "Libraries/include/glm/mat3x2.hpp"
    "Libraries/include/glm/mat3x3.hpp"
    "Libraries/include/glm/mat3x4.hpp"
    "Libraries/include/glm/mat4x2.hpp"
    "Libraries/include/glm/mat4x3.hpp"
    "Libraries/include/glm/mat4x4.hpp"
    "Libraries/include/glm/matrix.hpp"
    "Libraries/include/glm/packing.hpp"
    "Libraries/include/glm/simd/common.h"
    "Libraries/include/glm/simd/exponential.h"
    "Libraries/include/glm/simd/geometric.h"
    "Libraries/include/glm/simd/integer.h"
    "Libraries/include/glm/simd/matrix.h"
    "Libraries/include/glm/simd/neon.h"
    "Libraries/include/glm/simd/packing.h"
    "Libraries/include/glm/simd/platform.h"
    "Libraries/include/glm/simd/trigonometric.h"
    "Libraries/include/glm/simd/vector_relational.h"
    "Libraries/include/glm/trigonometric.hpp"
    "Libraries/include/glm/vec2.hpp"
    "Libraries/include/glm/vec3.hpp"
    "Libraries/include/glm/vec4.hpp"
    "Libraries/include/glm/vector_relational.hpp"
    "Libraries/include/imgui/imconfig.h"
    "Libraries/include/imgui/imgui.h"
    "Libraries/include/imgui/imgui_impl_glfw.h"
    "Libraries/include/imgui/imgui_impl_opengl3.h"
    "Libraries/include/imgui/imgui_impl_opengl3_loader.h"
    "Libraries/include/imgui/imgui_internal.h"
    "Libraries/include/imgui/imgui_stdlib.h"
    "Libraries/include/imgui/imstb_rectpack.h"
    "Libraries/include/imgui/imstb_textedit.h"
    "Libraries/include/imgui/imstb_truetype.h"
    "Libraries/include/imgui/imgui_internal.h"
    "Libraries/include/imgui/imgui_impl_glfw.h"
    "Libraries/include/KHR/khrplatform.h"
    "Libraries/include/vk_platform.h"
    "Libraries/include/miniaudio/miniaudio.h"
    "Libraries/include/fastnoiselite/FastNoiseLite.h"
    "Libraries/include/stb/stb_image.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "dat/cosmetic.cpp"
    "dat/nameGenerate.cpp"
    "dat/crafting.cpp"
    "dat/entities.cpp"
    "dat/game.cpp"
    "Libraries/include/antibox/core/audio.cpp"
    "Libraries/include/antibox/core/engine.cpp"
    "Libraries/include/antibox/core/imguiwindow.cpp"
    "Libraries/include/antibox/core/log.cpp"
    "Libraries/include/antibox/core/window.cpp"
    "Libraries/include/antibox/graphics/3d/camera.cpp"
    "Libraries/include/antibox/graphics/framebuffer.cpp"
    "Libraries/include/antibox/graphics/helpers.cpp"
    "Libraries/include/antibox/graphics/mesh.cpp"
    "Libraries/include/antibox/graphics/rendercommands.cpp"
    "Libraries/include/antibox/graphics/shader.cpp"
    "Libraries/include/antibox/graphics/sprite.cpp"
    "Libraries/include/antibox/graphics/texture.cpp"
    "Libraries/include/antibox/graphics/vertex.cpp"
    "Libraries/include/antibox/managers/rendermanager.cpp"
    "Libraries/include/antibox/objects/3d/model.cpp"
    "Libraries/include/antibox/objects/gameobject.cpp"
    "Libraries/include/antibox/objects/scene.cpp"
    "Libraries/include/antibox/tools/noise.cpp"
    "Libraries/include/glad/glad.c"
    "Libraries/include/glm/detail/glm.cpp"
    "Libraries/include/imgui/imgui.cpp"
    "Libraries/include/imgui/imgui_demo.cpp"
    "Libraries/include/imgui/imgui_draw.cpp"
    "Libraries/include/imgui/imgui_impl_glfw.cpp"
    "Libraries/include/imgui/imgui_impl_opengl3.cpp"
    "Libraries/include/imgui/imgui_stdlib.cpp"
    "Libraries/include/imgui/imgui_tables.cpp"
    "Libraries/include/imgui/imgui_widgets.cpp"
    "Libraries/include/imgui/imgui_impl_opengl3.h"
    "main.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${no_group_source_files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES} "Libraries/include/antibox/server/client.cpp" "Libraries/include/antibox/server/server.cpp"  "Libraries/include/antibox/tools/pathfinding.h")

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE runicdepths)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################

#find_package(Freetype REQUIRED)

if(WIN32)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}"
            OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
        )
    endif()
elseif(UNIX)
    # Set output directories for Linux
    set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}"
            OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
        )
endif()

################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
if(WIN32)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
        target_include_directories(${PROJECT_NAME} PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/include"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_include_directories(${PROJECT_NAME} PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/include"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_include_directories(${PROJECT_NAME} PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/include"
        )
    endif()
elseif(UNIX)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/include"
    )
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
endif()


################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++20;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    endif()
elseif(UNIX)  # This block is for Linux (e.g., WSL or native Linux)
    # GCC/Clang-specific compile options for WSL or Linux
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            -O2;         # Optimization for Release
            -DNDEBUG;    # Define NDEBUG for Release
        >
        $<$<CONFIG:Debug>:
            -O0;         # No optimization for Debug
            -g;          # Enable debug symbols
        >

        # Common flags for Linux
        -std=c++20;     # Use C++20 standard
        -Wall;          # Enable most warnings
        -Wextra;        # Enable additional warnings
        -fcf-protection=none;
    )
endif()

################################################################################
# Dependencies
################################################################################
if(WIN32)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
        set(ADDITIONAL_LIBRARY_DEPENDENCIES
            "glfw3;"
            "opengl32"
            #"Freetype::Freetype"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        set(ADDITIONAL_LIBRARY_DEPENDENCIES
            "glfw3;"
            "opengl32"
            #"Freetype::Freetype"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        set(ADDITIONAL_LIBRARY_DEPENDENCIES
            "glfw3;"
            "opengl32"
            #"Freetype::Freetype"
        )
    endif()
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    set(GLFW_STATIC ON)
    find_package(glfw3 REQUIRED)

    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        glfw
        OpenGL::GL
        X11
        pthread
        Xrandr
        Xi
    )
endif()

# Apply the dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/Libraries/lib"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/Libraries/lib"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/Libraries/lib"
    )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
