################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/utils/aLinkCreator/src/alcc-alcc.o \
../src/utils/aLinkCreator/src/alcc-ed2khash.o \
../src/utils/aLinkCreator/src/alcc-md4.o 

CPP_SRCS += \
../src/utils/aLinkCreator/src/alc.cpp \
../src/utils/aLinkCreator/src/alcc.cpp \
../src/utils/aLinkCreator/src/alcframe.cpp \
../src/utils/aLinkCreator/src/alcpix.cpp \
../src/utils/aLinkCreator/src/ed2khash.cpp \
../src/utils/aLinkCreator/src/md4.cpp 

OBJS += \
./src/utils/aLinkCreator/src/alc.o \
./src/utils/aLinkCreator/src/alcc.o \
./src/utils/aLinkCreator/src/alcframe.o \
./src/utils/aLinkCreator/src/alcpix.o \
./src/utils/aLinkCreator/src/ed2khash.o \
./src/utils/aLinkCreator/src/md4.o 

CPP_DEPS += \
./src/utils/aLinkCreator/src/alc.d \
./src/utils/aLinkCreator/src/alcc.d \
./src/utils/aLinkCreator/src/alcframe.d \
./src/utils/aLinkCreator/src/alcpix.d \
./src/utils/aLinkCreator/src/ed2khash.d \
./src/utils/aLinkCreator/src/md4.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils/aLinkCreator/src/%.o: ../src/utils/aLinkCreator/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


