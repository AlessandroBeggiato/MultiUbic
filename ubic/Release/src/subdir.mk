################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/causRed.c \
../src/debug.c \
../src/ec.c \
../src/exm.c \
../src/glue.c \
../src/h.c \
../src/main.c \
../src/marking.c \
../src/netconv.c \
../src/nodelist.c \
../src/output.c \
../src/pe.c \
../src/readlib.c \
../src/readpep.c \
../src/stack.c \
../src/unfold.c 

OBJS += \
./src/causRed.o \
./src/debug.o \
./src/ec.o \
./src/exm.o \
./src/glue.o \
./src/h.o \
./src/main.o \
./src/marking.o \
./src/netconv.o \
./src/nodelist.o \
./src/output.o \
./src/pe.o \
./src/readlib.o \
./src/readpep.o \
./src/stack.o \
./src/unfold.o 

C_DEPS += \
./src/causRed.d \
./src/debug.d \
./src/ec.d \
./src/exm.d \
./src/glue.d \
./src/h.d \
./src/main.d \
./src/marking.d \
./src/netconv.d \
./src/nodelist.d \
./src/output.d \
./src/pe.d \
./src/readlib.d \
./src/readpep.d \
./src/stack.d \
./src/unfold.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I../include -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


