################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/kademlia/kademlia/Entry.cpp \
../src/kademlia/kademlia/Indexed.cpp \
../src/kademlia/kademlia/Kademlia.cpp \
../src/kademlia/kademlia/Prefs.cpp \
../src/kademlia/kademlia/Search.cpp \
../src/kademlia/kademlia/SearchManager.cpp \
../src/kademlia/kademlia/UDPFirewallTester.cpp 

OBJS += \
./src/kademlia/kademlia/Entry.o \
./src/kademlia/kademlia/Indexed.o \
./src/kademlia/kademlia/Kademlia.o \
./src/kademlia/kademlia/Prefs.o \
./src/kademlia/kademlia/Search.o \
./src/kademlia/kademlia/SearchManager.o \
./src/kademlia/kademlia/UDPFirewallTester.o 

CPP_DEPS += \
./src/kademlia/kademlia/Entry.d \
./src/kademlia/kademlia/Indexed.d \
./src/kademlia/kademlia/Kademlia.d \
./src/kademlia/kademlia/Prefs.d \
./src/kademlia/kademlia/Search.d \
./src/kademlia/kademlia/SearchManager.d \
./src/kademlia/kademlia/UDPFirewallTester.d 


# Each subdirectory must supply rules for building sources it contributes
src/kademlia/kademlia/%.o: ../src/kademlia/kademlia/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


