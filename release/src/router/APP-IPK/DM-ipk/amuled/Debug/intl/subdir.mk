################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../intl/bindtextdom.c \
../intl/dcgettext.c \
../intl/dcigettext.c \
../intl/dcngettext.c \
../intl/dgettext.c \
../intl/dngettext.c \
../intl/explodename.c \
../intl/finddomain.c \
../intl/gettext.c \
../intl/intl-compat.c \
../intl/l10nflist.c \
../intl/loadmsgcat.c \
../intl/localcharset.c \
../intl/localealias.c \
../intl/localename.c \
../intl/ngettext.c \
../intl/os2compat.c \
../intl/osdep.c \
../intl/plural-exp.c \
../intl/plural.c \
../intl/textdomain.c 

OBJS += \
./intl/bindtextdom.o \
./intl/dcgettext.o \
./intl/dcigettext.o \
./intl/dcngettext.o \
./intl/dgettext.o \
./intl/dngettext.o \
./intl/explodename.o \
./intl/finddomain.o \
./intl/gettext.o \
./intl/intl-compat.o \
./intl/l10nflist.o \
./intl/loadmsgcat.o \
./intl/localcharset.o \
./intl/localealias.o \
./intl/localename.o \
./intl/ngettext.o \
./intl/os2compat.o \
./intl/osdep.o \
./intl/plural-exp.o \
./intl/plural.o \
./intl/textdomain.o 

C_DEPS += \
./intl/bindtextdom.d \
./intl/dcgettext.d \
./intl/dcigettext.d \
./intl/dcngettext.d \
./intl/dgettext.d \
./intl/dngettext.d \
./intl/explodename.d \
./intl/finddomain.d \
./intl/gettext.d \
./intl/intl-compat.d \
./intl/l10nflist.d \
./intl/loadmsgcat.d \
./intl/localcharset.d \
./intl/localealias.d \
./intl/localename.d \
./intl/ngettext.d \
./intl/os2compat.d \
./intl/osdep.d \
./intl/plural-exp.d \
./intl/plural.d \
./intl/textdomain.d 


# Each subdirectory must supply rules for building sources it contributes
intl/%.o: ../intl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


