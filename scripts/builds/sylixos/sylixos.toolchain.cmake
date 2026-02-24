#*********************************************************************************************************
#
#                                    中国软件开源组织
#
#                                   嵌入式实时操作系统
#
#                                SylixOS(TM)  LW : long wing
#
#                               Copyright All Rights Reserved
#
#--------------文件信息--------------------------------------------------------------------------------
#
# 文   件   名: sylixos.toolchain.cmake
#
# 创   建   人: lijiaming(李佳明)
#
# 文件创建日期: 2022 年 06 月 03 日
#
# 描        述: SylixOS cmake toolchain file
#*********************************************************************************************************

set(CMAKE_SYSTEM_NAME SylixOS)
set(SYLIXOS ON)
set(ENV{SYLIXOS} ON)

message(STATUS "CMAKE_SYSTEM_NAME is ${CMAKE_SYSTEM_NAME}")
message(STATUS "SYLIXOS is ${SYLIXOS}")
#*********************************************************************************************************
# config.mk
#*********************************************************************************************************
# set (CMAKE_SYLIXOS_BASE_PATH D:/work/projects/vsoa_ai_sdk/sophon_sylixos_proj/aarch64_base)
if(CMAKE_SYLIXOS_BASE_PATH)
    string(REPLACE "\\" "/" result "${CMAKE_SYLIXOS_BASE_PATH}")
    set(CMAKE_SYLIXOS_BASE_PATH ${result})
    set(ENV{SYLIXOS_BASE_PATH} "${CMAKE_SYLIXOS_BASE_PATH}")
else()
    return()
endif()

set(SYLIXOS_BASE_PATH $ENV{SYLIXOS_BASE_PATH})

message(STATUS "SylixOS Base Path: ${SYLIXOS_BASE_PATH}")

if(NOT EXISTS ${SYLIXOS_BASE_PATH})
    message(FATAL_ERROR "${SYLIXOS_BASE_PATH} not found.")
endif()

