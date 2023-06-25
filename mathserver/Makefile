CURRENT_DIR = $(shell pwd)
SRC = src
OBJ = objects
INCLUDES = include
C_COMPILER = gcc
C_COMPILERFLAGS = -c 
LDFLAGS = -std=gnu99 -I $(CURRENT_DIR)/$(INCLUDES) -Wall -g -lpthread -lm
all: tools.o matinv_par.o kmeans_par.o strategies.o client.o server.o
	$(C_COMPILER) $(INCLUDES)/configs.h $(OBJ)/tools.o $(OBJ)/matinv_par.o -o $(CURRENT_DIR)/matinvpar $(LDFLAGS)
	$(C_COMPILER) $(INCLUDES)/configs.h $(OBJ)/tools.o $(OBJ)/kmeans_par.o -o $(CURRENT_DIR)/kmeanspar $(LDFLAGS)
	$(C_COMPILER) $(INCLUDES)/configs.h $(OBJ)/tools.o $(OBJ)/client.o -o $(CURRENT_DIR)/client $(LDFLAGS)
	$(C_COMPILER) $(INCLUDES)/configs.h $(OBJ)/tools.o $(OBJ)/strategies.o $(OBJ)/server.o -o $(CURRENT_DIR)/server $(LDFLAGS)
	
tools.o: $(CURRENT_DIR)/$(SRC)/tools.c
	$(C_COMPILER) $(C_COMPILERFLAGS) $(CURRENT_DIR)/$(SRC)/tools.c -o $(CURRENT_DIR)/$(OBJ)/tools.o $(LDFLAGS)
matinv_par.o: $(CURRENT_DIR)/$(SRC)/matinv_par.c
	$(C_COMPILER) $(C_COMPILERFLAGS) $(CURRENT_DIR)/$(SRC)/matinv_par.c -o $(CURRENT_DIR)/$(OBJ)/matinv_par.o $(LDFLAGS)
kmeans_par.o: $(CURRENT_DIR)/$(SRC)/kmeans_par.c
	$(C_COMPILER) $(C_COMPILERFLAGS) $(CURRENT_DIR)/$(SRC)/kmeans_par.c -o $(CURRENT_DIR)/$(OBJ)/kmeans_par.o $(LDFLAGS)
strategies.o: $(CURRENT_DIR)/$(SRC)/strategies.c
	$(C_COMPILER) $(C_COMPILERFLAGS) $(CURRENT_DIR)/$(SRC)/strategies.c -o $(CURRENT_DIR)/$(OBJ)/strategies.o $(LDFLAGS)

client.o: $(CURRENT_DIR)/$(SRC)/client.c
	$(C_COMPILER) $(C_COMPILERFLAGS) $(CURRENT_DIR)/$(SRC)/client.c -o $(CURRENT_DIR)/$(OBJ)/client.o $(LDFLAGS)
server.o: $(CURRENT_DIR)/$(SRC)/server.c
	$(C_COMPILER) $(C_COMPILERFLAGS) $(CURRENT_DIR)/$(SRC)/server.c -o $(CURRENT_DIR)/$(OBJ)/server.o $(LDFLAGS)

clean:
	rm -rf $(CURRENT_DIR)/client 
	rm -rf $(CURRENT_DIR)/server
	rm -rf $(CURRENT_DIR)/matinvpar
	rm -rf $(CURRENT_DIR)/kmeanspar
	rm -rf $(CURRENT_DIR)/$(OBJ)/*.o
	rm -rf *soln*.txt
	rm -rf $(CURRENT_DIR)/../computed_results/*soln*.txt