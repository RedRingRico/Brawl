###############################################################################
#                                                      Brawl Makefile [Linux] #
###############################################################################
.SILENT:
ifndef TOPDIR
export TOPDIR			:= $(shell cd ../../ && pwd)
export TOPSRC			= $(TOPDIR)/Source
export SHADERSRC		= $(TOPSRC)/Shaders
export TARGETDIR		= $(TOPDIR)/Bin/$(PLATFORM)/$(ARCH)/$(BITSIZE)
export SHADERSDIR		= $(TOPDIR)/Bin/$(PLATFORM)/$(ARCH)/$(BITSIZE)/Data/Shaders
export OBJSDIR			= $(TOPDIR)/Obj/$(PLATFORM)/$(ARCH)/$(BITSIZE)/GCC/$(GCCVER)/$(BUILD)
export PLATFORM			:= $(notdir $(CURDIR))
export BUILD_PLATFORM	:= $(shell echo $(PLATFORM) | tr [a-z] [A-Z])
endif
SOURCEDIR	= Source ../Common/Source

PROJECT := Brawl
TARGET := $(PROJECT)
OUTFILE	=	$(TARGETDIR)/$(TARGET)

BUILD_ARCH		:= X86
ARCH			:= x86 
BITSIZE			:= 32

GCCVER	= $(shell $(CXX) -dumpversion)

BUILD_TYPE := debug

32BIT ?= False
64BIT ?= False

##### Get the machine type to determine which set of libraries to use #########
UNAME			= $(shell uname)
UNAME_MACHINE	= $(shell uname -m)

ifeq ($(UNAME), Linux)
##### Check for forced 32-bit or 64-bit builds ################################
ifneq ($(64BIT), False)
	ARCH		=	x86
	BITSIZE		=	64
else
ifneq ($(32BIT), False)
	ARCH		= x86
	BITSIZE		= 32
else
##### Nothing forced.  Use what the machine reports ###########################
ifeq ($(UNAME_MACHINE), x86_64)
	ARCH		=	x86
	BITSIZE		=	64
endif
ifeq ($(UNAME_MACHINE), i686)
	ARCH		= x86
	BITSIZE		= 32
endif
endif
endif
endif

CXX = g++

CXXFLAGS_EXT	=	-std=c++14 -fno-rtti

CXXFLAGS	=	-c -m64 -DBRAWL_BUILD_$(BUILD_DEF) \
				-DBRAWL_PLATFORM_$(BUILD_PLATFORM) \
				-DBRAWL_PLATFORM_$(BUILD_PLATFORM)_$(BUILD_ARCH) \
				-DBRAWL_PLATFORM_$(BUILD_PLATFORM)_$(BUILD_ARCH)_$(BITSIZE) \
				-DBRAWL_BITSIZE_$(BITSIZE) -DBRAWL_ARCH_$(BUILD_ARCH) \
				-DVK_USE_PLATFORM_XCB_KHR -DVK_NO_PROTOTYPES \
				-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE \
				-I$(VULKAN_SDK)/include \
				-I$(TOPSRC)/$(PLATFORM)/Headers -I$(TOPSRC)/Common/Headers \
				-I$(TOPSRC)/Libs
SYSIPATH	=
INCPATH		= 
SYSLPATH	=
LIBPATH		= -L$(VULKAN_SDK)/lib
SYSLIBS		= -lrt -pthread -lvulkan -lxcb -lxcb-randr -lxcb-xfixes -ldl
LINKFLAGS	=

##### Debug #######
debug:		BUILD		= Debug
debug:		BUILD_TYPE	= debug
debug:		BUILD_DEF	= DEBUG
debug:		TARGET := $(TARGET)D
debug:		CXXFLAGS += -O0 -ggdb -g3 -Wall -D_DEBUG $(CXXFLAGS_EXT)
debug:		LIBS =
debug:		$(TARGET)

##### Release #####
release:	BUILD		= Release
release:	BUILD_TYPE	= release
release:	BUILD_DEF	= RELEASE
release:	TARGET := $(TARGET)
release:	CXXFLAGS += -O3 -s $(CXXFLAGS_EXT)
release:	LIBS =
release:	LINKFLAGS += -Wl,-S
release:	$(TARGET)

##### Profile #####
profile:	BUILD		= Profile
profile:	BUILD_TYPE	= profile
profile:	BUILD_DEF	= PROFILE
profile:	TARGET := $(TARGET)P
profile:	CXXFLAGS += -O3 -g -ggdb -Wall -D_DEBUG $(CXXFLAGS_EXT)
profile:	LIBS =
profile:	$(TARGET)

##### Build the object files while not in the Obj directory ###################
ifneq ($(OBJSDIR), $(CURDIR))

VERSIONINFO:
	printf "%s" "------------------------- Generating Version Information ---"
	printf "%s\n" "-------------------"
	mkdir -p $(TOPSRC)/Common/Headers
	sh $(TOPSRC)/Common/GitVersion.sh $(TOPSRC)/Common/Headers/GitVersion.hpp $(PROJECT)
	printf "%s" "------------------------------------------------------------"
	printf "%s\n" "-------------------"

CLOC:
	@sh ./Cloc.sh ./Headers/CLOC.hpp ${PROJECT}

TARGETDIR:
	@mkdir -p $(TARGETDIR)

SHADERSDIR:
	@mkdir -p $(TARGETDIR)

