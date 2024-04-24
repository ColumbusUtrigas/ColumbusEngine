# file manipulation functions list:
# add_dir_to_target
# add_dir_to_target_recurse
# group_files_by_directory
# dir_to_var
# dir_to_var_recurse

# shader functions list:
# compile_shader_glsl
# compile_shader_hlsl
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
function(compile_shader_glsl shaderName stages binaryLocation)
	get_filename_component(STEM ${shaderName} NAME_WLE)
	get_filename_component(SHADER_PATH ${shaderName} ABSOLUTE)
	get_filename_component(DIR ${shaderName} DIRECTORY)
	file(RELATIVE_PATH SHADER_PATH_REL ${CMAKE_CURRENT_SOURCE_DIR} ${SHADER_PATH})

	set(SPIRV "${SPIRV_OUTPUT_DIR}${DIR}/${STEM}.csd")
	cmake_path(GET SPIRV PARENT_PATH SPIRV_DIR)

	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_OUTPUT_DIR}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_DIR}"
		COMMAND ${CMAKE_COMMAND} -E env DXC_PATH=${DXC_PATH} "$<TARGET_FILE_DIR:${TARGET}>/ShaderCompiler" ${SPIRV} ${SHADER_PATH} ${stages} 
		# COMMAND ${CMAKE_COMMAND} -E copy ${SPIRV} "$<TARGET_FILE_DIR:${TARGET}>${DIR}/${STEM}.csd"
		DEPENDS ${shaderName} ShaderCompiler
	)

	# just a weird way to append a value to a list in the parent scope
	set(TMP_LIST ${${binaryLocation}})
	list(APPEND TMP_LIST ${SPIRV})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)

endfunction(compile_shader_glsl)

function(compile_shader_hlsl shaderName stages binaryLocation)
	get_filename_component(STEM ${shaderName} NAME_WLE)
	get_filename_component(SHADER_PATH ${shaderName} ABSOLUTE)
	get_filename_component(DIR ${shaderName} DIRECTORY)
	file(RELATIVE_PATH SHADER_PATH_REL ${CMAKE_CURRENT_SOURCE_DIR} ${SHADER_PATH})

	set(SPIRV "${SPIRV_OUTPUT_DIR}${DIR}/${STEM}.csd")
	cmake_path(GET SPIRV PARENT_PATH SPIRV_DIR)

	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_OUTPUT_DIR}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_DIR}"
		COMMAND ${CMAKE_COMMAND} -E env DXC_PATH=${DXC_PATH} "$<TARGET_FILE_DIR:${TARGET}>/ShaderCompiler" ${SPIRV} ${SHADER_PATH} ${stages} 
		# COMMAND ${CMAKE_COMMAND} -E copy ${SPIRV} "$<TARGET_FILE_DIR:${TARGET}>${DIR}/${STEM}.csd"
		DEPENDS ${shaderName} ShaderCompiler
	)

	# just a weird way to append a value to a list in the parent scope
	set(TMP_LIST ${${binaryLocation}})
	list(APPEND TMP_LIST ${SPIRV})
	set(${binaryLocation} "${TMP_LIST}" PARENT_SCOPE)

endfunction(compile_shader_hlsl)