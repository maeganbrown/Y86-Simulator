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
#include "Instructions.h"
#include "Memory.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"

/**
 *  Names: Maegan Brown + Rachel Jordan
 */

bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];

    valM = 0;
    stat = mreg->getstat()->getOutput();
    uint64_t icode = mreg->geticode()->getOutput(), valE = mreg->getvalE()->getOutput();
    uint64_t dstE = mreg->getdstE()->getOutput(), dstM = mreg->getdstM()->getOutput();

    //obtain address that is used to access memory by using Addr method
    uint64_t mem_address = mem_addr(icode, valE, mreg);

    Memory * memInst = Memory::getInstance();
    bool error;
    //if (mem_read()), then use Memory class to read a long
    if (mem_read(icode))
    {
        valM = memInst->getLong(mem_address, error);
    }
    //if (mem_write()), then use Memory class to write a long
    else if (mem_write(icode))
    {
        memInst->putLong(mreg->getvalA()->getOutput(), mem_address, error);
    }
    
    if(error)
    {
        stat = SADR;
    }

    setWInput(wreg, stat, icode, valE, valM, dstE, dstM);

    return false;
}

void MemoryStage::doClockHigh(PipeReg ** pregs)
{
    W * wreg = (W *) pregs[WREG];

    wreg->getstat()->normal();
    wreg->geticode()->normal();
    wreg->getvalE()->normal();
    wreg->getvalM()->normal();
    wreg->getdstE()->normal();
    wreg->getdstM()->normal();
}

void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE,
                            uint64_t valM, uint64_t dstE, uint64_t dstM)
{
    wreg->getstat()->setInput(stat);
    wreg->geticode()->setInput(icode);
    wreg->getvalE()->setInput(valE);
    wreg->getvalM()->setInput(valM);
    wreg->getdstE()->setInput(dstE);
    wreg->getdstM()->setInput(dstM);
}

/**
 *  Addr component
 *
 *  @param M_icode
 *  @param M_valE
 *  @param mreg is used to obtain M_valA
 *  @return the memory address needed by instruction
 */
uint64_t MemoryStage::mem_addr(uint64_t M_icode, uint64_t M_valE, M * mreg)
{
    if (M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL || 
        M_icode == IMRMOVQ)
    {
        return M_valE;
    }
    else if (M_icode == IPOPQ || M_icode == IRET)
    {
        return mreg->getvalA()->getOutput();
    }
    else
    {
        return 0;
    }
}


/**
 *  Mem. Read
 *  Memory needs to be read from if instruction is a:
 *  - mrmovq
 *  - popq
 *  - ret
 *
 *  @param M_icode
 *  @return true if we need to read from memory
 */
bool MemoryStage::mem_read(uint64_t M_icode)
{
    return (M_icode == IMRMOVQ || M_icode == IPOPQ || M_icode == IRET);
}

/**
 *  Mem. Write
 *  Memory needs to be written to if instruction is a:
 *  - rmmovq
 *  - pushq
 *  - call
 *
 *  @param M_icode
 *  @return true if we need to write to memory
 */
bool MemoryStage::mem_write(uint64_t M_icode)
{
    return (M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL);
}

/**
 *  getter for valM
 *  @return valM
 */
uint64_t MemoryStage::get_valM()
{
    return valM;
}

/**
 *  getter for stat 
 *  @return stat
 */
uint64_t MemoryStage::getm_stat()
{
    return stat;
}


