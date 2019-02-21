################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/utils/wxCas/src/linuxmon.cpp \
../src/utils/wxCas/src/onlinesig.cpp \
../src/utils/wxCas/src/wxcas.cpp \
../src/utils/wxCas/src/wxcascte.cpp \
../src/utils/wxCas/src/wxcasframe.cpp \
../src/utils/wxCas/src/wxcaspix.cpp \
../src/utils/wxCas/src/wxcasprefs.cpp \
../src/utils/wxCas/src/wxcasprint.cpp 

OBJS += \
./src/utils/wxCas/src/linuxmon.o \
./src/utils/wxCas/src/onlinesig.o \
./src/utils/wxCas/src/wxcas.o \
./src/utils/wxCas/src/wxcascte.o \
./src/utils/wxCas/src/wxcasframe.o \
./src/utils/wxCas/src/wxcaspix.o \
./src/utils/wxCas/src/wxcasprefs.o \
./src/utils/wxCas/src/wxcasprint.o 

CPP_DEPS += \
./src/utils/wxCas/src/linuxmon.d \
./src/utils/wxCas/src/onlinesig.d \
./src/utils/wxCas/src/wxcas.d \
./src/utils/wxCas/src/wxcascte.d \
./src/utils/wxCas/src/wxcasframe.d \
./src/utils/wxCas/src/wxcaspix.d \
./src/utils/wxCas/src/wxcasprefs.d \
./src/utils/wxCas/src/wxcasprint.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils/wxCas/src/%.o: ../src/utils/wxCas/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


