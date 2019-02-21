################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/kademlia/routing/Contact.cpp \
../src/kademlia/routing/RoutingBin.cpp \
../src/kademlia/routing/RoutingZone.cpp 

OBJS += \
./src/kademlia/routing/Contact.o \
./src/kademlia/routing/RoutingBin.o \
./src/kademlia/routing/RoutingZone.o 

CPP_DEPS += \
./src/kademlia/routing/Contact.d \
./src/kademlia/routing/RoutingBin.d \
./src/kademlia/routing/RoutingZone.d 


# Each subdirectory must supply rules for building sources it contributes
src/kademlia/routing/%.o: ../src/kademlia/routing/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


