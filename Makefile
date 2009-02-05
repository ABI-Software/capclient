
################################################################
#Defaults (All make variables can be overridden my specfiying on the make command line).

DEBUG = true
CMGUI_PATH = $(CMISS_ROOT)/cmgui
BUILD_VERSION = cmgui-wx-debug

###############################################################
OBJECT_PATH = .
include $(CMGUI_PATH)/source/common.Makefile_lib
CMGUI_LIB_PATH = $(CMGUI_PATH)/lib/$(LIB_ARCH_DIR)/$(BUILD_VERSION)
CMGUI_LIBS = cmgui
ALL_FLAGS += $(OPTIMISATION_FLAGS) $(COMPILE_FLAGS) -I$(CMGUI_PATH)/source

   ITK_DEFINES = -DUSE_ITK
   ITK_SRCDIR = $(CMISS_ROOT)/itk/src
   ITK_BINDIR = $(CMISS_ROOT)/itk/lib/$(LIB_ARCH_DIR)
   ITK_INC = -I$(ITK_BINDIR) -I$(ITK_SRCDIR)/Code/Algorithms -I$(ITK_SRCDIR)/Code/BasicFilters -I$(ITK_SRCDIR)/Code/Common -I$(ITK_SRCDIR)/Code/Numerics/Statistics -I$(ITK_SRCDIR)/Utilities/vxl/vcl -I$(ITK_SRCDIR)/Utilities/vxl/core -I$(ITK_BINDIR)/Utilities/vxl/vcl -I$(ITK_BINDIR)/Utilities/vxl/core/
   ITK_LIBPATH_PREFIX = -L
   ITK_LIB_PREFIX = -l 
   ITK_LIB_SUFFIX =
   ITK_BIN_CONFIG_DIR = 
   ifeq ($(COMPILER),msvc)
     ITK_LIBPATH_PREFIX = /libpath:
     ITK_LIB_PREFIX = 
     ITK_LIB_SUFFIX = .lib
     ITK_BIN_CONFIG_DIR = /Release
   endif
   ITK_LIB = $(ITK_LIBPATH_PREFIX)$(ITK_BINDIR)/bin$(ITK_BIN_CONFIG_DIR) $(ITK_LIB_PREFIX)ITKAlgorithms$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)ITKStatistics$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)ITKBasicFilters$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)ITKCommon$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itkvnl$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itkvnl_algo$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itkvnl$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itknetlib$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itksys$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)ITKDICOMParser$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itkzlib$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itkzlib$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itktiff$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itkjpeg12$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)itkjpeg16$(ITK_LIB_SUFFIX) $(ITK_LIB_PREFIX)ITKNrrdIO$(ITK_LIB_SUFFIX) 


   IMAGEMAGICK_PATH = $(CMISS_ROOT)/image_libraries
   IMAGEMAGICK_LIB = $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libMagickCore.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libtiff.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libpng.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libjpeg.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libbz2.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libz.a
   ifeq ($(USE_LIBGDCM), true)
         IMAGEMAGICK_LIB += -L$(IMAGEMAGICK_PATH)/$(LIB_ARCH_DIR)/lib -lgdcmCWRAPPER -lgdcmMSFF -lgdcmDSED -lgdcmzlib -lgdcmmd5 -lgdcmDICT -lgdcmCommon -lgdcmIOD -lgdcmopenjpeg -lgdcmjpeg8 -lgdcmjpeg16 -lgdcmjpeg12 -lgdcmexpat
   endif
   ifneq ($(wildcard $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libltdl.a),)
      #When this first appeared it seemed to be configured for most versions, now it seems to be configured for very few.  Assume we need it only if it is found.
      IMAGEMAGICK_LIB += $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libltdl.a
   endif
ifeq ($(USE_XML2),true)
   IMAGEMAGICK_LIB += $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libxml2.a
endif # USE_XML2

WX_DIR = /Users/jchu014/cmiss/wxWidgets-2.8.8/bin/
DEBUG = true
   ifneq ($(DEBUG),true)
     WX_DEBUG_FLAG = no
   else # $(DEBUG) != true
     WX_DEBUG_FLAG = yes
   endif # $(DEBUG) != true
   #Default list does not include gl, so we list them here.
   #Using xrc means that we require most things (and static wx libs don't automatically pull
   #in the other dependent wx-libs)
   #Use linkdeps so that we don't get all the other system libraries.
   WX_STATIC_FLAG=yes
   WX_CONFIG_LIBS := $(shell $(WX_DIR)wx-config --linkdeps --unicode=no --debug=$(WX_DEBUG_FLAG) --static=$(WX_STATIC_FLAG) xrc,gl,xml,adv,html,core,base)
   ifneq ($(WX_CONFIG_LIBS),)
     #Presume it succeeded, use this config
     USER_INTERFACE_LIB += $(WX_CONFIG_LIBS)
   else
     $(warning Static wx build not detected, trying a dynamic version)
     #Use the full libs
     WX_STATIC_FLAG = no
     WX_CONFIG_LIBS := $(shell $(WX_DIR)wx-config --libs --unicode=no --debug=$(WX_DEBUG_FLAG) --static=$(WX_STATIC_FLAG) xrc,gl,xml,adv,html,core,base)
     ifneq ($(WX_CONFIG_LIBS),)
       #Presume this worked
       USER_INTERFACE_LIB += $(WX_CONFIG_LIBS)
     else
       $(error cmgui build error wx config not matched for $(WX_DIR)wx-config --libs --unicode=no --debug=$(WX_DEBUG_FLAG) xrc,gl,xml,adv,html,core,base)
     endif
   endif
   USER_INTERFACE_INC += $(shell $(WX_DIR)wx-config --cxxflags --unicode=no --debug=$(WX_DEBUG_FLAG) --static=$(WX_STATIC_FLAG))

