################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../unittests/muleunit/main.cpp \
../unittests/muleunit/test.cpp \
../unittests/muleunit/testcase.cpp \
../unittests/muleunit/testregistry.cpp 

OBJS += \
./unittests/muleunit/main.o \
./unittests/muleunit/test.o \
./unittests/muleunit/testcase.o \
./unittests/muleunit/testregistry.o 

CPP_DEPS += \
./unittests/muleunit/main.d \
./unittests/muleunit/test.d \
./unittests/muleunit/testcase.d \
./unittests/muleunit/testregistry.d 


# Each subdirectory must supply rules for building sources it contributes
unittests/muleunit/%.o: ../unittests/muleunit/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


