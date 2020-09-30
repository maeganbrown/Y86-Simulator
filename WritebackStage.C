#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "WritebackStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

/**
 *  Names: Maegan Brown + Rachel Jordan
 */

bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    W * wreg = (W *) pregs[WREG];
    uint64_t stat = wreg->getstat()->getOutput();
    return stat != SAOK;
}

void WritebackStage::doClockHigh(PipeReg ** pregs)
{
    RegisterFile * regInst = RegisterFile::getInstance();
    W * wreg = (W *) pregs[WREG];
    bool error;
    
    uint64_t W_valE = wreg->getvalE()->getOutput();
    uint64_t W_dstE = wreg->getdstE()->getOutput();
    regInst->writeRegister(W_valE, W_dstE, error);
    
    uint64_t W_valM = wreg->getvalM()->getOutput();
    uint64_t W_dstM = wreg->getdstM()->getOutput();
    regInst->writeRegister(W_valM, W_dstM, error);
}


