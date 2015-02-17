#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libprotobuf2jansson.la

#######################################
# Build information for each library

# Sources for libprotobuf2jansson
libprotobuf2jansson_la_SOURCES = libprotobuf2jansson.c varint.c meta_descriptor.json.c

# Linker options libprotobuf2janssonProgram
libprotobuf2jansson_la_LDFLAGS = $(jansson_LIBS)

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libprotobuf2jansson_la_CPPFLAGS = $(jansson_CFLAGS) -I$(top_srcdir)/include

