################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../unittests/tests/CTagTest.cpp \
../unittests/tests/CUInt128Test.cpp \
../unittests/tests/FileDataIOTest.cpp \
../unittests/tests/FormatTest.cpp \
../unittests/tests/NetworkFunctionsTest.cpp \
../unittests/tests/PathTest.cpp \
../unittests/tests/RangeMapTest.cpp \
../unittests/tests/StringFunctionsTest.cpp \
../unittests/tests/TextFileTest.cpp 

OBJS += \
./unittests/tests/CTagTest.o \
./unittests/tests/CUInt128Test.o \
./unittests/tests/FileDataIOTest.o \
./unittests/tests/FormatTest.o \
./unittests/tests/NetworkFunctionsTest.o \
./unittests/tests/PathTest.o \
./unittests/tests/RangeMapTest.o \
./unittests/tests/StringFunctionsTest.o \
./unittests/tests/TextFileTest.o 

CPP_DEPS += \
./unittests/tests/CTagTest.d \
./unittests/tests/CUInt128Test.d \
./unittests/tests/FileDataIOTest.d \
./unittests/tests/FormatTest.d \
./unittests/tests/NetworkFunctionsTest.d \
./unittests/tests/PathTest.d \
./unittests/tests/RangeMapTest.d \
./unittests/tests/StringFunctionsTest.d \
./unittests/tests/TextFileTest.d 


# Each subdirectory must supply rules for building sources it contributes
unittests/tests/%.o: ../unittests/tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


