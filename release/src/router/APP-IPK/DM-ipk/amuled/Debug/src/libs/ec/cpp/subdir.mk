################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/libs/ec/cpp/ECMuleSocket.o \
../src/libs/ec/cpp/ECPacket.o \
../src/libs/ec/cpp/ECSocket.o \
../src/libs/ec/cpp/ECSpecialTags.o \
../src/libs/ec/cpp/ECTag.o \
../src/libs/ec/cpp/RemoteConnect.o 

CPP_SRCS += \
../src/libs/ec/cpp/ECMuleSocket.cpp \
../src/libs/ec/cpp/ECPacket.cpp \
../src/libs/ec/cpp/ECSocket.cpp \
../src/libs/ec/cpp/ECSpecialTags.cpp \
../src/libs/ec/cpp/ECTag.cpp \
../src/libs/ec/cpp/RemoteConnect.cpp 

OBJS += \
./src/libs/ec/cpp/ECMuleSocket.o \
./src/libs/ec/cpp/ECPacket.o \
./src/libs/ec/cpp/ECSocket.o \
./src/libs/ec/cpp/ECSpecialTags.o \
./src/libs/ec/cpp/ECTag.o \
./src/libs/ec/cpp/RemoteConnect.o 

CPP_DEPS += \
./src/libs/ec/cpp/ECMuleSocket.d \
./src/libs/ec/cpp/ECPacket.d \
./src/libs/ec/cpp/ECSocket.d \
./src/libs/ec/cpp/ECSpecialTags.d \
./src/libs/ec/cpp/ECTag.d \
./src/libs/ec/cpp/RemoteConnect.d 


# Each subdirectory must supply rules for building sources it contributes
src/libs/ec/cpp/%.o: ../src/libs/ec/cpp/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


