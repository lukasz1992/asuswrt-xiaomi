################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../platforms/Windows/MSVC9/PCH.cpp 

OBJS += \
./platforms/Windows/MSVC9/PCH.o 

CPP_DEPS += \
./platforms/Windows/MSVC9/PCH.d 


# Each subdirectory must supply rules for building sources it contributes
platforms/Windows/MSVC9/%.o: ../platforms/Windows/MSVC9/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