OPERATING_SYSTEM = darwin

   ifeq ($(OPERATING_SYSTEM),linux)
      ifneq ($(STATIC_LINK),true)
         USER_INTERFACE_LIB += $(shell pkg-config gtk+-2.0 gthread-2.0 --libs) -lXmu  -lXinerama
         ifeq ($(INSTRUCTION),x86_64)
	    USER_INTERFACE_LIB +=
	 else
	    USER_INTERFACE_LIB += -lXxf86vm
	 endif 
      else # $(STATIC_LINK) != true
         USER_INTERFACE_LIB += -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangox-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0 -lwxexpat-2.6-i686-linux
      endif # $(STATIC_LINK) != true
   else # $(OPERATING_SYSTEM) == linux
      ifeq ($(OPERATING_SYSTEM),win32)
         USER_INTERFACE_LIB += -lcomctl32 -lctl3d32 -L$(shell $(WX_DIR)wx-config --prefix)/lib -lwxexpat-2.8-i386-mingw32msvc
      else # $(OPERATING_SYSTEM) == win32
         ifeq ($(OPERATING_SYSTEM),darwin)
            USER_INTERFACE_LIB += -L$(shell $(WX_DIR)wx-config --prefix)/lib -framework QuickTime -framework IOKit -framework Carbon -framework Cocoa -framework System -framework WebKit -framework OpenGL -lwxexpatd-2.8
         else # $(OPERATING_SYSTEM) == darwin
            USER_INTERFACE_LIB += $(shell $(WX_DIR)wx-config --libs xrc,gl,xml,adv,html,core,base)
         endif # $(OPERATING_SYSTEM) == darwin
      endif # $(OPERATING_SYSTEM) == win32
   endif # $(OPERATING_SYSTEM) == linux

   INTERPRETER_PATH = $(CMISS_ROOT)/perl_interpreter
   INTERPRETER_INC = -I$(INTERPRETER_PATH)/source/
   INTERPRETER_DEFINES = -DPERL_INTERPRETER
   INTERPRETER_SRCS =
   INTERPRETER_LIB = \
	   $(INTERPRETER_PATH)/lib/$(LIB_ARCH_DIR)/libperlinterpreter.a
	   
	   
   XML2_PATH = $(CMISS_ROOT)/image_libraries/
   XML2_DEFINES = -DHAVE_XML2
   XML2_INC = -I$(XML2_PATH)/include/$(LIB_ARCH_DIR)/libxml2/
   XML2_LIB = $(XML2_PATH)/lib/$(LIB_ARCH_DIR)/libxml2.a

LIBS = -L$(CMGUI_LIB_PATH) $(foreach lib,$(CMGUI_LIBS),-l$(lib)) -framework OpenGL -framework Carbon -framework AGL -L/System/Library/Frameworks/OpenGL.framework/Libraries/ \
	$(ITK_LIB) $(IMAGEMAGICK_LIB) $(USER_INTERFACE_LIB) $(INTERPRETER_LIB) $(XML2_LIB) -liconv
	
CMGUI_LIB_FILES = $(foreach lib,$(CMGUI_LIBS),$(CMGUI_LIB_PATH)/lib$(lib).a)
LD_LIBRARY_PATH = $(CMGUI_LIB_PATH)
export LD_LIBRARY_PATH

ALL_FLAGS += $(USER_INTERFACE_INC) $(INTERPRETER_INC) -DWX_USER_INTERFACE -DPERL_INTERPRETER -DUSE_ITK

###############################################################################
# File management.  This is where the source, header, and object files are
# defined

#
# source files
srcfiles 	:= 		ViewerFrame.cpp \
		main.cpp
#		SceneViewer.cpp \
#		TextEditorApp.cpp \

# object files
objects		:= $(patsubst %.cpp, %.$(OBJ_SUFFIX), $(srcfiles))
###############################################################################



.PHONY: clean clobber distclean

###############################################################################
# Target:
#
target 	   := api_test

all:: $(target)

#
# Dependencies
#
DEPEND_FILES = $(objects:%.$(OBJ_SUFFIX)=%.d)

# include the dependency list
include $(DEPEND_FILES)
$(target): $(DEPEND_FILES)

$(target): $(CMGUI_LIB_FILES)

# Production rules:  how to make the target - depends on library configuration
$(target): $(objects)
	@echo "Linking "$@"..."
	$(call BuildNormalTarget,$(target),.,$(objects), $(LIBS))
#	@$(CXX) $(CXXFLAGS) $(objects) -o $@ $(LIBS) $(LDFLAGS) $(EXTERNAL_FLAGS)

echo:
	@echo "obj= $(objects)"

# Useful rules.
clean:
	@rm -f $(objects) $(target) 

clobber:
	@$(MAKE) clean
	@rm -f $(target)

distclean:
	@$(MAKE) clobber
	@rm -f *.o *.g.o *.pg.o

run: $(target)
	@echo "***************************************************************"
	@echo "* Running Example " $(target)
	@echo "***************************************************************"
	@echo " "
	@$(run_environment) ./$(target) $(run_flags)
	@echo " "
	@echo "***************************************************************"
	@echo "* Done Running Example " $(target)
	@echo "***************************************************************"



###############################################################################