if(SYLIXOS_BASE_PATH)
    file(STRINGS "${SYLIXOS_BASE_PATH}/config.mk" MK_STRINGS REGEX "^MULTI_PLATFORM_BUILD := .*")
    string(REPLACE "MULTI_PLATFORM_BUILD := " "" MULTI_PLATFORM_BUILD "${MK_STRINGS}")

    message(STATUS "MULTI_PLATFORM_BUILD := ${MULTI_PLATFORM_BUILD}")
    if(MULTI_PLATFORM_BUILD STREQUAL "yes")
        message(MESSAGE "${SYLIXOS_BASE_PATH} is multi platform build. supporting.")
        # 解析 PLATFORMS
        file(STRINGS "${SYLIXOS_BASE_PATH}/config.mk" PLATFORM_STRINGS REGEX "^PLATFORMS := .*")
        string(REPLACE "PLATFORMS := " "" PLATFORMS "${PLATFORM_STRINGS}")
        string(STRIP "${PLATFORMS}" PLATFORMS) # 去除首尾空格
        message(STATUS "Detected PLATFORMS: ${PLATFORMS}")

        # 取第一个平台（通常 MULTI_PLATFORM_BUILD 下也只用一个目标平台）
        list(GET PLATFORMS 0 TARGET_PLATFORM)
        message(STATUS "Target platform: ${TARGET_PLATFORM}")

        # 读取 platforms.mk
        if(EXISTS "${SYLIXOS_BASE_PATH}/platforms.mk")
            file(READ "${SYLIXOS_BASE_PATH}/platforms.mk" PLATFORMS_CONTENT)

            # 按行分割
            string(REPLACE "\n" ";" PLATFORMS_LINES "${PLATFORMS_CONTENT}")

            set(FOUND FALSE)
            foreach(LINE IN LISTS PLATFORMS_LINES)
                # 去除首尾空白
                string(STRIP "${LINE}" STRIPPED_LINE)

                # 跳过空行和注释
                if(STRIPPED_LINE STREQUAL "" OR STRIPPED_LINE MATCHES "^#")
                    continue()
                endif()
                # 查找以 TARGET_PLATFORM 开头的行
                if(NOT FOUND AND STRIPPED_LINE MATCHES "^${TARGET_PLATFORM}")
                    set(FOUND TRUE)
                    # === 开始解析当前行 ===
                    # 使用 separate_arguments 按空白分割成列表
                    separate_arguments(COLS NATIVE_COMMAND "${STRIPPED_LINE}")
                    list(LENGTH COLS COL_COUNT)

                    if(COL_COUNT LESS 5)
                        message(FATAL_ERROR "Platform line for ${TARGET_PLATFORM} has fewer than 5 columns: ${STRIPPED_LINE}")
                    endif()

                    # 第1列是平台名（已知），跳过
                    # 第2列是 TOOLCHAIN_PREFIX（注意它前面有 :=，但实际是第二项）
                    # 示例分解: [ARM64_GENERIC, :=, aarch64-sylixos-elf-, generic, reserve, default]
                    # 我们关注的是 := 之后的内容

                    # 找到 := 的位置，其后第一个非操作符项是 TOOLCHAIN_PREFIX
                    list(FIND COLS ":=" COLON_EQUAL_INDEX)
                    if(COLON_EQUAL_INDEX EQUAL -1)
                        message(FATAL_ERROR "':=' not found in platform line for ${TARGET_PLATFORM}")
                    endif()

                    # TOOLCHAIN_PREFIX 是 := 之后的第一个有效值
                    math(EXPR TOOLCHAIN_IDX "${COLON_EQUAL_INDEX} + 1")
                    if (TOOLCHAIN_IDX LESS COL_COUNT)
                        list(GET COLS ${TOOLCHAIN_IDX} TOOLCHAIN_PREFIX)
                        # 强制转为字符串并去空格
                        set(TOOLCHAIN_PREFIX "${TOOLCHAIN_PREFIX}")
                        string(STRIP "${TOOLCHAIN_PREFIX}" TOOLCHAIN_PREFIX)
                        # 再次确保没有 \r\n
                        string(REGEX REPLACE "[\r\n]$" "" TOOLCHAIN_PREFIX "${TOOLCHAIN_PREFIX}")
                    else()
                        message(FATAL_ERROR "TOOLCHAIN_PREFIX not found after ':=' for ${TARGET_PLATFORM}")
                    endif()

                    # CPU_TYPE 是 TOOLCHAIN_IDX + 1
                    math(EXPR CPU_IDX "${TOOLCHAIN_IDX} + 1")
                    if (CPU_IDX LESS COL_COUNT)
                        list(GET COLS ${CPU_IDX} CPU_TYPE)
                        set(CPU_TYPE "${CPU_TYPE}")
                        string(STRIP "${CPU_TYPE}" CPU_TYPE)
                    else()
                        message(FATAL_ERROR "CPU_TYPE not found for ${TARGET_PLATFORM}")
                    endif()

                    list(GET COLS 5 FPU_TYPE)
                    set(FPU_TYPE "${FPU_TYPE}")
                    string(STRIP "${FPU_TYPE}" FPU_TYPE)

                    # 输出结果
                    message(STATUS "TOOLCHAIN_PREFIX := ${TOOLCHAIN_PREFIX}")
                    message(STATUS "CPU_TYPE := ${CPU_TYPE}")
                    message(STATUS "FPU_TYPE := ${FPU_TYPE}")

                    # 解析完成，跳出循环
                    break()
                endif()
            endforeach()

            if(NOT FOUND)
                message(FATAL_ERROR "Platform '${TARGET_PLATFORM}' not found in platforms.mk")
            endif()

            if(NOT DEFINED TOOLCHAIN_PREFIX)
                message(FATAL_ERROR "TOOLCHAIN_PREFIX not defined for platform ${TARGET_PLATFORM}")
            endif()
            if(NOT DEFINED CPU_TYPE)
                message(FATAL_ERROR "CPU_TYPE not defined for platform ${TARGET_PLATFORM}")
            endif()

            file(STRINGS "${SYLIXOS_BASE_PATH}/config.mk" MK_STRINGS REGEX "^DEBUG_LEVEL := .*")
            string(REPLACE "DEBUG_LEVEL := " "" DEBUG_LEVEL "${MK_STRINGS}")
            # FPU_TYPE 可选，若必要也可检查

        else()
            message(FATAL_ERROR "platforms.mk not found at ${SYLIXOS_BASE_PATH}/platforms.mk")
        endif()
    else()
        file(STRINGS "${SYLIXOS_BASE_PATH}/config.mk" MK_STRINGS REGEX "^TOOLCHAIN_PREFIX := .*")
        if(NOT MK_STRINGS)
            message(FATAL_ERROR "TOOLCHAIN_PREFIX not found in config.mk")
        endif()
        list(GET MK_STRINGS 0 TOOLCHAIN_PREFIX_LINE)
        string(REPLACE "TOOLCHAIN_PREFIX := " "" TOOLCHAIN_PREFIX "${TOOLCHAIN_PREFIX_LINE}")
        string(STRIP "${TOOLCHAIN_PREFIX}" TOOLCHAIN_PREFIX) # 去空格

        file(STRINGS "${SYLIXOS_BASE_PATH}/config.mk" MK_STRINGS REGEX "^CPU_TYPE := .*")
        if(MK_STRINGS)
            list(GET MK_STRINGS 0 CPU_LINE)
            string(REPLACE "CPU_TYPE := " "" CPU_TYPE "${CPU_LINE}")
            string(STRIP "${CPU_TYPE}" CPU_TYPE)
        endif()

        file(STRINGS "${SYLIXOS_BASE_PATH}/config.mk" MK_STRINGS REGEX "^FPU_TYPE := .*")
        if(MK_STRINGS)
            list(GET MK_STRINGS 0 FPU_LINE)
            string(REPLACE "FPU_TYPE := " "" FPU_TYPE "${FPU_LINE}")
            string(STRIP "${FPU_TYPE}" FPU_TYPE)
        endif()

        file(STRINGS "${SYLIXOS_BASE_PATH}/config.mk" MK_STRINGS REGEX "^DEBUG_LEVEL := .*")
        string(REPLACE "DEBUG_LEVEL := " "" DEBUG_LEVEL "${MK_STRINGS}")
    endif()
