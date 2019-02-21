################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/webserver/src/amuleweb-ExternalConnector.o \
../src/webserver/src/amuleweb-LoggerConsole.o \
../src/webserver/src/amuleweb-NetworkFunctions.o \
../src/webserver/src/amuleweb-OtherFunctions.o \
../src/webserver/src/amuleweb-RLE.o \
../src/webserver/src/amuleweb-UPnPBase.o \
../src/webserver/src/amuleweb-WebInterface.o \
../src/webserver/src/amuleweb-WebServer.o \
../src/webserver/src/amuleweb-WebSocket.o \
../src/webserver/src/amuleweb-php_amule_lib.o \
../src/webserver/src/amuleweb-php_core_lib.o \
../src/webserver/src/amuleweb-php_lexer.o \
../src/webserver/src/amuleweb-php_parser.o \
../src/webserver/src/amuleweb-php_syntree.o 

CPP_SRCS += \
../src/webserver/src/WebInterface.cpp \
../src/webserver/src/WebServer.cpp \
../src/webserver/src/WebSocket.cpp \
../src/webserver/src/php_amule_lib.cpp \
../src/webserver/src/php_amule_lib_standalone.cpp \
../src/webserver/src/php_core_lib.cpp \
../src/webserver/src/php_syntree.cpp 

C_SRCS += \
../src/webserver/src/php_lexer.c \
../src/webserver/src/php_parser.c 

OBJS += \
./src/webserver/src/WebInterface.o \
./src/webserver/src/WebServer.o \
./src/webserver/src/WebSocket.o \
./src/webserver/src/php_amule_lib.o \
./src/webserver/src/php_amule_lib_standalone.o \
./src/webserver/src/php_core_lib.o \
./src/webserver/src/php_lexer.o \
./src/webserver/src/php_parser.o \
./src/webserver/src/php_syntree.o 

C_DEPS += \
./src/webserver/src/php_lexer.d \
./src/webserver/src/php_parser.d 

CPP_DEPS += \
./src/webserver/src/WebInterface.d \
./src/webserver/src/WebServer.d \
./src/webserver/src/WebSocket.d \
./src/webserver/src/php_amule_lib.d \
./src/webserver/src/php_amule_lib_standalone.d \
./src/webserver/src/php_core_lib.d \
./src/webserver/src/php_syntree.d 


# Each subdirectory must supply rules for building sources it contributes
src/webserver/src/%.o: ../src/webserver/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/webserver/src/%.o: ../src/webserver/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


