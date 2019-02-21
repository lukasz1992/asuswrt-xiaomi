################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/utils/plasmamule/plasma-applet-plasmamule.cpp \
../src/utils/plasmamule/plasma-engine-plasmamule.cpp \
../src/utils/plasmamule/plasmamule-dbus.cpp \
../src/utils/plasmamule/plasmamule-engine-feeder.cpp \
../src/utils/plasmamule/qt-emc.cpp 

OBJS += \
./src/utils/plasmamule/plasma-applet-plasmamule.o \
./src/utils/plasmamule/plasma-engine-plasmamule.o \
./src/utils/plasmamule/plasmamule-dbus.o \
./src/utils/plasmamule/plasmamule-engine-feeder.o \
./src/utils/plasmamule/qt-emc.o 

CPP_DEPS += \
./src/utils/plasmamule/plasma-applet-plasmamule.d \
./src/utils/plasmamule/plasma-engine-plasmamule.d \
./src/utils/plasmamule/plasmamule-dbus.d \
./src/utils/plasmamule/plasmamule-engine-feeder.d \
./src/utils/plasmamule/qt-emc.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils/plasmamule/%.o: ../src/utils/plasmamule/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


