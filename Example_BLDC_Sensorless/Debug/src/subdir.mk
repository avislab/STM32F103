################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bldc.c \
../src/main.c \
../src/syscalls.c \
../src/system_stm32f10x.c \
../src/systickdelay.c 

OBJS += \
./src/bldc.o \
./src/main.o \
./src/syscalls.o \
./src/system_stm32f10x.o \
./src/systickdelay.o 

C_DEPS += \
./src/bldc.d \
./src/main.d \
./src/syscalls.d \
./src/system_stm32f10x.d \
./src/systickdelay.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -I"/home/andre/workspace/Example_BLDC_Sensorless/StdPeriph_Driver/inc" -I"/home/andre/workspace/Example_BLDC_Sensorless/inc" -I"/home/andre/workspace/Example_BLDC_Sensorless/CMSIS/device" -I"/home/andre/workspace/Example_BLDC_Sensorless/CMSIS/core" -O0 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


