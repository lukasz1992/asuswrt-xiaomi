################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/utils/fileview/FileView.cpp \
../src/utils/fileview/KadFiles.cpp \
../src/utils/fileview/Print.cpp \
../src/utils/fileview/eD2kFiles.cpp 

OBJS += \
./src/utils/fileview/FileView.o \
./src/utils/fileview/KadFiles.o \
./src/utils/fileview/Print.o \
./src/utils/fileview/eD2kFiles.o 

CPP_DEPS += \
./src/utils/fileview/FileView.d \
./src/utils/fileview/KadFiles.d \
./src/utils/fileview/Print.d \
./src/utils/fileview/eD2kFiles.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils/fileview/%.o: ../src/utils/fileview/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


