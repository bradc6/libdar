#Use the lastest Cmake policy on FindCmake Includes
cmake_policy(SET CMP0017 NEW)

# Libdar provides a complete abstraction layer 
#for handling Disk ARchive (dar)'s archives.

project(dar)
cmake_minimum_required(VERSION 2.8.6)

#The directory where the cmake configuration and modules are located
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
                      ${CMAKE_MODULE_PATH})

find_package(Doxygen)
find_package(ZLIB)
find_package(BZip2)
find_package(LZO)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIBDAR_SOURCE_DIR ${SOURCE_DIR}/libdar)

set(libdar-Source
	${LIBDAR_SOURCE_DIR}/archive.cpp
	${LIBDAR_SOURCE_DIR}/archive.hpp
	${LIBDAR_SOURCE_DIR}/archive_options.cpp
	${LIBDAR_SOURCE_DIR}/archive_options.hpp
	${LIBDAR_SOURCE_DIR}/archive_version.cpp
	${LIBDAR_SOURCE_DIR}/archive_version.hpp
	${LIBDAR_SOURCE_DIR}/cache.cpp
	${LIBDAR_SOURCE_DIR}/cache.hpp
	${LIBDAR_SOURCE_DIR}/catalogue.cpp
	${LIBDAR_SOURCE_DIR}/catalogue.hpp
	${LIBDAR_SOURCE_DIR}/compile_time_features.cpp
	${LIBDAR_SOURCE_DIR}/compile_time_features.hpp
	${LIBDAR_SOURCE_DIR}/compressor.cpp
	${LIBDAR_SOURCE_DIR}/compressor.hpp
	${LIBDAR_SOURCE_DIR}/crc.cpp
	${LIBDAR_SOURCE_DIR}/crc.hpp
	${LIBDAR_SOURCE_DIR}/criterium.cpp
	${LIBDAR_SOURCE_DIR}/criterium.hpp
	${LIBDAR_SOURCE_DIR}/crypto.cpp
	${LIBDAR_SOURCE_DIR}/crypto.hpp
	${LIBDAR_SOURCE_DIR}/cygwin_adapt.hpp
	${LIBDAR_SOURCE_DIR}/data_tree.cpp
	${LIBDAR_SOURCE_DIR}/data_tree.hpp
	${LIBDAR_SOURCE_DIR}/database.cpp
	${LIBDAR_SOURCE_DIR}/database.hpp
	${LIBDAR_SOURCE_DIR}/database_header.cpp
	${LIBDAR_SOURCE_DIR}/database_header.hpp
	${LIBDAR_SOURCE_DIR}/database_options.hpp
	${LIBDAR_SOURCE_DIR}/deci.cpp
	${LIBDAR_SOURCE_DIR}/deci.hpp
	${LIBDAR_SOURCE_DIR}/defile.cpp
	${LIBDAR_SOURCE_DIR}/defile.hpp
	${LIBDAR_SOURCE_DIR}/ea.cpp
	${LIBDAR_SOURCE_DIR}/ea.hpp
	${LIBDAR_SOURCE_DIR}/ea_filesystem.cpp
	${LIBDAR_SOURCE_DIR}/ea_filesystem.hpp
	${LIBDAR_SOURCE_DIR}/elastic.cpp
	${LIBDAR_SOURCE_DIR}/elastic.hpp
	${LIBDAR_SOURCE_DIR}/erreurs.cpp
	${LIBDAR_SOURCE_DIR}/erreurs.hpp
	${LIBDAR_SOURCE_DIR}/erreurs_ext.hpp
	${LIBDAR_SOURCE_DIR}/escape.cpp
	${LIBDAR_SOURCE_DIR}/escape.hpp
	${LIBDAR_SOURCE_DIR}/escape_catalogue.cpp
	${LIBDAR_SOURCE_DIR}/escape_catalogue.hpp
	${LIBDAR_SOURCE_DIR}/etage.cpp
	${LIBDAR_SOURCE_DIR}/etage.hpp
	${LIBDAR_SOURCE_DIR}/fichier.cpp
	${LIBDAR_SOURCE_DIR}/fichier.hpp
	${LIBDAR_SOURCE_DIR}/filesystem.cpp
	${LIBDAR_SOURCE_DIR}/filesystem.hpp
	${LIBDAR_SOURCE_DIR}/filtre.cpp
	${LIBDAR_SOURCE_DIR}/filtre.hpp
	${LIBDAR_SOURCE_DIR}/generic_file.cpp
	${LIBDAR_SOURCE_DIR}/generic_file.hpp
	${LIBDAR_SOURCE_DIR}/hash_fichier.cpp
	${LIBDAR_SOURCE_DIR}/hash_fichier.hpp
	${LIBDAR_SOURCE_DIR}/header.cpp
	${LIBDAR_SOURCE_DIR}/header.hpp
	${LIBDAR_SOURCE_DIR}/header_version.cpp
	${LIBDAR_SOURCE_DIR}/header_version.hpp
	${LIBDAR_SOURCE_DIR}/infinint.hpp
	${LIBDAR_SOURCE_DIR}/int_tools.cpp
	${LIBDAR_SOURCE_DIR}/int_tools.hpp
	${LIBDAR_SOURCE_DIR}/integers.cpp
	${LIBDAR_SOURCE_DIR}/integers.hpp
	${LIBDAR_SOURCE_DIR}/label.cpp
	${LIBDAR_SOURCE_DIR}/label.hpp
	${LIBDAR_SOURCE_DIR}/libdar.cpp
	${LIBDAR_SOURCE_DIR}/libdar.hpp
	${LIBDAR_SOURCE_DIR}/libdar_4_4.cpp
	${LIBDAR_SOURCE_DIR}/libdar_4_4.hpp
	${LIBDAR_SOURCE_DIR}/limitint.cpp
	${LIBDAR_SOURCE_DIR}/limitint.hpp
	${LIBDAR_SOURCE_DIR}/list_entry.cpp
	${LIBDAR_SOURCE_DIR}/list_entry.hpp
	${LIBDAR_SOURCE_DIR}/macro_tools.cpp
	${LIBDAR_SOURCE_DIR}/macro_tools.hpp
	${LIBDAR_SOURCE_DIR}/mask.cpp
	${LIBDAR_SOURCE_DIR}/mask.hpp
	${LIBDAR_SOURCE_DIR}/mask_list.cpp
	${LIBDAR_SOURCE_DIR}/mask_list.hpp
	${LIBDAR_SOURCE_DIR}/mem_ui.cpp
	${LIBDAR_SOURCE_DIR}/mem_ui.hpp
	${LIBDAR_SOURCE_DIR}/memory_check.cpp
	${LIBDAR_SOURCE_DIR}/memory_check.hpp
	${LIBDAR_SOURCE_DIR}/memory_file.cpp
	${LIBDAR_SOURCE_DIR}/memory_file.hpp
	${LIBDAR_SOURCE_DIR}/nls_swap.hpp
	${LIBDAR_SOURCE_DIR}/null_file.hpp
	${LIBDAR_SOURCE_DIR}/path.cpp
	${LIBDAR_SOURCE_DIR}/path.hpp
	${LIBDAR_SOURCE_DIR}/pile.cpp
	${LIBDAR_SOURCE_DIR}/pile.hpp
	${LIBDAR_SOURCE_DIR}/real_infinint.cpp
	${LIBDAR_SOURCE_DIR}/real_infinint.hpp
	${LIBDAR_SOURCE_DIR}/sar.cpp
	${LIBDAR_SOURCE_DIR}/sar.hpp
	${LIBDAR_SOURCE_DIR}/sar_tools.cpp
	${LIBDAR_SOURCE_DIR}/sar_tools.hpp
	${LIBDAR_SOURCE_DIR}/scrambler.cpp
	${LIBDAR_SOURCE_DIR}/scrambler.hpp
	${LIBDAR_SOURCE_DIR}/secu_string.cpp
	${LIBDAR_SOURCE_DIR}/secu_string.hpp
	${LIBDAR_SOURCE_DIR}/semaphore.cpp
	${LIBDAR_SOURCE_DIR}/semaphore.hpp
	${LIBDAR_SOURCE_DIR}/sparse_file.cpp
	${LIBDAR_SOURCE_DIR}/sparse_file.hpp
	${LIBDAR_SOURCE_DIR}/special_alloc.cpp
	${LIBDAR_SOURCE_DIR}/special_alloc.hpp
	${LIBDAR_SOURCE_DIR}/statistics.cpp
	${LIBDAR_SOURCE_DIR}/statistics.hpp
	${LIBDAR_SOURCE_DIR}/storage.cpp
	${LIBDAR_SOURCE_DIR}/storage.hpp
	${LIBDAR_SOURCE_DIR}/string_file.cpp
	${LIBDAR_SOURCE_DIR}/string_file.hpp
	${LIBDAR_SOURCE_DIR}/terminateur.cpp
	${LIBDAR_SOURCE_DIR}/terminateur.hpp
	${LIBDAR_SOURCE_DIR}/thread_cancellation.cpp
	${LIBDAR_SOURCE_DIR}/thread_cancellation.hpp
	${LIBDAR_SOURCE_DIR}/tlv.cpp
	${LIBDAR_SOURCE_DIR}/tlv.hpp
	${LIBDAR_SOURCE_DIR}/tlv_list.cpp
	${LIBDAR_SOURCE_DIR}/tlv_list.hpp
	${LIBDAR_SOURCE_DIR}/tools.cpp
	${LIBDAR_SOURCE_DIR}/tools.hpp
	${LIBDAR_SOURCE_DIR}/tronc.cpp
	${LIBDAR_SOURCE_DIR}/tronc.hpp
	${LIBDAR_SOURCE_DIR}/tronconneuse.cpp
	${LIBDAR_SOURCE_DIR}/tronconneuse.hpp
	${LIBDAR_SOURCE_DIR}/trontextual.cpp
	${LIBDAR_SOURCE_DIR}/trontextual.hpp
	${LIBDAR_SOURCE_DIR}/tuyau.cpp
	${LIBDAR_SOURCE_DIR}/tuyau.hpp
	${LIBDAR_SOURCE_DIR}/user_group_bases.cpp
	${LIBDAR_SOURCE_DIR}/user_group_bases.hpp
	${LIBDAR_SOURCE_DIR}/user_interaction.cpp
	${LIBDAR_SOURCE_DIR}/user_interaction.hpp
	${LIBDAR_SOURCE_DIR}/wrapperlib.cpp
	${LIBDAR_SOURCE_DIR}/wrapperlib.hpp
	${LIBDAR_SOURCE_DIR}/zapette.cpp
	${LIBDAR_SOURCE_DIR}/zapette.hpp
	)

add_library(dar ${libdar-Source})