endif()

message(STATUS "TOOLCHAIN_PREFIX := ${TOOLCHAIN_PREFIX}")
message(STATUS "FPU_TYPE := ${FPU_TYPE}")
message(STATUS "CPU_TYPE := ${CPU_TYPE}")
message(STATUS "DEBUG_LEVEL := ${DEBUG_LEVEL}")
string(TOUPPER ${DEBUG_LEVEL} upper_debug_level)
if(upper_debug_level STREQUAL "DEBUG")
    set(DEBUG_LEVEL "Debug")
else()
    set(DEBUG_LEVEL "Release")
endif()


#*********************************************************************************************************
# target.mk
#*********************************************************************************************************
# set(LOCAL_INC_PATH         "")
# set(LOCAL_DSYMBOL          "")
# set(LOCAL_CFLAGS           "")
# set(LOCAL_CXXFLAGS         "")
# set(LOCAL_LINKFLAGS        "")
# set(LOCAL_DEPEND_LIB       "")
# set(LOCAL_DEPEND_LIB_PATH  "")
set(LOCAL_USE_CXX "yes")
set(LOCAL_USE_CXX_EXCEPT "yes")
set(LOCAL_USE_GCOV "no")
set(LOCAL_USE_OMP "no")
set(LOCAL_NO_UNDEF_SYM "no")

if(FPU_TYPE AND NOT (FPU_TYPE MATCHES "disable") AND NOT (FPU_TYPE MATCHES "default"))
    if((FPU_TYPE MATCHES "hard-float") OR (FPU_TYPE MATCHES "soft-float") OR (FPU_TYPE MATCHES "double-float") OR (FPU_TYPE MATCHES "float-abi=soft") OR (FPU_TYPE MATCHES "float-abi=hard"))
        set(ARCH_FPUFLAGS "-m${FPU_TYPE}")
    elseif(FPU_TYPE MATCHES "softfp")
        set(ARCH_FPUFLAGS "-mfloat-abi=softfp")
    else()
        set(ARCH_FPUFLAGS "-mfloat-abi=softfp -mfpu=${FPU_TYPE}")
    endif()
