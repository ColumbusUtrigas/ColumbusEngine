# file manipulation functions list:
# add_dir_to_target
# add_dir_to_target_recurse
# group_files_by_directory
# dir_to_var
# dir_to_var_recurse
# add_subdirectory_with_folder

# shader functions list:
# compile_shader_glsl
# compile_shader_glsl_permutation
# compile_shader_glsl_permutation_ranges
# compile_shader_hlsl
# compile_shader_hlsl_permutation
# compile_shader_hlsl_permutation_ranges
#### require SPIRV_OUTPUT_DIR and DXC_PATH to be set

########### FILE MANIPULATION FUNCTIONS ###########
function(group_files_by_directory)
	cmake_parse_arguments(GROUP_FILES "" "" "SRCS" ${ARGN})

	foreach(FILE ${GROUP_FILES_SRCS})
		# Get the directory of the source file
		get_filename_component(PARENT_DIR ${FILE} DIRECTORY)

		# Remove common directory prefix to make the group
		string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" GROUP ${PARENT_DIR})

		# Make sure we are using windows slashes
		# string(REPLACE "/" "\\" GROUP ${GROUP})

		# Group into "Source Files" and "Header Files"
		#    if ("${FILE}" MATCHES ".*\\.cpp")
		#       set(GROUP "Source Files${GROUP}")
		#    elseif("${FILE}" MATCHES ".*\\.h")
		#       set(GROUP "Header Files${GROUP}")
		#    endif()

		source_group(${GROUP} FILES ${FILE})
	endforeach()
endfunction()

