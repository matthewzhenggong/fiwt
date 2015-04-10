#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-XC16_dsPIC33EP512MU810.mk)" "nbproject/Makefile-local-XC16_dsPIC33EP512MU810.mk"
include nbproject/Makefile-local-XC16_dsPIC33EP512MU810.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=XC16_dsPIC33EP512MU810
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/FlightInWindTunnel.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/FlightInWindTunnel.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=configuration_bits.c main.c system.c traps.c user.c XBeeZBS2.c SerialStream.c task.c clock.c idle.c D:/source/fiwt/FlightInWindTunnel.X/ADC1.c D:/source/fiwt/FlightInWindTunnel.X/AnalogInput.c D:/source/fiwt/FlightInWindTunnel.X/PWMx.c D:/source/fiwt/FlightInWindTunnel.X/Servo.c D:/source/fiwt/FlightInWindTunnel.X/servoTask.c D:/source/fiwt/FlightInWindTunnel.X/UART1.c D:/source/fiwt/FlightInWindTunnel.X/UART2.c D:/source/fiwt/FlightInWindTunnel.X/UART3.c D:/source/fiwt/FlightInWindTunnel.X/UART4.c Enc.c IMU.c SPIS.c msg_comp.c msg_ac.c msg_gnd.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/configuration_bits.o ${OBJECTDIR}/main.o ${OBJECTDIR}/system.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/user.o ${OBJECTDIR}/XBeeZBS2.o ${OBJECTDIR}/SerialStream.o ${OBJECTDIR}/task.o ${OBJECTDIR}/clock.o ${OBJECTDIR}/idle.o ${OBJECTDIR}/_ext/747220533/ADC1.o ${OBJECTDIR}/_ext/747220533/AnalogInput.o ${OBJECTDIR}/_ext/747220533/PWMx.o ${OBJECTDIR}/_ext/747220533/Servo.o ${OBJECTDIR}/_ext/747220533/servoTask.o ${OBJECTDIR}/_ext/747220533/UART1.o ${OBJECTDIR}/_ext/747220533/UART2.o ${OBJECTDIR}/_ext/747220533/UART3.o ${OBJECTDIR}/_ext/747220533/UART4.o ${OBJECTDIR}/Enc.o ${OBJECTDIR}/IMU.o ${OBJECTDIR}/SPIS.o ${OBJECTDIR}/msg_comp.o ${OBJECTDIR}/msg_ac.o ${OBJECTDIR}/msg_gnd.o
POSSIBLE_DEPFILES=${OBJECTDIR}/configuration_bits.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/system.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/user.o.d ${OBJECTDIR}/XBeeZBS2.o.d ${OBJECTDIR}/SerialStream.o.d ${OBJECTDIR}/task.o.d ${OBJECTDIR}/clock.o.d ${OBJECTDIR}/idle.o.d ${OBJECTDIR}/_ext/747220533/ADC1.o.d ${OBJECTDIR}/_ext/747220533/AnalogInput.o.d ${OBJECTDIR}/_ext/747220533/PWMx.o.d ${OBJECTDIR}/_ext/747220533/Servo.o.d ${OBJECTDIR}/_ext/747220533/servoTask.o.d ${OBJECTDIR}/_ext/747220533/UART1.o.d ${OBJECTDIR}/_ext/747220533/UART2.o.d ${OBJECTDIR}/_ext/747220533/UART3.o.d ${OBJECTDIR}/_ext/747220533/UART4.o.d ${OBJECTDIR}/Enc.o.d ${OBJECTDIR}/IMU.o.d ${OBJECTDIR}/SPIS.o.d ${OBJECTDIR}/msg_comp.o.d ${OBJECTDIR}/msg_ac.o.d ${OBJECTDIR}/msg_gnd.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/configuration_bits.o ${OBJECTDIR}/main.o ${OBJECTDIR}/system.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/user.o ${OBJECTDIR}/XBeeZBS2.o ${OBJECTDIR}/SerialStream.o ${OBJECTDIR}/task.o ${OBJECTDIR}/clock.o ${OBJECTDIR}/idle.o ${OBJECTDIR}/_ext/747220533/ADC1.o ${OBJECTDIR}/_ext/747220533/AnalogInput.o ${OBJECTDIR}/_ext/747220533/PWMx.o ${OBJECTDIR}/_ext/747220533/Servo.o ${OBJECTDIR}/_ext/747220533/servoTask.o ${OBJECTDIR}/_ext/747220533/UART1.o ${OBJECTDIR}/_ext/747220533/UART2.o ${OBJECTDIR}/_ext/747220533/UART3.o ${OBJECTDIR}/_ext/747220533/UART4.o ${OBJECTDIR}/Enc.o ${OBJECTDIR}/IMU.o ${OBJECTDIR}/SPIS.o ${OBJECTDIR}/msg_comp.o ${OBJECTDIR}/msg_ac.o ${OBJECTDIR}/msg_gnd.o

