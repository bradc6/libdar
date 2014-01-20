#Use the lastest Cmake policy on FindCmake Includes
cmake_policy(SET CMP0017 NEW)

# Libdar provides a complete abstraction layer 
#for handling Disk ARchive (dar)'s archives.

project(dar)
cmake_minimum_required(VERSION 2.8.6)

#The directory where the cmake configuration and modules are located
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
                      ${CMAKE_MODULE_PATH})

INCLUDE(CheckIncludeFiles)
#Set the current version of the library
set(LIBDAR_MAJOR_VERSION 2)
set(LIBDAR_MINOR_VERSION 4)
set(LIBDAR_PATCH_VERSION 12)

option(DISABLEDOXYGEN "Disable Doxygen reference documentation from being built" OFF)
option(DISABLEZLIB "Disable ZLIB compression support" OFF)
option(DISABLEBZIP2"Disable BZIP2 compression support" OFF)
option(DISABLELZO "Disable LZO compression support" OFF)

option(NATIVELANGUAGE "Request for Native Language" ON)
if(NATIVELANGUAGE)
	add_definitions(-DENABLE_NLS)
endif(NATIVELANGUAGE) 

option(EXTENDEDFILEATTRIBUTES "Enable the use of extended file attributes" OFF)
if(EXTENDEDFILEATTRIBUTES)
	add_definitions(-DEA_SUPPORT)
endif(EXTENDEDFILEATTRIBUTES)

#Find optional dependancies
if(NOT DISABLEDOXYGEN)
	find_package(Doxygen)
endif()

if(NOT DISABLEZLIB)
	find_package(ZLIB)
endif()

if(NOT DISABLEBZIP2)
	find_package(BZip2)
endif()

if(NOT DISABLELZO)
	find_package(LZO)
endif()

#Check for mutliple include file
#Hopefully there is a faster way to do this
check_include_files("string.h" HAVE_STRING_H)
if(HAVE_STRING_H)
	add_definitions(-DHAVE_STRING_H)
endif(HAVE_STRING_H)

check_include_files("strings.h" HAVE_STRINGS_H)
if(HAVE_STRINGS_H)
	add_definitions(-DHAVE_STRINGS_H)
endif(HAVE_STRINGS_H)

check_include_files("ctype.h" HAVE_CTYPE_H)
if(HAVE_CTYPE_H)
	add_definitions(-DHAVE_CTYPE_H)
endif(HAVE_CTYPE_H)

check_include_files("sys/ioctl.h" HAVE_SYS_IOCTL_H)
if(HAVE_SYS_IOCTL_H)
	add_definitions(-DHAVE_SYS_IOCTL_H)
endif(HAVE_SYS_IOCTL_H)

check_include_files("sys/ndir.h" HAVE_SYS_NDIR_H)
if(HAVE_SYS_NDIR_H)
	add_definitions(-DHAVE_SYS_NDIR_H)
endif(HAVE_SYS_NDIR_H)

check_include_files("sys/dir.h" HAVE_SYS_DIR_H)
if(HAVE_SYS_DIR_H)
	add_definitions(-DHAVE_SYS_DIR_H)
endif(HAVE_SYS_DIR_H)

check_include_files("sys/socket.h" HAVE_SYS_SOCKET_H)
if(HAVE_SYS_SOCKET_H)
	add_definitions(-DHAVE_SYS_SOCKET_H)
endif(HAVE_SYS_SOCKET_H)

check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
if(HAVE_SYS_STAT_H)
	add_definitions(-DHAVE_SYS_STAT_H)
endif(HAVE_SYS_STAT_H)

check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
if(HAVE_SYS_TYPES_H)
	add_definitions(-DHAVE_SYS_TYPES_H)
endif(HAVE_SYS_TYPES_H)

check_include_files("sys/un.h" HAVE_SYS_UN_H)
if(HAVE_SYS_UN_H)
	add_definitions(-DHAVE_SYS_UN_H)
endif(HAVE_SYS_UN_H)

check_include_files("sys/utsname.h" HAVE_SYS_UTSNAME_H)
if(HAVE_SYS_UTSNAME_H)
	add_definitions(-DHAVE_SYS_UTSNAME_H)
endif(HAVE_SYS_UTSNAME_H)

check_include_files("sys/wait.h" HAVE_SYS_WAIT_H)
if(HAVE_SYS_WAIT_H)
	add_definitions(-DHAVE_SYS_WAIT_H)
endif(HAVE_SYS_WAIT_H)

check_include_files("sys/xattr.h" HAVE_SYS_XATTR_H)
if(HAVE_SYS_XATTR_H)
	add_definitions(-DHAVE_SYS_XATTR_H)
endif(HAVE_SYS_XATTR_H)

check_include_files("attr/xattr.h" HAVE_ATTR_XATTR_H)
if(HAVE_ATTR_XATTR_H)
	add_definitions(-DHAVE_ATTR_XATTR_H)
endif(HAVE_ATTR_XATTR_H)

check_include_files("termios.h" HAVE_TERMIOS_H)
if(HAVE_TERMIOS_H)
	add_definitions(-DHAVE_TERMIOS_H)
