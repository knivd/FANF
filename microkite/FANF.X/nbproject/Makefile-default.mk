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
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../fatfs/ff.c ../fatfs/pic32sd.c ../fatfs/ccsbcs.c ../../generic/atoms.c ../../generic/fanf.c ../../generic/memory.c ../../generic/shellw.c ../../generic/time_t.c ../main.c ../platform.c ../mk_words.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/2116833129/ff.o ${OBJECTDIR}/_ext/2116833129/pic32sd.o ${OBJECTDIR}/_ext/2116833129/ccsbcs.o ${OBJECTDIR}/_ext/713179191/atoms.o ${OBJECTDIR}/_ext/713179191/fanf.o ${OBJECTDIR}/_ext/713179191/memory.o ${OBJECTDIR}/_ext/713179191/shellw.o ${OBJECTDIR}/_ext/713179191/time_t.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/platform.o ${OBJECTDIR}/_ext/1472/mk_words.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/2116833129/ff.o.d ${OBJECTDIR}/_ext/2116833129/pic32sd.o.d ${OBJECTDIR}/_ext/2116833129/ccsbcs.o.d ${OBJECTDIR}/_ext/713179191/atoms.o.d ${OBJECTDIR}/_ext/713179191/fanf.o.d ${OBJECTDIR}/_ext/713179191/memory.o.d ${OBJECTDIR}/_ext/713179191/shellw.o.d ${OBJECTDIR}/_ext/713179191/time_t.o.d ${OBJECTDIR}/_ext/1472/main.o.d ${OBJECTDIR}/_ext/1472/platform.o.d ${OBJECTDIR}/_ext/1472/mk_words.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/2116833129/ff.o ${OBJECTDIR}/_ext/2116833129/pic32sd.o ${OBJECTDIR}/_ext/2116833129/ccsbcs.o ${OBJECTDIR}/_ext/713179191/atoms.o ${OBJECTDIR}/_ext/713179191/fanf.o ${OBJECTDIR}/_ext/713179191/memory.o ${OBJECTDIR}/_ext/713179191/shellw.o ${OBJECTDIR}/_ext/713179191/time_t.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/platform.o ${OBJECTDIR}/_ext/1472/mk_words.o

# Source Files
SOURCEFILES=../fatfs/ff.c ../fatfs/pic32sd.c ../fatfs/ccsbcs.c ../../generic/atoms.c ../../generic/fanf.c ../../generic/memory.c ../../generic/shellw.c ../../generic/time_t.c ../main.c ../platform.c ../mk_words.c


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
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX170F256D
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/2116833129/ff.o: ../fatfs/ff.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2116833129 
	@${RM} ${OBJECTDIR}/_ext/2116833129/ff.o.d 
	@${RM} ${OBJECTDIR}/_ext/2116833129/ff.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2116833129/ff.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/2116833129/ff.o.d" -o ${OBJECTDIR}/_ext/2116833129/ff.o ../fatfs/ff.c   
	
${OBJECTDIR}/_ext/2116833129/pic32sd.o: ../fatfs/pic32sd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2116833129 
	@${RM} ${OBJECTDIR}/_ext/2116833129/pic32sd.o.d 
	@${RM} ${OBJECTDIR}/_ext/2116833129/pic32sd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2116833129/pic32sd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/2116833129/pic32sd.o.d" -o ${OBJECTDIR}/_ext/2116833129/pic32sd.o ../fatfs/pic32sd.c   
	
${OBJECTDIR}/_ext/2116833129/ccsbcs.o: ../fatfs/ccsbcs.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2116833129 
	@${RM} ${OBJECTDIR}/_ext/2116833129/ccsbcs.o.d 
	@${RM} ${OBJECTDIR}/_ext/2116833129/ccsbcs.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2116833129/ccsbcs.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/2116833129/ccsbcs.o.d" -o ${OBJECTDIR}/_ext/2116833129/ccsbcs.o ../fatfs/ccsbcs.c   
	
${OBJECTDIR}/_ext/713179191/atoms.o: ../../generic/atoms.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/atoms.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/atoms.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/atoms.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/atoms.o.d" -o ${OBJECTDIR}/_ext/713179191/atoms.o ../../generic/atoms.c   
	
${OBJECTDIR}/_ext/713179191/fanf.o: ../../generic/fanf.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/fanf.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/fanf.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/fanf.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/fanf.o.d" -o ${OBJECTDIR}/_ext/713179191/fanf.o ../../generic/fanf.c   
	
${OBJECTDIR}/_ext/713179191/memory.o: ../../generic/memory.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/memory.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/memory.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/memory.o.d" -o ${OBJECTDIR}/_ext/713179191/memory.o ../../generic/memory.c   
	
${OBJECTDIR}/_ext/713179191/shellw.o: ../../generic/shellw.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/shellw.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/shellw.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/shellw.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/shellw.o.d" -o ${OBJECTDIR}/_ext/713179191/shellw.o ../../generic/shellw.c   
	
${OBJECTDIR}/_ext/713179191/time_t.o: ../../generic/time_t.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/time_t.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/time_t.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/time_t.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/time_t.o.d" -o ${OBJECTDIR}/_ext/713179191/time_t.o ../../generic/time_t.c   
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d" -o ${OBJECTDIR}/_ext/1472/main.o ../main.c   
	