function(dir_to_var PATH SOURCES)
	file(GLOB TMP_SRC ${PATH}/*.cpp ${PATH}/*.c ${PATH}/*.hpp ${PATH}/*.h)
	group_files_by_directory(SRCS ${TMP_SRC})
	set(${SOURCES} ${TMP_SRC} PARENT_SCOPE)
endfunction()

function(dir_to_var_recurse PATH SOURCES)
	file(GLOB_RECURSE TMP_SRC ${PATH}/*.cpp ${PATH}/*.c ${PATH}/*.hpp ${PATH}/*.h)
	group_files_by_directory(SRCS ${TMP_SRC})
	set(${SOURCES} ${TMP_SRC} PARENT_SCOPE)
endfunction()

function(add_dir_to_target TARGET PATH)
    file(GLOB TMP_SRC ${PATH}/*.cpp ${PATH}/*.c)
    file(GLOB TMP_HDR ${PATH}/*.hpp ${PATH}/*.h)
    target_sources(${TARGET} PRIVATE ${TMP_SRC})
    target_sources(${TARGET} PRIVATE ${TMP_HDR})
    group_files_by_directory(SRCS ${TMP_SRC})
    group_files_by_directory(SRCS ${TMP_HDR})
endfunction()

function(add_dir_to_target_recurse TARGET PATH)
	file(GLOB_RECURSE TMP_SRC ${PATH}/*.cpp ${PATH}/*.c ${PATH}/*.hpp ${PATH}/*.h)
	target_sources(${TARGET} PRIVATE ${TMP_SRC})
	group_files_by_directory(SRCS ${TMP_SRC})
endfunction()

########### SHADER FUNCTIONS ###########
function(compile_shader_impl shaderName stages outputStem binaryLocation)
	set(DEFINES ${ARGN})
	get_filename_component(STEM ${shaderName} NAME_WLE)
	get_filename_component(SHADER_PATH ${shaderName} ABSOLUTE)
	get_filename_component(DIR ${shaderName} DIRECTORY)
	file(RELATIVE_PATH SHADER_PATH_REL ${CMAKE_CURRENT_SOURCE_DIR} ${SHADER_PATH})

	if ("${outputStem}" STREQUAL "")
		set(OUTPUT_STEM "${STEM}")
	else()
		set(OUTPUT_STEM "${outputStem}")
	endif()

	set(SPIRV "${SPIRV_OUTPUT_DIR}${DIR}/${OUTPUT_STEM}.csd")
	cmake_path(GET SPIRV PARENT_PATH SPIRV_DIR)

	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_OUTPUT_DIR}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_DIR}"
		COMMAND ${CMAKE_COMMAND} -E env DXC_PATH=${DXC_PATH} "${BINARIES_DIR}/$<CONFIG>/ShaderCompiler" ${SPIRV} ${SHADER_PATH} ${stages} ${DEFINES}
		DEPENDS ${shaderName} ShaderCompiler
	)

	# just a weird way to append a value to a list in the parent scope
	set(TMP_LIST ${${binaryLocation}})
	list(APPEND TMP_LIST ${SPIRV})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)

endfunction(compile_shader_impl)

function(compile_shader_glsl shaderName stages binaryLocation)
	compile_shader_impl(${shaderName} ${stages} "" ${binaryLocation})

	set(TMP_LIST ${${binaryLocation}})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)
endfunction(compile_shader_glsl)

function(compile_shader_glsl_permutation shaderName stages permutationName binaryLocation)
	set(DEFINES ${ARGN})
	get_filename_component(STEM ${shaderName} NAME_WLE)
	compile_shader_impl(${shaderName} ${stages} "${STEM}_${permutationName}" ${binaryLocation} ${DEFINES})

	set(TMP_LIST ${${binaryLocation}})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)
endfunction(compile_shader_glsl_permutation)

function(compile_shader_permutation_ranges_impl shaderName stages binaryLocation)
	set(AXES ${ARGN})
	list(LENGTH AXES AXES_LENGTH)
	math(EXPR AXES_REMAINDER "${AXES_LENGTH} % 3")

	if (NOT AXES_REMAINDER EQUAL 0)
		message(FATAL_ERROR "Shader permutation ranges must be specified as triples: MACRO MIN MAX")
	endif()

	get_filename_component(STEM ${shaderName} NAME_WLE)
	get_filename_component(SHADER_PATH ${shaderName} ABSOLUTE)
	get_filename_component(DIR ${shaderName} DIRECTORY)
	set(PERMUTATIONS "__EMPTY__|__EMPTY__")
	set(PERMUTATION_AXIS_ARGS)

	set(AXIS_INDEX 0)
	while(AXIS_INDEX LESS AXES_LENGTH)
		list(GET AXES ${AXIS_INDEX} MACRO_NAME)
		math(EXPR MIN_INDEX "${AXIS_INDEX} + 1")
		math(EXPR MAX_INDEX "${AXIS_INDEX} + 2")
		list(GET AXES ${MIN_INDEX} MIN_VALUE)
		list(GET AXES ${MAX_INDEX} MAX_VALUE)

		list(APPEND PERMUTATION_AXIS_ARGS --permutation-axis ${MACRO_NAME} ${MIN_VALUE} ${MAX_VALUE})

		set(NEXT_PERMUTATIONS)
		foreach(PERMUTATION ${PERMUTATIONS})
			string(REPLACE "|" ";" PERMUTATION_PARTS "${PERMUTATION}")
			list(GET PERMUTATION_PARTS 0 PERMUTATION_SUFFIX)
			list(GET PERMUTATION_PARTS 1 PERMUTATION_DEFINES)

			foreach(VALUE RANGE ${MIN_VALUE} ${MAX_VALUE})
				if ("${PERMUTATION_SUFFIX}" STREQUAL "__EMPTY__")
					set(NEXT_SUFFIX "${MACRO_NAME}_${VALUE}")
					set(NEXT_DEFINES "${MACRO_NAME}=${VALUE}")
				else()
					set(NEXT_SUFFIX "${PERMUTATION_SUFFIX}_${MACRO_NAME}_${VALUE}")
					set(NEXT_DEFINES "${PERMUTATION_DEFINES},${MACRO_NAME}=${VALUE}")
				endif()

				list(APPEND NEXT_PERMUTATIONS "${NEXT_SUFFIX}|${NEXT_DEFINES}")
			endforeach()
		endforeach()

		set(PERMUTATIONS ${NEXT_PERMUTATIONS})
		math(EXPR AXIS_INDEX "${AXIS_INDEX} + 3")
	endwhile()

	set(PERMUTATION_ARGS)
	foreach(PERMUTATION ${PERMUTATIONS})
		string(REPLACE "|" ";" PERMUTATION_PARTS "${PERMUTATION}")
		list(GET PERMUTATION_PARTS 0 PERMUTATION_SUFFIX)
		list(GET PERMUTATION_PARTS 1 PERMUTATION_DEFINES)
		string(REPLACE "," ";" DEFINES "${PERMUTATION_DEFINES}")

		list(APPEND PERMUTATION_ARGS --permutation ${PERMUTATION_SUFFIX} ${DEFINES})
	endforeach()

	set(SPIRV "${SPIRV_OUTPUT_DIR}${DIR}/${STEM}.csd")
	cmake_path(GET SPIRV PARENT_PATH SPIRV_DIR)

	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_OUTPUT_DIR}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_DIR}"
		COMMAND ${CMAKE_COMMAND} -E env DXC_PATH=${DXC_PATH} "${BINARIES_DIR}/$<CONFIG>/ShaderCompiler" ${SPIRV} ${SHADER_PATH} ${stages} ${PERMUTATION_AXIS_ARGS} ${PERMUTATION_ARGS}
		DEPENDS ${shaderName} ShaderCompiler
	)

	set(TMP_LIST ${${binaryLocation}})
	list(APPEND TMP_LIST ${SPIRV})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)
endfunction(compile_shader_permutation_ranges_impl)

function(compile_shader_glsl_permutation_ranges shaderName stages binaryLocation)
	compile_shader_permutation_ranges_impl(${shaderName} ${stages} ${binaryLocation} ${ARGN})

	set(TMP_LIST ${${binaryLocation}})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)
endfunction(compile_shader_glsl_permutation_ranges)

function(compile_shader_hlsl shaderName stages binaryLocation)
	compile_shader_impl(${shaderName} ${stages} "" ${binaryLocation})

	set(TMP_LIST ${${binaryLocation}})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)
endfunction(compile_shader_hlsl)

function(compile_shader_hlsl_permutation shaderName stages permutationName binaryLocation)
	set(DEFINES ${ARGN})
	get_filename_component(STEM ${shaderName} NAME_WLE)
	compile_shader_impl(${shaderName} ${stages} "${STEM}_${permutationName}" ${binaryLocation} ${DEFINES})

	set(TMP_LIST ${${binaryLocation}})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)
endfunction(compile_shader_hlsl_permutation)

function(compile_shader_hlsl_permutation_ranges shaderName stages binaryLocation)
	compile_shader_permutation_ranges_impl(${shaderName} ${stages} ${binaryLocation} ${ARGN})

	set(TMP_LIST ${${binaryLocation}})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)
endfunction(compile_shader_hlsl_permutation_ranges)

function(get_all_targets _result _dir)
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        get_all_targets(${_result} "${_subdir}")
    endforeach()
    get_property(_sub_targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

function(add_subdirectory_with_folder _folder_name _folder)
    add_subdirectory(${_folder} ${ARGN})
    get_all_targets(_targets "${_folder}")
    foreach(_target IN LISTS _targets)
        set_target_properties(
            ${_target}
            PROPERTIES FOLDER "${_folder_name}"
        )
    endforeach()
endfunction()
