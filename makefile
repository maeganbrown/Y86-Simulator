CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o Memory.o Tools.o RegisterFile.o ConditionCodes.o Loader.o\
F.o D.o E.o M.o W.o FetchStage.o DecodeStage.o ExecuteStage.o MemoryStage.o\
WritebackStage.o PipeReg.o PipeRegField.o Simulate.o
STAGESR = F.h D.h M.h W.h
PIPE =  PipeRegField.o PipeReg.h
STAGES = $(PIPE) $(STAGESR) RegisterFile.h Status.h Debug.h Stage.h\
Instructions.h
INST = Instructions.h RegisterFile.h Status.h
.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Memory.h RegisterFile.h ConditionCodes.h Loader.h Debug.h\
PipeReg.h Stage.h Simulate.h

Tools.o: Tools.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h

ConditionCodes.o: ConditionCodes.h Tools.h

Loader.o: Loader.h Memory.h

F.o: F.h $(INST) $(PIPE)

D.o: D.h $(INST) $(PIPE)

E.o: E.h $(INST) $(PIPE)

M.o: M.h $(INST) $(PIPE)

W.o: W.h $(INST) $(PIPE)

FetchStage.o: FetchStage.h $(STAGES) E.h Tools.h Memory.h DecodeStage.h\
ExecuteStage.h MemoryStage.h

DecodeStage.o: DecodeStage.h $(STAGES) E.h ExecuteStage.h MemoryStage.h

ExecuteStage.o: ExecuteStage.h $(STAGES) E.h Tools.h ConditionCodes.h\
MemoryStage.h

MemoryStage.o: MemoryStage.h $(STAGES) Memory.h

WritebackStage.o: WritebackStage.h $(STAGES)

PipeReg.o: PipeReg.h

PipeRegField.o: PipeRegField.h

Simulate.o: $(PIPE) $(STAGESR) E.h Stage.h ExecuteStage.h Memory.h\
DecodeStage.h FetchStage.h WritebackStage.h Simulate.h Memory.h\
RegisterFile.h ConditionCodes.h

clean:
	rm $(OBJ) yess	

run:
	make clean
	make yess
	./run.sh