endif()

#*********************************************************************************************************
# arch.mk
#*********************************************************************************************************
message(STATUS "TOOLCHAIN_PREFIX = '${TOOLCHAIN_PREFIX}'")
message(STATUS "TOOLCHAIN_PREFIX length = '${TOOLCHAIN_PREFIX}'" " -> length: ${CMAKE_MATCH_0}")
string(LENGTH "${TOOLCHAIN_PREFIX}" LEN)
message(STATUS "TOOLCHAIN_PREFIX length: ${LEN}")
#*********************************************************************************************************
# i386
#*********************************************************************************************************
if ("${TOOLCHAIN_PREFIX}" STREQUAL "i386-sylixos-elf-")
    set(CMAKE_SYSTEM_PROCESSOR "x86")
    set(ARCH_COMMONFLAGS "-mlong-double-64 -fno-omit-frame-pointer -fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "")
    set(ARCH_KO_LDFLAGS "-nostdlib -r")

    set(ARCH_KLIB_CFLAGS "")

    set(ARCH_KERNEL_CFLAGS "")
    set(ARCH_KERNEL_LDFLAGS "")

    set(ARCH_CPUFLAGS "-march=${CPU_TYPE} ${ARCH_FPUFLAGS}")

    #*********************************************************************************************************
    # x86-64
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "x86_64-sylixos-elf-")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")

    set(ARCH_COMMONFLAGS "-mlong-double-64 -mno-red-zone -fno-omit-frame-pointer -fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "-mcmodel=large")
    set(ARCH_KO_LDFLAGS "-nostdlib -r")

    set(ARCH_KLIB_CFLAGS "-mcmodel=large")

    set(ARCH_KERNEL_CFLAGS "-mcmodel=kernel")
    set(ARCH_KERNEL_LDFLAGS "-z max-page-size=4096")

    if(CPU_TYPE AND NOT (FPU_TYPE MATCHES "generic"))
        set(ARCH_CPUFLAGS "-march=${CPU_TYPE} -m64 ${ARCH_FPUFLAGS}")
    else()
        set(ARCH_CPUFLAGS "-m64 ${ARCH_FPUFLAGS}")
    endif()

#*********************************************************************************************************
# ARM
#*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "arm-sylixos-eabi-")
    set(CMAKE_SYSTEM_PROCESSOR "arm")

    set(ARCH_COMMONFLAGS "-mno-unaligned-access -fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-nostdlib -Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "")
    set(ARCH_KO_LDFLAGS "-nostdlib -r")

    set(ARCH_KLIB_CFLAGS "")

    set(ARCH_KERNEL_CFLAGS "")
    set(ARCH_KERNEL_LDFLAGS "")
    set(ARCH_CPUFLAGS "-mcpu=${CPU_TYPE} ${ARCH_FPUFLAGS}")

    #*********************************************************************************************************
    # aarch64
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "aarch64-sylixos-elf-")

    message(STATUS "✅ Entered aarch64 branch")

    set(CMAKE_SYSTEM_PROCESSOR "aarch64")

    set(ARCH_COMMONFLAGS "-fno-omit-frame-pointer -mstrict-align -ffixed-x18 -fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "-mcmodel=large -mgeneral-regs-only")
    set(ARCH_KO_LDFLAGS "-nostdlib -r")

    set(ARCH_KLIB_CFLAGS "")

    set(ARCH_KERNEL_CFLAGS "-mgeneral-regs-only")
    set(ARCH_KERNEL_LDFLAGS "-Wl,--build-id=none")
    set(ARCH_CPUFLAGS "-mcpu=${CPU_TYPE} ${ARCH_FPUFLAGS}")

    #*********************************************************************************************************
    # MIPS
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "mips-sylixos-elf-")
    set(CMAKE_SYSTEM_PROCESSOR "mips")

    set(ARCH_COMMONFLAGS "-fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC -mabicalls")
    set(ARCH_PIC_LDFLAGS "-mabicalls -Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "-mlong-calls")
    set(ARCH_KO_LDFLAGS "-nostdlib -r")

    set(ARCH_KLIB_CFLAGS "-mlong-calls")

    set(ARCH_KERNEL_CFLAGS "")
    set(ARCH_KERNEL_LDFLAGS "")

    set(ARCH_CPUFLAGS "-march=${CPU_TYPE} -EL -G 0 -mno-branch-likely ${ARCH_FPUFLAGS}")

    #*********************************************************************************************************
    # MIPS64
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "mips64-sylixos-elf-")
    set(CMAKE_SYSTEM_PROCESSOR "mips64")

    set(ARCH_COMMONFLAGS "-fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC -mabicalls")
    set(ARCH_PIC_LDFLAGS "-mabicalls -Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "-mlong-calls")
    set(ARCH_KO_LDFLAGS "-nostdlib -r")

    set(ARCH_KLIB_CFLAGS "-mlong-calls")

    set(ARCH_KERNEL_CFLAGS "")
    set(ARCH_KERNEL_LDFLAGS "")

    set(ARCH_CPUFLAGS "-march=${CPU_TYPE} -EL -G 0 -mno-branch-likely -mabi=64 ${ARCH_FPUFLAGS}")

    #*********************************************************************************************************
    # PowerPC
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "ppc-sylixos-eabi-")
    set(CMAKE_SYSTEM_PROCESSOR "powerpc")

    set(ARCH_COMMONFLAGS "-G 0 -mstrict-align -fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "")
    set(ARCH_KO_LDFLAGS "-nostdlib -r")

    set(ARCH_KLIB_CFLAGS "")

    set(ARCH_KERNEL_CFLAGS "")
    set(ARCH_KERNEL_LDFLAGS "")
    set(ARCH_CPUFLAGS "-mcpu=${CPU_TYPE} ${ARCH_FPUFLAGS}")

    #*********************************************************************************************************
    # SPARC
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "sparc-sylixos-elf-")
    set(CMAKE_SYSTEM_PROCESSOR "sparc")

    set(ARCH_COMMONFLAGS "-fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "")
    set(ARCH_KO_LDFLAGS "-nostdlib -r")

    set(ARCH_KLIB_CFLAGS "")

    set(ARCH_KERNEL_CFLAGS "")
    set(ARCH_KERNEL_LDFLAGS "")
    set(ARCH_CPUFLAGS "-mcpu=${CPU_TYPE} ${ARCH_FPUFLAGS}")

    #*********************************************************************************************************
    # RISC-V
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "riscv-sylixos-elf-")
    set(CMAKE_SYSTEM_PROCESSOR "riscv")

    set(ARCH_COMMONFLAGS "-mstrict-align -mcmodel=medany -mno-save-restore -fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-Wl,-shared -fPIC -shared")

    set(ARCH_KO_CFLAGS "-fPIC")
    set(ARCH_KO_LDFLAGS "-r -fPIC")

    set(ARCH_KLIB_CFLAGS "")

    set(ARCH_KERNEL_CFLAGS "")
    set(ARCH_KERNEL_LDFLAGS "")
    set(ARCH_CPUFLAGS "-march=${CPU_TYPE} -mabi=${FPU_TYPE}")

    #*********************************************************************************************************
    # C-SKY
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "csky-sylixos-elfabiv2-")
    set(CMAKE_SYSTEM_PROCESSOR "csky")

    set(ARCH_COMMONFLAGS "-fno-omit-frame-pointer -fno-strict-aliasing")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-Wl,-shared -fPIC -Wl,-z,max-page-size=65536")

    set(ARCH_KO_CFLAGS "-Wa,-mno-force2bsr")
    set(ARCH_KO_LDFLAGS "-nostdlib -r -Wl,-z,max-page-size=65536")

    set(ARCH_KLIB_CFLAGS "-Wa,-mno-force2bsr")

    set(ARCH_KERNEL_CFLAGS "")
    set(ARCH_KERNEL_LDFLAGS "-Wl,-z,max-page-size=65536")
    set(ARCH_CPUFLAGS "-march=${CPU_TYPE} ${ARCH_FPUFLAGS}")

    #*********************************************************************************************************
    # LoongARCH64
    #*********************************************************************************************************
