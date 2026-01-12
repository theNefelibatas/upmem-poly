DPU_DIR := dpu
HOST_DIR := host
BUILD_DIR ?= build

NR_TASKLETS ?= 4
NR_DPUS ?= 1

HOST_TARGET := ${BUILD_DIR}/host
DPU_TARGET := ${BUILD_DIR}/dpu

.PHONY: all clean test

INCLUDES := include
HOST_SOURCES := $(wildcard ${HOST_DIR}/*.cpp)
DPU_SOURCES := $(wildcard ${DPU_DIR}/*.c)

__dirs := $(shell mkdir -p ${BUILD_DIR})

COMMON_FLAGS := -Wall -Wextra -Werror -g -I${INCLUDES}
HOST_FLAGS := ${COMMON_FLAGS} -std=c++17 -O3 `dpu-pkg-config --cflags --libs dpu` -DNR_TASKLETS=${NR_TASKLETS} -DNR_DPUS=${NR_DPUS} -DDPU_TARGET=${DPU_TARGET}
DPU_FLAGS := ${COMMON_FLAGS} -O2 -DNR_TASKLETS=${NR_TASKLETS} -DNR_DPUS=${NR_DPUS}

all: ${HOST_TARGET} ${DPU_TARGET}

${HOST_TARGET}: ${HOST_SOURCES} ${INCLUDES}
	clang++ -o $@ ${HOST_SOURCES} ${HOST_FLAGS}

${DPU_TARGET}: ${DPU_SOURCES} ${INCLUDES}
	dpu-upmem-dpurte-clang ${DPU_FLAGS} -o $@ ${DPU_SOURCES}

clean:
	$(RM) -r $(BUILD_DIR)

test: ${HOST_TARGET} ${DPU_TARGET}
	./${HOST_TARGET}
