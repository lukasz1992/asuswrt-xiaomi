################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/kademlia/utils/UInt128.cpp 

OBJS += \
./src/kademlia/utils/UInt128.o 

CPP_DEPS += \
./src/kademlia/utils/UInt128.d 


# Each subdirectory must supply rules for building sources it contributes
src/kademlia/utils/%.o: ../src/kademlia/utils/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


