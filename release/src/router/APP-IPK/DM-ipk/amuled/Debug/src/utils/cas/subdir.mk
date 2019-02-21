################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/utils/cas/cas.c \
../src/utils/cas/configfile.c \
../src/utils/cas/functions.c \
../src/utils/cas/graphics.c \
../src/utils/cas/html.c \
../src/utils/cas/lines.c 

OBJS += \
./src/utils/cas/cas.o \
./src/utils/cas/configfile.o \
./src/utils/cas/functions.o \
./src/utils/cas/graphics.o \
./src/utils/cas/html.o \
./src/utils/cas/lines.o 

C_DEPS += \
./src/utils/cas/cas.d \
./src/utils/cas/configfile.d \
./src/utils/cas/functions.d \
./src/utils/cas/graphics.d \
./src/utils/cas/html.d \
./src/utils/cas/lines.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils/cas/%.o: ../src/utils/cas/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