# Source Files
SOURCEFILES=configuration_bits.c main.c system.c traps.c user.c XBeeZBS2.c SerialStream.c task.c clock.c idle.c D:/source/fiwt/FlightInWindTunnel.X/ADC1.c D:/source/fiwt/FlightInWindTunnel.X/AnalogInput.c D:/source/fiwt/FlightInWindTunnel.X/PWMx.c D:/source/fiwt/FlightInWindTunnel.X/Servo.c D:/source/fiwt/FlightInWindTunnel.X/servoTask.c D:/source/fiwt/FlightInWindTunnel.X/UART1.c D:/source/fiwt/FlightInWindTunnel.X/UART2.c D:/source/fiwt/FlightInWindTunnel.X/UART3.c D:/source/fiwt/FlightInWindTunnel.X/UART4.c Enc.c IMU.c SPIS.c msg_comp.c msg_ac.c msg_gnd.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-XC16_dsPIC33EP512MU810.mk dist/${CND_CONF}/${IMAGE_TYPE}/FlightInWindTunnel.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP512MU810
MP_LINKER_FILE_OPTION=,--script=p33EP512MU810.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/configuration_bits.o: configuration_bits.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/configuration_bits.o.d 
	@${RM} ${OBJECTDIR}/configuration_bits.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  configuration_bits.c  -o ${OBJECTDIR}/configuration_bits.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/configuration_bits.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/configuration_bits.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/system.o: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/system.o.d 
	@${RM} ${OBJECTDIR}/system.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  system.c  -o ${OBJECTDIR}/system.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/system.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/system.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/traps.o: traps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/traps.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/user.o: user.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/user.o.d 
	@${RM} ${OBJECTDIR}/user.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  user.c  -o ${OBJECTDIR}/user.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/user.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/user.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/XBeeZBS2.o: XBeeZBS2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/XBeeZBS2.o.d 
	@${RM} ${OBJECTDIR}/XBeeZBS2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  XBeeZBS2.c  -o ${OBJECTDIR}/XBeeZBS2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/XBeeZBS2.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/XBeeZBS2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/SerialStream.o: SerialStream.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SerialStream.o.d 
	@${RM} ${OBJECTDIR}/SerialStream.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  SerialStream.c  -o ${OBJECTDIR}/SerialStream.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/SerialStream.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/SerialStream.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/task.o: task.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/task.o.d 
	@${RM} ${OBJECTDIR}/task.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  task.c  -o ${OBJECTDIR}/task.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/task.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/task.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/clock.o: clock.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/clock.o.d 
	@${RM} ${OBJECTDIR}/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  clock.c  -o ${OBJECTDIR}/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/clock.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/idle.o: idle.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/idle.o.d 
	@${RM} ${OBJECTDIR}/idle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  idle.c  -o ${OBJECTDIR}/idle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/idle.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/idle.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/ADC1.o: D:/source/fiwt/FlightInWindTunnel.X/ADC1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/ADC1.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/ADC1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/ADC1.c  -o ${OBJECTDIR}/_ext/747220533/ADC1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/ADC1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/ADC1.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/AnalogInput.o: D:/source/fiwt/FlightInWindTunnel.X/AnalogInput.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/AnalogInput.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/AnalogInput.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/AnalogInput.c  -o ${OBJECTDIR}/_ext/747220533/AnalogInput.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/AnalogInput.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/AnalogInput.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/PWMx.o: D:/source/fiwt/FlightInWindTunnel.X/PWMx.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/PWMx.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/PWMx.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/PWMx.c  -o ${OBJECTDIR}/_ext/747220533/PWMx.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/PWMx.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/PWMx.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/Servo.o: D:/source/fiwt/FlightInWindTunnel.X/Servo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/Servo.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/Servo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/Servo.c  -o ${OBJECTDIR}/_ext/747220533/Servo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/Servo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/Servo.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/servoTask.o: D:/source/fiwt/FlightInWindTunnel.X/servoTask.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/servoTask.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/servoTask.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/servoTask.c  -o ${OBJECTDIR}/_ext/747220533/servoTask.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/servoTask.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/servoTask.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/UART1.o: D:/source/fiwt/FlightInWindTunnel.X/UART1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART1.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/UART1.c  -o ${OBJECTDIR}/_ext/747220533/UART1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/UART1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/UART1.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/UART2.o: D:/source/fiwt/FlightInWindTunnel.X/UART2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART2.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/UART2.c  -o ${OBJECTDIR}/_ext/747220533/UART2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/UART2.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/UART2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/UART3.o: D:/source/fiwt/FlightInWindTunnel.X/UART3.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART3.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART3.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/UART3.c  -o ${OBJECTDIR}/_ext/747220533/UART3.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/UART3.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/UART3.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/UART4.o: D:/source/fiwt/FlightInWindTunnel.X/UART4.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART4.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART4.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/UART4.c  -o ${OBJECTDIR}/_ext/747220533/UART4.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/UART4.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/UART4.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Enc.o: Enc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Enc.o.d 
	@${RM} ${OBJECTDIR}/Enc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Enc.c  -o ${OBJECTDIR}/Enc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Enc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/Enc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/IMU.o: IMU.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/IMU.o.d 
	@${RM} ${OBJECTDIR}/IMU.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  IMU.c  -o ${OBJECTDIR}/IMU.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/IMU.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/IMU.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/SPIS.o: SPIS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SPIS.o.d 
	@${RM} ${OBJECTDIR}/SPIS.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  SPIS.c  -o ${OBJECTDIR}/SPIS.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/SPIS.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/SPIS.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/msg_comp.o: msg_comp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/msg_comp.o.d 
	@${RM} ${OBJECTDIR}/msg_comp.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  msg_comp.c  -o ${OBJECTDIR}/msg_comp.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/msg_comp.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/msg_comp.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/msg_ac.o: msg_ac.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/msg_ac.o.d 
	@${RM} ${OBJECTDIR}/msg_ac.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  msg_ac.c  -o ${OBJECTDIR}/msg_ac.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/msg_ac.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/msg_ac.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/msg_gnd.o: msg_gnd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/msg_gnd.o.d 
	@${RM} ${OBJECTDIR}/msg_gnd.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  msg_gnd.c  -o ${OBJECTDIR}/msg_gnd.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/msg_gnd.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/msg_gnd.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/configuration_bits.o: configuration_bits.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/configuration_bits.o.d 
	@${RM} ${OBJECTDIR}/configuration_bits.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  configuration_bits.c  -o ${OBJECTDIR}/configuration_bits.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/configuration_bits.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/configuration_bits.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/system.o: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/system.o.d 
	@${RM} ${OBJECTDIR}/system.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  system.c  -o ${OBJECTDIR}/system.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/system.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/system.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/traps.o: traps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/traps.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/user.o: user.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/user.o.d 
	@${RM} ${OBJECTDIR}/user.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  user.c  -o ${OBJECTDIR}/user.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/user.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/user.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/XBeeZBS2.o: XBeeZBS2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/XBeeZBS2.o.d 
	@${RM} ${OBJECTDIR}/XBeeZBS2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  XBeeZBS2.c  -o ${OBJECTDIR}/XBeeZBS2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/XBeeZBS2.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/XBeeZBS2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/SerialStream.o: SerialStream.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SerialStream.o.d 
	@${RM} ${OBJECTDIR}/SerialStream.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  SerialStream.c  -o ${OBJECTDIR}/SerialStream.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/SerialStream.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/SerialStream.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/task.o: task.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/task.o.d 
	@${RM} ${OBJECTDIR}/task.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  task.c  -o ${OBJECTDIR}/task.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/task.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/task.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/clock.o: clock.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/clock.o.d 
	@${RM} ${OBJECTDIR}/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  clock.c  -o ${OBJECTDIR}/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/clock.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/clock.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/idle.o: idle.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/idle.o.d 
	@${RM} ${OBJECTDIR}/idle.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  idle.c  -o ${OBJECTDIR}/idle.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/idle.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/idle.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/ADC1.o: D:/source/fiwt/FlightInWindTunnel.X/ADC1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/ADC1.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/ADC1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/ADC1.c  -o ${OBJECTDIR}/_ext/747220533/ADC1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/ADC1.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/ADC1.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/AnalogInput.o: D:/source/fiwt/FlightInWindTunnel.X/AnalogInput.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/AnalogInput.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/AnalogInput.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/AnalogInput.c  -o ${OBJECTDIR}/_ext/747220533/AnalogInput.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/AnalogInput.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/AnalogInput.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/PWMx.o: D:/source/fiwt/FlightInWindTunnel.X/PWMx.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/PWMx.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/PWMx.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/PWMx.c  -o ${OBJECTDIR}/_ext/747220533/PWMx.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/PWMx.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/PWMx.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/Servo.o: D:/source/fiwt/FlightInWindTunnel.X/Servo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/Servo.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/Servo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/Servo.c  -o ${OBJECTDIR}/_ext/747220533/Servo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/Servo.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/Servo.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/servoTask.o: D:/source/fiwt/FlightInWindTunnel.X/servoTask.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/servoTask.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/servoTask.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/servoTask.c  -o ${OBJECTDIR}/_ext/747220533/servoTask.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/servoTask.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/servoTask.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/UART1.o: D:/source/fiwt/FlightInWindTunnel.X/UART1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART1.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/UART1.c  -o ${OBJECTDIR}/_ext/747220533/UART1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/UART1.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/UART1.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/UART2.o: D:/source/fiwt/FlightInWindTunnel.X/UART2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART2.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/UART2.c  -o ${OBJECTDIR}/_ext/747220533/UART2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/UART2.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/UART2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/UART3.o: D:/source/fiwt/FlightInWindTunnel.X/UART3.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART3.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART3.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/UART3.c  -o ${OBJECTDIR}/_ext/747220533/UART3.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/UART3.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/UART3.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/747220533/UART4.o: D:/source/fiwt/FlightInWindTunnel.X/UART4.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/747220533" 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART4.o.d 
	@${RM} ${OBJECTDIR}/_ext/747220533/UART4.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  D:/source/fiwt/FlightInWindTunnel.X/UART4.c  -o ${OBJECTDIR}/_ext/747220533/UART4.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/747220533/UART4.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/_ext/747220533/UART4.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Enc.o: Enc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Enc.o.d 
	@${RM} ${OBJECTDIR}/Enc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Enc.c  -o ${OBJECTDIR}/Enc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Enc.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/Enc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/IMU.o: IMU.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/IMU.o.d 
	@${RM} ${OBJECTDIR}/IMU.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  IMU.c  -o ${OBJECTDIR}/IMU.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/IMU.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/IMU.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/SPIS.o: SPIS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SPIS.o.d 
	@${RM} ${OBJECTDIR}/SPIS.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  SPIS.c  -o ${OBJECTDIR}/SPIS.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/SPIS.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/SPIS.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/msg_comp.o: msg_comp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/msg_comp.o.d 
	@${RM} ${OBJECTDIR}/msg_comp.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  msg_comp.c  -o ${OBJECTDIR}/msg_comp.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/msg_comp.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/msg_comp.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/msg_ac.o: msg_ac.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/msg_ac.o.d 
	@${RM} ${OBJECTDIR}/msg_ac.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  msg_ac.c  -o ${OBJECTDIR}/msg_ac.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/msg_ac.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/msg_ac.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/msg_gnd.o: msg_gnd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/msg_gnd.o.d 
	@${RM} ${OBJECTDIR}/msg_gnd.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  msg_gnd.c  -o ${OBJECTDIR}/msg_gnd.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/msg_gnd.o.d"      -mno-eds-warn  -g -omf=elf -fast-math -O0 -falign-arrays -I"../pt" -mcci -msmart-io=1 -Werror -Wall -msfr-warn=on  -save-temps 
	@${FIXDEPS} "${OBJECTDIR}/msg_gnd.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/FlightInWindTunnel.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  libdsp-elf.a  
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/FlightInWindTunnel.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    libdsp-elf.a  -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -omf=elf -fast-math  -mreserve=data@0x1000:0x101B -mreserve=data@0x101C:0x101D -mreserve=data@0x101E:0x101F -mreserve=data@0x1020:0x1021 -mreserve=data@0x1022:0x1023 -mreserve=data@0x1024:0x1027 -mreserve=data@0x1028:0x104F   -Wl,--local-stack,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/FlightInWindTunnel.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  libdsp-elf.a 
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/FlightInWindTunnel.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    libdsp-elf.a  -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -fast-math -Wl,--local-stack,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}\\xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/FlightInWindTunnel.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/XC16_dsPIC33EP512MU810
	${RM} -r dist/XC16_dsPIC33EP512MU810

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