${OBJECTDIR}/_ext/1472/platform.o: ../platform.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/platform.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/1472/platform.o.d" -o ${OBJECTDIR}/_ext/1472/platform.o ../platform.c   
	
${OBJECTDIR}/_ext/1472/mk_words.o: ../mk_words.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/mk_words.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/mk_words.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/mk_words.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/1472/mk_words.o.d" -o ${OBJECTDIR}/_ext/1472/mk_words.o ../mk_words.c   
	
else
${OBJECTDIR}/_ext/2116833129/ff.o: ../fatfs/ff.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2116833129 
	@${RM} ${OBJECTDIR}/_ext/2116833129/ff.o.d 
	@${RM} ${OBJECTDIR}/_ext/2116833129/ff.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2116833129/ff.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/2116833129/ff.o.d" -o ${OBJECTDIR}/_ext/2116833129/ff.o ../fatfs/ff.c   
	
${OBJECTDIR}/_ext/2116833129/pic32sd.o: ../fatfs/pic32sd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2116833129 
	@${RM} ${OBJECTDIR}/_ext/2116833129/pic32sd.o.d 
	@${RM} ${OBJECTDIR}/_ext/2116833129/pic32sd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2116833129/pic32sd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/2116833129/pic32sd.o.d" -o ${OBJECTDIR}/_ext/2116833129/pic32sd.o ../fatfs/pic32sd.c   
	
${OBJECTDIR}/_ext/2116833129/ccsbcs.o: ../fatfs/ccsbcs.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2116833129 
	@${RM} ${OBJECTDIR}/_ext/2116833129/ccsbcs.o.d 
	@${RM} ${OBJECTDIR}/_ext/2116833129/ccsbcs.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2116833129/ccsbcs.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/2116833129/ccsbcs.o.d" -o ${OBJECTDIR}/_ext/2116833129/ccsbcs.o ../fatfs/ccsbcs.c   
	
${OBJECTDIR}/_ext/713179191/atoms.o: ../../generic/atoms.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/atoms.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/atoms.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/atoms.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/atoms.o.d" -o ${OBJECTDIR}/_ext/713179191/atoms.o ../../generic/atoms.c   
	
${OBJECTDIR}/_ext/713179191/fanf.o: ../../generic/fanf.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/fanf.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/fanf.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/fanf.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/fanf.o.d" -o ${OBJECTDIR}/_ext/713179191/fanf.o ../../generic/fanf.c   
	
${OBJECTDIR}/_ext/713179191/memory.o: ../../generic/memory.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/memory.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/memory.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/memory.o.d" -o ${OBJECTDIR}/_ext/713179191/memory.o ../../generic/memory.c   
	
${OBJECTDIR}/_ext/713179191/shellw.o: ../../generic/shellw.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/shellw.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/shellw.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/shellw.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/shellw.o.d" -o ${OBJECTDIR}/_ext/713179191/shellw.o ../../generic/shellw.c   
	
${OBJECTDIR}/_ext/713179191/time_t.o: ../../generic/time_t.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/713179191 
	@${RM} ${OBJECTDIR}/_ext/713179191/time_t.o.d 
	@${RM} ${OBJECTDIR}/_ext/713179191/time_t.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/713179191/time_t.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/713179191/time_t.o.d" -o ${OBJECTDIR}/_ext/713179191/time_t.o ../../generic/time_t.c   
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d" -o ${OBJECTDIR}/_ext/1472/main.o ../main.c   
	
${OBJECTDIR}/_ext/1472/platform.o: ../platform.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/platform.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/platform.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/1472/platform.o.d" -o ${OBJECTDIR}/_ext/1472/platform.o ../platform.c   
	
${OBJECTDIR}/_ext/1472/mk_words.o: ../mk_words.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/mk_words.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/mk_words.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/mk_words.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -fdata-sections -mlong-calls -fomit-frame-pointer -DMAX_PROCESS=9 -DMORE_WORDS_FILE="..\microkite\mk_words.h" -DREAL="long double" -MMD -MF "${OBJECTDIR}/_ext/1472/mk_words.o.d" -o ${OBJECTDIR}/_ext/1472/mk_words.o ../mk_words.c   
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mdebugger -D__MPLAB_DEBUGGER_PK3=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}           -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC00490:0x1FC00BEF -mreserve=boot@0x1FC00490:0x1FC00BEF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,--defsym=_min_heap_size=46336,--defsym=_min_stack_size=4096,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--cref
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=46336,--defsym=_min_stack_size=4096,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--cref
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
	@echo Normalizing hex file
	@"C:/Program Files (x86)/Microchip/MPLABX/mplab_ide/mplab_ide/modules/../../bin/hexmate" --edf="C:/Program Files (x86)/Microchip/MPLABX/mplab_ide/mplab_ide/modules/../../dat/en_msgs.txt" dist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.hex -odist/${CND_CONF}/${IMAGE_TYPE}/FANF.X.${IMAGE_TYPE}.hex

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

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