elseif ("${TOOLCHAIN_PREFIX}" STREQUAL "loongarch64-sylixos-elf-")
    set(CMAKE_SYSTEM_PROCESSOR "loongarch64")

    set(ARCH_COMMONFLAGS "-mstrict-align")

    set(ARCH_PIC_ASFLAGS "")
    set(ARCH_PIC_CFLAGS "-fPIC")
    set(ARCH_PIC_LDFLAGS "-Wl,-shared -fPIC -shared -Wl,-z,max-page-size=65536")

    set(ARCH_KO_CFLAGS "-Wa,-mla-global-with-abs,-mla-local-with-abs")
    set(ARCH_KO_LDFLAGS "-nostdlib -r -Wl,-z,max-page-size=65536")

    set(ARCH_KLIB_CFLAGS "")

    set(ARCH_KERNEL_CFLAGS "-Wa,-mla-global-with-pcrel")
    set(ARCH_KERNEL_LDFLAGS "-Wl,-z,max-page-size=65536")
    set(ARCH_CPUFLAGS "-march=${CPU_TYPE} -G 0 -mabi=lp64d ${ARCH_CPUFLAGS}")
else ()
    message(FATAL_ERROR "Unsupported TOOLCHAIN_PREFIX: ${TOOLCHAIN_PREFIX}")
    return()
