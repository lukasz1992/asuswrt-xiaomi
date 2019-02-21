################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/libs/common/FileFunctions.o \
../src/libs/common/Format.o \
../src/libs/common/MD5Sum.o \
../src/libs/common/MuleDebug.o \
../src/libs/common/Path.o \
../src/libs/common/StringFunctions.o \
../src/libs/common/TextFile.o \
../src/libs/common/strerror_r.o 

CPP_SRCS += \
../src/libs/common/FileFunctions.cpp \
../src/libs/common/Format.cpp \
../src/libs/common/MD5Sum.cpp \
../src/libs/common/MuleDebug.cpp \
../src/libs/common/Path.cpp \
../src/libs/common/StringFunctions.cpp \
../src/libs/common/TextFile.cpp 

C_SRCS += \
../src/libs/common/strerror_r.c 

OBJS += \
./src/libs/common/FileFunctions.o \
./src/libs/common/Format.o \
./src/libs/common/MD5Sum.o \
./src/libs/common/MuleDebug.o \
./src/libs/common/Path.o \
./src/libs/common/StringFunctions.o \
./src/libs/common/TextFile.o \
./src/libs/common/strerror_r.o 

C_DEPS += \
./src/libs/common/strerror_r.d 

CPP_DEPS += \
./src/libs/common/FileFunctions.d \
./src/libs/common/Format.d \
./src/libs/common/MD5Sum.d \
./src/libs/common/MuleDebug.d \
./src/libs/common/Path.d \
./src/libs/common/StringFunctions.d \
./src/libs/common/TextFile.d 


# Each subdirectory must supply rules for building sources it contributes
src/libs/common/%.o: ../src/libs/common/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/libs/common/%.o: ../src/libs/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


