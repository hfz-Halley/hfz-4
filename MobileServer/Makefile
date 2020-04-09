#
# Makefile
#
# Makefile for Firefcin Ecosystem Network Management Daemon 
#

include $(PROJECT_BASE)/build/rules/global
CXXFLAGS=-g -std=c++11 -Wall -pedantic

PATH_NAME=$(shell pwd)
INCDIR += -I $(PATH_NAME)/include/
INCDIR += -I $(PATH_NAME)/include/Dhcp
INCDIR += -I $(PATH_NAME)/include/Gobi4G
INCDIR += -I $(PATH_NAME)/include/Serial/
INCDIR += -I $(PATH_NAME)/src/
SRCS_PATH_CPP = $(wildcard ./src/*.cpp )
SRCS_PATH_C= $(wildcard ./src/*.c )

SRCS_CPP= $(subst ./src/, ,$(SRCS_PATH_CPP)) 
SRCS_C= $(subst ./src/, ,$(SRCS_PATH_C)) 

objects  += $(patsubst %.cpp, %, $(SRCS_CPP))
objects  += $(patsubst %.c, %, $(SRCS_C))

$(info $(nn))
$(info $(objects))

target         = MobileServer
#target         = caller
target_version = 1
target_libs    = PocoNet PocoOSP  PocoZip PocoUtil PocoJSON PocoXML PocoFoundation  IoTServerBase dbus-1 sigc-2.0 dbus-cxx 

include $(PROJECT_BASE)/build/rules/exec
ifdef POCO_UNBUNDLED
        SYSLIBS += -lz -lpcre -lexpat
endif

