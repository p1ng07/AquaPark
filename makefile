COMMON_SRC_DIR := ./src/common

MONIT_SRC_DIR := ./src/monitor
SIMUL_SRC_DIR := ./src/simulador

OBJ_DIR_COMMON := ./obj
OBJ_DIR_SIMUL := ./obj/simul
OBJ_DIR_MONIT := ./obj/monit

# ficheiros a compilar dos diferentes targets
COMMON_SRC := $(wildcard ${COMMON_SRC_DIR}/*.c)
MONIT_SRC := $(wildcard ${MONIT_SRC_DIR}/*.c)
SIMUL_SRC := $(wildcard ${SIMUL_SRC_DIR}/*.c)

OBJ_COMMON := $(COMMON_SRC:$(COMMON_SRC_DIR)/../%.c=$(OBJ_DIR_COMMON)/%.o)
OBJ_MONIT := $(MONIT_SRC:$(MONIT_SRC_DIR)/../%.c=$(OBJ_DIR_MONIT)/%.o)
OBJ_SIMUL := $(SIMUL_SRC:$(SIMUL_SRC_DIR)/../%.c=$(OBJ_DIR_SIMUL)/%.o)

all: monitor simulador

monitor: $(OBJ_MONIT) $(OBJ_COMMON)
	cc -g $^ -o $@

simulador: $(OBJ_COMMON)  $(OBJ_SIMUL)
	cc -g $^ -o $@

$(OBJ_DIR_COMMON)/%.o: $(COMMON_SRC)/%.c | $(OBJ_DIR_COMMON) 
	cc -g -c $< -o $@

$(OBJ_DIR_MONIT)/%.o: $(MONIT_SRC)/%.c | $(OBJ_DIR_MONIT) 
	cc -g -c $< -o $@

$(OBJ_DIR_SIMUL)/%.o: $(SIMUL_SRC)/%.c | $(OBJ_DIR_SIMUL) 
	cc -g -c $< -o $@
