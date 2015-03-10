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
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=cof
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/DIDV_firmware.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/DIDV_firmware.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=usb_includes/cdc.c usb_includes/usb_functions.c usb_includes/usb_stack.c main.c tlc5940.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/usb_includes/cdc.o ${OBJECTDIR}/usb_includes/usb_functions.o ${OBJECTDIR}/usb_includes/usb_stack.o ${OBJECTDIR}/main.o ${OBJECTDIR}/tlc5940.o
POSSIBLE_DEPFILES=${OBJECTDIR}/usb_includes/cdc.o.d ${OBJECTDIR}/usb_includes/usb_functions.o.d ${OBJECTDIR}/usb_includes/usb_stack.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/tlc5940.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/usb_includes/cdc.o ${OBJECTDIR}/usb_includes/usb_functions.o ${OBJECTDIR}/usb_includes/usb_stack.o ${OBJECTDIR}/main.o ${OBJECTDIR}/tlc5940.o

# Source Files
SOURCEFILES=usb_includes/cdc.c usb_includes/usb_functions.c usb_includes/usb_stack.c main.c tlc5940.c


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
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/DIDV_firmware.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F4550
MP_PROCESSOR_OPTION_LD=18f4550
MP_LINKER_DEBUG_OPTION= -u_DEBUGCODESTART=0x7dc0 -u_DEBUGCODELEN=0x240 -u_DEBUGDATASTART=0x3f4 -u_DEBUGDATALEN=0xb
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/usb_includes/cdc.o: usb_includes/cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/usb_includes" 
	@${RM} ${OBJECTDIR}/usb_includes/cdc.o.d 
	@${RM} ${OBJECTDIR}/usb_includes/cdc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/usb_includes/cdc.o   usb_includes/cdc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/usb_includes/cdc.o 
	@${FIXDEPS} "${OBJECTDIR}/usb_includes/cdc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/usb_includes/usb_functions.o: usb_includes/usb_functions.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/usb_includes" 
	@${RM} ${OBJECTDIR}/usb_includes/usb_functions.o.d 
	@${RM} ${OBJECTDIR}/usb_includes/usb_functions.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/usb_includes/usb_functions.o   usb_includes/usb_functions.c 
	@${DEP_GEN} -d ${OBJECTDIR}/usb_includes/usb_functions.o 
	@${FIXDEPS} "${OBJECTDIR}/usb_includes/usb_functions.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/usb_includes/usb_stack.o: usb_includes/usb_stack.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/usb_includes" 
	@${RM} ${OBJECTDIR}/usb_includes/usb_stack.o.d 
	@${RM} ${OBJECTDIR}/usb_includes/usb_stack.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/usb_includes/usb_stack.o   usb_includes/usb_stack.c 
	@${DEP_GEN} -d ${OBJECTDIR}/usb_includes/usb_stack.o 
	@${FIXDEPS} "${OBJECTDIR}/usb_includes/usb_stack.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/main.o   main.c 
	@${DEP_GEN} -d ${OBJECTDIR}/main.o 
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/tlc5940.o: tlc5940.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/tlc5940.o.d 
	@${RM} ${OBJECTDIR}/tlc5940.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/tlc5940.o   tlc5940.c 
	@${DEP_GEN} -d ${OBJECTDIR}/tlc5940.o 
	@${FIXDEPS} "${OBJECTDIR}/tlc5940.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
else
${OBJECTDIR}/usb_includes/cdc.o: usb_includes/cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/usb_includes" 
	@${RM} ${OBJECTDIR}/usb_includes/cdc.o.d 
	@${RM} ${OBJECTDIR}/usb_includes/cdc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/usb_includes/cdc.o   usb_includes/cdc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/usb_includes/cdc.o 
	@${FIXDEPS} "${OBJECTDIR}/usb_includes/cdc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/usb_includes/usb_functions.o: usb_includes/usb_functions.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/usb_includes" 
	@${RM} ${OBJECTDIR}/usb_includes/usb_functions.o.d 
	@${RM} ${OBJECTDIR}/usb_includes/usb_functions.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/usb_includes/usb_functions.o   usb_includes/usb_functions.c 
	@${DEP_GEN} -d ${OBJECTDIR}/usb_includes/usb_functions.o 
	@${FIXDEPS} "${OBJECTDIR}/usb_includes/usb_functions.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/usb_includes/usb_stack.o: usb_includes/usb_stack.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/usb_includes" 
	@${RM} ${OBJECTDIR}/usb_includes/usb_stack.o.d 
	@${RM} ${OBJECTDIR}/usb_includes/usb_stack.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/usb_includes/usb_stack.o   usb_includes/usb_stack.c 
	@${DEP_GEN} -d ${OBJECTDIR}/usb_includes/usb_stack.o 
	@${FIXDEPS} "${OBJECTDIR}/usb_includes/usb_stack.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/main.o: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/main.o   main.c 
	@${DEP_GEN} -d ${OBJECTDIR}/main.o 
	@${FIXDEPS} "${OBJECTDIR}/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/tlc5940.o: tlc5940.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/tlc5940.o.d 
	@${RM} ${OBJECTDIR}/tlc5940.o 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -ms -oa- -o-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/tlc5940.o   tlc5940.c 
	@${DEP_GEN} -d ${OBJECTDIR}/tlc5940.o 
	@${FIXDEPS} "${OBJECTDIR}/tlc5940.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/DIDV_firmware.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    rm18f4550\ -\ HID\ Bootload.lkr
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) "rm18f4550 - HID Bootload.lkr"  -p$(MP_PROCESSOR_OPTION_LD)  -w -x -u_DEBUG -m"${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"  -z__MPLAB_BUILD=1  -u_CRUNTIME -z__MPLAB_DEBUG=1 -z__MPLAB_DEBUGGER_PK3=1 $(MP_LINKER_DEBUG_OPTION) -l ${MP_CC_DIR}/../lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/DIDV_firmware.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}   
else
dist/${CND_CONF}/${IMAGE_TYPE}/DIDV_firmware.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   rm18f4550\ -\ HID\ Bootload.lkr
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) "rm18f4550 - HID Bootload.lkr"  -p$(MP_PROCESSOR_OPTION_LD)  -w  -m"${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map"  -z__MPLAB_BUILD=1  -u_CRUNTIME -l ${MP_CC_DIR}/../lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/DIDV_firmware.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}   
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