OBJSDIR:
	@mkdir -p $(OBJSDIR)

CXXFILES	:= $(foreach dir,$(SOURCEDIR),$(notdir $(wildcard $(dir)/*.cpp)))
VERTFILES	:= $(foreach dir,$(SHADERSRC),$(wildcard $(dir)/*.vert))
FRAGFILES	:= $(foreach dir,$(SHADERSRC),$(wildcard $(dir)/*.frag))
export VPATH	:= $(foreach dir,$(SOURCEDIR),$(CURDIR)/$(dir)) $(SHADERDIR)
export OBJS		:= $(CXXFILES:.cpp=.o)
#export SPVS		:= $(TARGETDIR)/Shaders/$(notdir $(VERTFILES:.vert=.vert.spv)) #$(FRAGFILES:.frag=_frag.spv)
export SPVS		:= $(VERTFILES:.vert=.vert.spv) $(FRAGFILES:.frag=.frag.spv)

$(TARGET): OBJSDIR TARGETDIR SHADERSDIR CLOC VERSIONINFO $(SPVS)
	/usr/bin/time -f "%E" -o $(OBJSDIR)/compiletime $(MAKE) \
	--no-print-directory -C $(OBJSDIR) -f $(TOPSRC)/$(PLATFORM)/Makefile \
	$(BUILD_TYPE)
	printf "\n%s" "----------------------------------------------------------"
	printf "%s\n" "---------------------"
	printf "Compile time: "
	cat $(OBJSDIR)/compiletime
	printf "%s" "------------------------------------------------------------"
	printf "%s\n" "-------------------"

%.frag.spv: %.frag
	printf "Compiling shader: $< ... \n"
	glslc $< -o $(SHADERSDIR)/$(notdir $@)

%.vert.spv: %.vert
	#printf "Found vert: $(notdir $<)\n"
	#printf "$*\n"
	#printf "Going to $(SHADERSDIR)/$(notdir $@)\n"
	#printf "OG: $<\n"
	printf "Compiling shader: $< ... \n"
	glslc $< -o $(SHADERSDIR)/$(notdir $@)
	#printf "Finished with return code: $$?\n"

else

##### In the intermediate build directory #####################################

$(TARGET): $(OBJS)
	printf "\n\n%s" "------------------------- MESSAGES FROM THE COMPILER ---"
	printf "%s\n" "-----------------------"
	cat *.cmsgs
	printf "%s" "------------------------------------------------------------"
	printf "%s\n\n" "-------------------"
	printf "Creating: $(OUTFILE)... "
	$(CXX) -ggdb -g3 -o $(OUTFILE) $(LINKFLAGS) $(OBJS) $(SYSLPATH) \
	$(LIBPATH) $(LIBS) $(SYSLIBS) 2> lmsgs;\
	RETVAL=$$?;\
	if env | grep -q ^TERM=; then\
		COLOURSUPPORT=`tput colors`;\
		if [ $$COLOURSUPPORT -gt -1 ]; then\
			cat lmsgs | sed ''/error/s//`printf "\033[31merror\033[0m"`/'' > lmsgs;\
			cat lmsgs | sed ''/warning/s//`printf "\034[31mwarning\033[0m"`/'' > lmsgs;\
		fi;\
	fi;\
	if [[ $$RETVAL == 0 ]]; then\
		printf "[OK]\n";\
	else\
		printf "\n\n%s" "------------------------------------ ERROR --------";\
		printf "%s\n" "----------------------------";\
		cat lmsgs;\
		printf "%s" "-------------------------------------------------------";\
		printf "%s\n" "------------------------";\
		exit 1;\
	fi

%.o: %.cpp
	printf "Compiling: $<... "
	$(CXX) -MMD -MP -MF $*.d $(CXXFLAGS) $(SYSIPATH) $(INCPATH) \
	$< -o $@ 2> $*.cmsgs;\
	RETVAL=$$?;\
	if env | grep -q ^TERM=; then\
		COLOURSUPPORT=`tput colors`;\
		if [ $$COLOURSUPPORT -gt -1 ]; then\
			cat $*.cmsgs | sed ''/error/s//`printf "\033[31merror\033[0m"`/'' > $*.cmsgs;\
			cat $*.cmsgs | sed ''/warning/s//`printf "\033[34mwarning\033[0m"`/'' > $*.cmsgs;\
		fi;\
	fi;\
	if [[ $$RETVAL == 0 ]]; then\
		printf "[OK]\n";\
	else\
		printf "\n\n%s" "--------------------------------- ERROR -----------";\
		printf "%s\n" "----------------------------";\
		cat $*.cmsgs;\
		printf "%s" "-------------------------------------------------------";\
		printf "%s\n\n" "------------------------";\
		exit 1;\
	fi

-include $(OBJSDIR)/*.d

endif

.PHONY: clean completeclean
clean:
	printf "Removing files from: $(TARGETDIR)... "
	rm -f $(TARGETDIR)/*
	printf "[OK]\n"
	printf "Removing files from: $(OBJSDIR)... "
	rm -rf $(OBJSDIR)/*
	printf "[OK]\n"

completeclean:
	printf "Removing files from: $(TOPDIR)/Bin... "
	rm -rf $(TOPDIR)/Bin
	printf "[OK]\n"
	printf "Removing files from: $(TOPDIR)/Obj... "
	rm -rf $(TOPDIR)/Obj
	printf "[OK]\n"

