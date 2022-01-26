################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
main.obj: ../main.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"D:/WinSoft/CCS/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/bin/cl430" -vmspx --use_hw_mpy=F5 --include_path="D:/WinSoft/CCS/ccsv8/ccs_base/msp430/include" --include_path="D:/Biblioteca/workspace_v8/emptyProject" --include_path="D:/Biblioteca/workspace_v8/emptyProject/driverlib/MSP430FR2xx_4xx" --include_path="D:/WinSoft/CCS/ccsv8/tools/compiler/ti-cgt-msp430_18.1.1.LTS/include" --advice:hw_config=all --define=__MSP430FR2433__ --define=DEPRECATED -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="main.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


