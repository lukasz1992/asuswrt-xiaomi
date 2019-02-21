################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/kademlia/net/KademliaUDPListener.cpp \
../src/kademlia/net/PacketTracking.cpp 

OBJS += \
./src/kademlia/net/KademliaUDPListener.o \
./src/kademlia/net/PacketTracking.o 

CPP_DEPS += \
./src/kademlia/net/KademliaUDPListener.d \
./src/kademlia/net/PacketTracking.d 


# Each subdirectory must supply rules for building sources it contributes
src/kademlia/net/%.o: ../src/kademlia/net/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