endif(HAVE_TERMIOS_H)

check_include_files("time.h" HAVE_TIME_H)
if(HAVE_TIME_H)
	add_definitions(-DHAVE_TIME_H)
endif(HAVE_TIME_H)

check_include_files("unistd.h" HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
	add_definitions(-DHAVE_UNISTD_H)
endif(HAVE_UNISTD_H)

check_include_files("utime.h" HAVE_UTIME_H)
if(HAVE_UTIME_H)
	add_definitions(-DHAVE_UTIME_H)
endif(HAVE_UTIME_H)

check_include_files("dirent.h" HAVE_DIRENT_H)
if(HAVE_DIRENT_H)
	add_definitions(-DHAVE_DIRENT_H)
endif(HAVE_DIRENT_H)

check_include_files("dlfcn.h" HAVE_DLFCN_H)
if(HAVE_DLFCN_H)
	add_definitions(-DHAVE_DLFCN_H)
endif(HAVE_DLFCN_H)

check_include_files("errno.h" HAVE_ERRNO_H)
if(HAVE_ERRNO_H)
	add_definitions(-DHAVE_ERRNO_H)
endif(HAVE_ERRNO_H)

check_include_files("execinfo.h" HAVE_EXECINFO_H)
if(HAVE_EXECINFO_H)
	add_definitions(-DHAVE_EXECINFO_H)
endif(HAVE_EXECINFO_H)

check_include_files("fcntl.h" HAVE_FCNTL_H)
if(HAVE_FCNTL_H)
	add_definitions(-DHAVE_FCNTL_H)
endif(HAVE_FCNTL_H)

check_include_files("fnmatch.h" HAVE_FNMATCH_H)
if(HAVE_FNMATCH_H)
	add_definitions(-DHAVE_FNMATCH_H)
endif(HAVE_FNMATCH_H)

check_include_files("grp.h" HAVE_GRP_H)
if(HAVE_GRP_H)
	add_definitions(-DHAVE_GRP_H)
endif(HAVE_GRP_H)

check_include_files("inttypes.h" HAVE_INTTYPES_H)
if(HAVE_INTTYPES_H)
	add_definitions(-DHAVE_INTTYPES_H)
endif(HAVE_INTTYPES_H)

check_include_files("limits.h" HAVE_LIMITS_H)
if(HAVE_LIMITS_H)
	add_definitions(-DHAVE_LIMITS_H)
endif(HAVE_LIMITS_H)

check_include_files("memory.h" HAVE_MEMORY_H)
if(HAVE_MEMORY_H)
	add_definitions(-DHAVE_MEMORY_H)
endif(HAVE_MEMORY_H)

check_include_files("netinet/in.h" HAVE_NETINET_IN_H)
if(HAVE_NETINET_IN_H)
	add_definitions(-DHAVE_NETINET_IN_H)
endif(HAVE_NETINET_IN_H)

check_include_files("pthread.h" HAVE_PTHREAD_H)
if(HAVE_PTHREAD_H)
	add_definitions(-DHAVE_PTHREAD_H)
endif(HAVE_PTHREAD_H)

check_include_files("pwd.h" HAVE_PWD_H)
if(HAVE_PWD_H)
	add_definitions(-DHAVE_PWD_H)
endif(HAVE_PWD_H)

check_include_files("regex.h" HAVE_REGEX_H)
if(HAVE_REGEX_H)
	add_definitions(-DHAVE_REGEX_H)
endif(HAVE_REGEX_H)

check_include_files("signal.h" HAVE_SIGNAL_H)
if(HAVE_SIGNAL_H)
	add_definitions(-DHAVE_SIGNAL_H)
endif(HAVE_SIGNAL_H)

check_include_files("stddef.h" HAVE_STDDEF_H)
if(HAVE_STDDEF_H)
	add_definitions(-DHAVE_STDDEF_H)
endif(HAVE_STDDEF_H)

check_include_files("stdint.h" HAVE_STDINT_H)
if(HAVE_STDINT_H)
	add_definitions(-DHAVE_STDINT_H)
endif(HAVE_STDINT_H)

check_include_files("stdio.h" HAVE_STDIO_H)
if(HAVE_STDIO_H)
	add_definitions(-DHAVE_STDIO_H)
endif(HAVE_STDIO_H)

check_include_files("stdlib.h" HAVE_STDLIB_H)
if(HAVE_STDLIB_H)
	add_definitions(-DHAVE_STDLIB_H)
endif(HAVE_STDLIB_H)

check_include_files("arpa/inet.h" HAVE_ARPA_INET_H)
if(HAVE_ARPA_INET_H)
	add_definitions(-DHAVE_ARPA_INET_H)
endif(HAVE_ARPA_INET_H)

check_include_files("getopt.h" HAVE_GETOPT_H)
if(HAVE_GETOPT_H)
	add_definitions(-DHAVE_GETOPT_H)
endif(HAVE_GETOPT_H)
#End of Includes

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
target_link_libraries(dar ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${LZO_LIBS})