################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include unittests/tests/subdir.mk
-include unittests/muleunit/subdir.mk
-include src/webserver/src/subdir.mk
-include src/utils/wxCas/src/subdir.mk
-include src/utils/plasmamule/subdir.mk
-include src/utils/subdir.mk
-include src/utils/fileview/subdir.mk
-include src/utils/cas/subdir.mk
-include src/utils/aLinkCreator/src/subdir.mk
-include src/subdir.mk
-include src/libs/ec/cpp/subdir.mk
-include src/libs/common/subdir.mk
-include src/kademlia/utils/subdir.mk
-include src/kademlia/routing/subdir.mk
-include src/kademlia/net/subdir.mk
-include src/kademlia/kademlia/subdir.mk
-include src/extern/wxWidgets/subdir.mk
-include platforms/Windows/MSVC9/subdir.mk
-include platforms/Windows/MSVC10/subdir.mk
-include intl/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: amuled

# Tool invocations
amuled: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++  -o"amuled" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) amuled
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