endif()

message(STATUS "CMAKE_SYSTEM_PROCESSOR is ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "ARCH_COMMONFLAGS is ${ARCH_COMMONFLAGS}")
message(STATUS "ARCH_CPUFLAGS is ${ARCH_CPUFLAGS}")
message(STATUS "ARCH_PIC_CFLAGS is ${ARCH_PIC_CFLAGS}")
message(STATUS "ARCH_PIC_LDFLAGS is ${ARCH_PIC_LDFLAGS}")
message(STATUS "ARCH_KO_CFLAGS is ${ARCH_KO_CFLAGS}")
message(STATUS "ARCH_KO_LDFLAGS is ${ARCH_KO_LDFLAGS}")
message(STATUS "ARCH_KERNEL_CFLAGS is ${ARCH_KERNEL_CFLAGS}")
message(STATUS "ARCH_KERNEL_LDFLAGS is ${ARCH_KERNEL_LDFLAGS}")
message(STATUS "ARCH_KLIB_CFLAGS is ${ARCH_KLIB_CFLAGS}")

#*********************************************************************************************************
# gcc.mk
#*********************************************************************************************************

#*********************************************************************************************************
# specify cross compilers and tools
#*********************************************************************************************************
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
message(STATUS "In gcc.mk, TOOLCHAIN_PREFIX is ${TOOLCHAIN_PREFIX}")
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc CACHE PATH "SylixOS Base Path")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++ CACHE PATH "SylixOS Base Path")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc CACHE PATH "SylixOS Base Path")
set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}g++ CACHE PATH "SylixOS Base Path")
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy CACHE PATH "SylixOS Base Path")
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump CACHE PATH "SylixOS Base Path")
set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar CACHE PATH "SylixOS Base Path")
set(CMAKE_STRIP ${TOOLCHAIN_PREFIX}strip CACHE PATH "SylixOS Base Path")
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size CACHE PATH "SylixOS Base Path")
set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib CACHE PATH "SylixOS Base Path")

message(STATUS "CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER is ${CMAKE_CXX_COMPILER}")
#*********************************************************************************************************
# Compiler optimize flag
#*********************************************************************************************************
set(TOOLCHAIN_OPTIMIZE_DEBUG "-O0 -g3 -gdwarf-2")
set(TOOLCHAIN_OPTIMIZE_RELEASE "-O2 -g3 -gdwarf-2")

