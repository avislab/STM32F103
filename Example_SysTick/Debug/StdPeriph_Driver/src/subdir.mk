################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../StdPeriph_Driver/src/misc.c \
../StdPeriph_Driver/src/stm32f10x_adc.c \
../StdPeriph_Driver/src/stm32f10x_bkp.c \
../StdPeriph_Driver/src/stm32f10x_can.c \
../StdPeriph_Driver/src/stm32f10x_cec.c \
../StdPeriph_Driver/src/stm32f10x_crc.c \
../StdPeriph_Driver/src/stm32f10x_dac.c \
../StdPeriph_Driver/src/stm32f10x_dbgmcu.c \
../StdPeriph_Driver/src/stm32f10x_dma.c \
../StdPeriph_Driver/src/stm32f10x_exti.c \
../StdPeriph_Driver/src/stm32f10x_flash.c \
../StdPeriph_Driver/src/stm32f10x_fsmc.c \
../StdPeriph_Driver/src/stm32f10x_gpio.c \
../StdPeriph_Driver/src/stm32f10x_i2c.c \
../StdPeriph_Driver/src/stm32f10x_iwdg.c \
../StdPeriph_Driver/src/stm32f10x_pwr.c \
../StdPeriph_Driver/src/stm32f10x_rcc.c \
../StdPeriph_Driver/src/stm32f10x_rtc.c \
../StdPeriph_Driver/src/stm32f10x_sdio.c \
../StdPeriph_Driver/src/stm32f10x_spi.c \
../StdPeriph_Driver/src/stm32f10x_tim.c \
../StdPeriph_Driver/src/stm32f10x_usart.c \
../StdPeriph_Driver/src/stm32f10x_wwdg.c 

OBJS += \
./StdPeriph_Driver/src/misc.o \
./StdPeriph_Driver/src/stm32f10x_adc.o \
./StdPeriph_Driver/src/stm32f10x_bkp.o \
./StdPeriph_Driver/src/stm32f10x_can.o \
./StdPeriph_Driver/src/stm32f10x_cec.o \
./StdPeriph_Driver/src/stm32f10x_crc.o \
./StdPeriph_Driver/src/stm32f10x_dac.o \
./StdPeriph_Driver/src/stm32f10x_dbgmcu.o \
./StdPeriph_Driver/src/stm32f10x_dma.o \
./StdPeriph_Driver/src/stm32f10x_exti.o \
./StdPeriph_Driver/src/stm32f10x_flash.o \
./StdPeriph_Driver/src/stm32f10x_fsmc.o \
./StdPeriph_Driver/src/stm32f10x_gpio.o \
./StdPeriph_Driver/src/stm32f10x_i2c.o \
./StdPeriph_Driver/src/stm32f10x_iwdg.o \
./StdPeriph_Driver/src/stm32f10x_pwr.o \
./StdPeriph_Driver/src/stm32f10x_rcc.o \
./StdPeriph_Driver/src/stm32f10x_rtc.o \
./StdPeriph_Driver/src/stm32f10x_sdio.o \
./StdPeriph_Driver/src/stm32f10x_spi.o \
./StdPeriph_Driver/src/stm32f10x_tim.o \
./StdPeriph_Driver/src/stm32f10x_usart.o \
./StdPeriph_Driver/src/stm32f10x_wwdg.o 

C_DEPS += \
./StdPeriph_Driver/src/misc.d \
./StdPeriph_Driver/src/stm32f10x_adc.d \
./StdPeriph_Driver/src/stm32f10x_bkp.d \
./StdPeriph_Driver/src/stm32f10x_can.d \
./StdPeriph_Driver/src/stm32f10x_cec.d \
./StdPeriph_Driver/src/stm32f10x_crc.d \
./StdPeriph_Driver/src/stm32f10x_dac.d \
./StdPeriph_Driver/src/stm32f10x_dbgmcu.d \
./StdPeriph_Driver/src/stm32f10x_dma.d \
./StdPeriph_Driver/src/stm32f10x_exti.d \
./StdPeriph_Driver/src/stm32f10x_flash.d \
./StdPeriph_Driver/src/stm32f10x_fsmc.d \
./StdPeriph_Driver/src/stm32f10x_gpio.d \
./StdPeriph_Driver/src/stm32f10x_i2c.d \
./StdPeriph_Driver/src/stm32f10x_iwdg.d \
./StdPeriph_Driver/src/stm32f10x_pwr.d \
./StdPeriph_Driver/src/stm32f10x_rcc.d \
./StdPeriph_Driver/src/stm32f10x_rtc.d \
./StdPeriph_Driver/src/stm32f10x_sdio.d \
./StdPeriph_Driver/src/stm32f10x_spi.d \
./StdPeriph_Driver/src/stm32f10x_tim.d \
./StdPeriph_Driver/src/stm32f10x_usart.d \
./StdPeriph_Driver/src/stm32f10x_wwdg.d 


# Each subdirectory must supply rules for building sources it contributes
StdPeriph_Driver/src/%.o: ../StdPeriph_Driver/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -I"/home/andre/workspace/Example_SysTick/StdPeriph_Driver/inc" -I"/home/andre/workspace/Example_SysTick/inc" -I"/home/andre/workspace/Example_SysTick/CMSIS/device" -I"/home/andre/workspace/Example_SysTick/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