#*********************************************************************************************************
# Toolchain flag
#*********************************************************************************************************
set(TOOLCHAIN_CXX_CFLAGS "")
set(TOOLCHAIN_CXX_EXCEPT_CFLAGS "${TOOLCHAIN_CXX_CFLAGS} -fexceptions -frtti")
set(TOOLCHAIN_NO_CXX_EXCEPT_CFLAGS "${TOOLCHAIN_CXX_CFLAGS} -fno-exceptions -fno-rtti")
set(TOOLCHAIN_GCOV_CFLAGS "-fprofile-arcs -ftest-coverage")
set(TOOLCHAIN_OMP_FLAGS "-fopenmp")
set(TOOLCHAIN_COMMONFLAGS "-Wall -fmessage-length=0 -fsigned-char -fno-short-enums")
set(TOOLCHAIN_ASFLAGS "-x assembler-with-cpp")
set(TOOLCHAIN_NO_UNDEF_SYM_FLAGS "@${SYLIXOS_BASE_PATH}/libsylixos/${CMAKE_BUILD_TYPE}/symbol.ld")
set(TOOLCHAIN_AR_FLAGS "-r")
set(TOOLCHAIN_STRIP_FLAGS "")
set(TOOLCHAIN_STRIP_KO_FLAGS "--strip-unneeded")

#*********************************************************************************************************
# Application.mk
#*********************************************************************************************************

#*********************************************************************************************************
# Include paths
#*********************************************************************************************************
set(TARGET_INC_PATH
    ${LOCAL_INC_PATH}
    ${SYLIXOS_BASE_PATH}/libsylixos/SylixOS
    ${SYLIXOS_BASE_PATH}/libsylixos/SylixOS/include
    ${SYLIXOS_BASE_PATH}/libsylixos/SylixOS/include/network
)

#*********************************************************************************************************
# Compiler preprocess
#*********************************************************************************************************
set(TARGET_DSYMBOL "-DSYLIXOS ${LOCAL_DSYMBOL}")

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
set(TARGET_CFLAGS "${LOCAL_CFLAGS}")
set(TARGET_CXXFLAGS "${LOCAL_CXXFLAGS}")
if(MULTI_PLATFORM_BUILD STREQUAL "yes")
    set(TARGET_LINKFLAGS "${SYLIXOS_BASE_PATH}/libsylixos/build/${TARGET_PLATFORM}/${DEBUG_LEVEL};${SYLIXOS_BASE_PATH}/libcextern/build/${TARGET_PLATFORM}/${DEBUG_LEVEL}")
else()
    set(TARGET_LINKFLAGS "${SYLIXOS_BASE_PATH}/libsylixos//${DEBUG_LEVEL};${SYLIXOS_BASE_PATH}/libcextern//${DEBUG_LEVEL}")
endif()

if(LOCAL_USE_CXX STREQUAL yes)
    set(TARGET_LIBRARIES "cextern;vpmpdm;stdc++;dsohandle;m;gcc;fastlock")
else()
    set(TARGET_LIBRARIES "cextern;vpmpdm;m;gcc;fastlock")
endif()

#*********************************************************************************************************
# Depend and compiler parameter (cplusplus in kernel MUST NOT use exceptions and rtti)
#*********************************************************************************************************
if(LOCAL_USE_GCOV STREQUAL yes)
    set(TARGET_GCOV_FLAGS "${TOOLCHAIN_GCOV_CFLAGS}")
else()
    set(TARGET_GCOV_FLAGS "")
endif()

if(LOCAL_USE_OMP STREQUAL yes)
    set(TARGET_OMP_FLAGS "${TOOLCHAIN_OMP_FLAGS}")
else()
    set(TARGET_OMP_FLAGS "")
endif()

if(LOCAL_NO_UNDEF_SYM STREQUAL yes)
    set(TARGET_NO_UNDEF_SYM_FLAGS "${TOOLCHAIN_NO_UNDEF_SYM_FLAGS}")
else()
    set(TARGET_NO_UNDEF_SYM_FLAGS "")
endif()

if(LOCAL_USE_CXX_EXCEPT STREQUAL yes)
    set(TARGET_CXX_EXCEPT ${TOOLCHAIN_CXX_EXCEPT_CFLAGS})
else()
    set(TARGET_CXX_EXCEPT ${TOOLCHAIN_NO_CXX_EXCEPT_CFLAGS})
endif()

set(TARGET_CPUFLAGS "${ARCH_CPUFLAGS}")
set(TARGET_COMMONFLAGS "${TARGET_CPUFLAGS} ${ARCH_COMMONFLAGS} ${TOOLCHAIN_COMMONFLAGS} ${TARGET_GCOV_FLAGS} ${TARGET_OMP_FLAGS}")

set(CMAKE_THREAD_LIBS_INIT "")
set(CMAKE_DL_LIBS "")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")
set(CMAKE_SHARED_LIBRARY_RPATH_ORIGIN_TOKEN "\$ORIGIN")
set(CMAKE_SHARED_LIBRARY_RPATH_LINK_C_FLAG "-Wl,-rpath-link,")
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-soname,")
set(CMAKE_EXE_EXPORTS_C_FLAG "-Wl,--export-dynamic")

set(CMAKE_ASM_FLAGS "${TARGET_COMMONFLAGS} ${ARCH_PIC_ASFLAGS} ${TOOLCHAIN_ASFLAGS} ${TARGET_DSYMBOL}" CACHE INTERNAL "ASM Compiler options")
set(CMAKE_C_FLAGS "${TARGET_COMMONFLAGS} ${ARCH_PIC_CFLAGS} ${TARGET_DSYMBOL} ${TARGET_CFLAGS}" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${TARGET_COMMONFLAGS} ${ARCH_PIC_CFLAGS} ${TARGET_DSYMBOL} ${TARGET_CXX_EXCEPT} ${TARGET_CXXFLAGS}" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_EXE_LINKER_FLAGS "${ARCH_PIC_LDFLAGS} ${TARGET_NO_UNDEF_SYM_FLAGS} " CACHE INTERNAL "Linker options")
set(CMAKE_SHARED_LINKER_FLAGS "${ARCH_PIC_LDFLAGS} " CACHE INTERNAL "Shared library Linker options")
set(CMAKE_MODULE_LINKER_FLAGS "${TARGET_COMMONFLAGS} ${TARGET_DSYMBOL} ${TARGET_CFLAGS}")

message(STATUS "+++++CMAKE_SHARED_LINKER_FLAGS is ${CMAKE_SHARED_LINKER_FLAGS}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_FLAGS_DEBUG "${TOOLCHAIN_OPTIMIZE_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${TOOLCHAIN_OPTIMIZE_RELEASE}")
set(CMAKE_C_FLAGS_MINSIZEREL "")
set(CMAKE_C_FLAGS_RELEWITHDEBINFO "")
set(CMAKE_CXX_FLAGS_DEBUG "${TOOLCHAIN_OPTIMIZE_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${TOOLCHAIN_OPTIMIZE_RELEASE}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "")
set(CMAKE_CXX_FLAGS_RELEWITHDEBINFO "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${TOOLCHAIN_OPTIMIZE_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${TOOLCHAIN_OPTIMIZE_RELEASE}")
set(CMAKE_EXE_LINKER_MINSIZEREL "")
set(CMAKE_EXE_LINKER_RELEWITHDEBINFO "")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${TOOLCHAIN_OPTIMIZE_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${TOOLCHAIN_OPTIMIZE_RELEASE}")
set(CMAKE_SHARED_LINKER_MINSIZEREL "")
set(CMAKE_SHARED_LINKER_RELEWITHDEBINFO "")

set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${TOOLCHAIN_OPTIMIZE_DEBUG}")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${TOOLCHAIN_OPTIMIZE_RELEASE}")
set(CMAKE_MODULE_LINKER_MINSIZEREL "")
set(CMAKE_MODULE_LINKER_RELEWITHDEBINFO "")

message(STATUS "Add include from sylixos.toolchain.cmake '${TARGET_INC_PATH}'")
message(STATUS "Add library from sylixos.toolchain.cmake '${TARGET_LIBRARIES}'")

link_directories(${TARGET_LINKFLAGS})
include_directories(${TARGET_INC_PATH})
link_libraries(${TARGET_LIBRARIES})

#*********************************************************************************************************
# End
#*********************************************************************************************************
