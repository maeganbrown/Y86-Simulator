#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "ExecuteStage.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
/**
 *  Names: Maegan Brown + Rachel Jordan
 */

bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    ExecuteStage * estage = (ExecuteStage *) stages[ESTAGE];  
    MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];

    uint64_t valA = 0, valB = 0, dstE = RNONE, dstM = RNONE;
    uint64_t stat = dreg->getstat()->getOutput(), icode = dreg->geticode()->getOutput();
    uint64_t ifun = dreg->getifun()->getOutput(), valC = dreg->getvalC()->getOutput();
    
    srcA = d_srcA(icode, dreg);
    srcB = d_srcB(icode, dreg);
    dstE = d_dstE(icode, dreg);
    dstM = d_dstM(icode, dreg);

    calculateControlSignals(ereg, estage);

    RegisterFile * regInst = RegisterFile::getInstance();

    valA = d_valA(srcA, regInst, estage, mstage, mreg, wreg, dreg);
    valB = d_valB(srcB, regInst, estage, mstage, mreg, wreg);
    
    setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);
        
    return false;
}

void DecodeStage::doClockHigh(PipeReg ** pregs)
{
    if (E_bubble)
    {
        bubbleE(pregs);   
    }
    else
    {
        normalE(pregs);
    }
}

/**
 *  executes execute stage registers with a bubble
 *  @param: pregs - PipeReg pointer used to obtain ereg pointer
 */
void DecodeStage::bubbleE(PipeReg ** pregs)
{
    E * ereg = (E *) pregs[EREG];

    ereg->getstat()->bubble(SAOK);
    ereg->geticode()->bubble(INOP);
    ereg->getifun()->bubble();
    ereg->getvalC()->bubble();
    ereg->getvalA()->bubble();
    ereg->getvalB()->bubble();
    ereg->getdstE()->bubble(RNONE);
    ereg->getdstM()->bubble(RNONE);
    ereg->getsrcA()->bubble(RNONE);
    ereg->getsrcB()->bubble(RNONE);
}

/**
 *  executes execute stage registers normally
 *  @param: pregs - PipeReg pointer used to obtain ereg pointer
 */
void DecodeStage::normalE(PipeReg ** pregs)
{
    E * ereg = (E *) pregs[EREG];

    ereg->getstat()->normal();
    ereg->geticode()->normal();
    ereg->getifun()->normal();
    ereg->getvalC()->normal();
    ereg->getvalA()->normal();
    ereg->getvalB()->normal();
    ereg->getdstE()->normal();
    ereg->getdstM()->normal();
    ereg->getsrcA()->normal();
    ereg->getsrcB()->normal();
}

void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun,
                            uint64_t valC, uint64_t valA, uint64_t valB,
                            uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
    ereg->getstat()->setInput(stat);
    ereg->geticode()->setInput(icode);
    ereg->getifun()->setInput(ifun);
    ereg->getvalC()->setInput(valC);
    ereg->getvalA()->setInput(valA);
    ereg->getvalB()->setInput(valB);
    ereg->getdstE()->setInput(dstE);
    ereg->getdstM()->setInput(dstM);
    ereg->getsrcA()->setInput(srcA);
    ereg->getsrcB()->setInput(srcB);
}

/**
 *  calculates d_srcB
 *  @param: D_icode - instruction code in D register
 *  @param: dreg - D register pointer needed to obtain vars
 *  @return calculated value for d_srcB
 */
uint64_t DecodeStage::d_srcA(uint64_t D_icode, D * dreg)
{
    if (D_icode == IRRMOVQ || D_icode == IRMMOVQ || D_icode == IOPQ
         || D_icode == IPUSHQ) 
    {
        return dreg->getrA()->getOutput();
    }
    else if (D_icode == IPOPQ || D_icode == IRET)
    {
        return RSP;
    }
    else
    {
        return RNONE;
    }
}

/**
 *  calculates d_srcB
 *  @param: D_icode - instruction code in D register
 *  @param: dreg - D register pointer needed to obtain vars
 *  @return calculated value for d_srcB
 */
uint64_t DecodeStage::d_srcB(uint64_t D_icode, D * dreg)
{
    if (D_icode == IOPQ || D_icode == IRMMOVQ || D_icode == IMRMOVQ)
    {
        return dreg->getrB()->getOutput();
    }
    else if (D_icode == IPUSHQ || D_icode == IPOPQ || D_icode == ICALL
            || D_icode == IRET)
    {
        return RSP;
    }
    else
    {
        return RNONE;
    }
}

/**
 *  calculates d_dstE
 *  @param: D_icode - instruction code in D register
 *  @param: dreg - D register pointer needed to obtain vars
 *  @return calculated value for d_dstE
 */
uint64_t DecodeStage::d_dstE(uint64_t D_icode, D * dreg)
{
    if (D_icode == IRRMOVQ || D_icode == IIRMOVQ || D_icode == IOPQ)
    {
        return dreg->getrB()->getOutput();
    }
    else if (D_icode == IPUSHQ || D_icode == IPOPQ || D_icode == ICALL
            || D_icode == IRET)
    {
        return RSP;
    }
    else
    {
        return RNONE;
    }
}

/**
 *  calculates d_dstM
 *  @param: D_icode - instruction code in D register
 *  @param: dreg - D register pointer needed to obtain vars
 *  @return calculated value for d_dstM
 */
uint64_t DecodeStage::d_dstM(uint64_t D_icode, D * dreg)
{
    if (D_icode == IMRMOVQ || D_icode == IPOPQ)
    {
        return dreg->getrA()->getOutput();
    }
    else
    {
        return RNONE;
    }
}

/**
 *  calculates d_valA
 *  @param: d_srcA - value of registerA
 *  @param: regInst - RegisterFile instance needed for readRegister func
 *  @param: estage - ExecuteStage pointer needed for var access
 *  @param: mstage - MemoryStage pointer needed for var access
 *  @param: mreg - M register access needed for var access
 *  @param: wreg - W register access needed for var access
 *  @param: dreg - D register access needed for var access
 *  @return value calculated for d_valA
 */
uint64_t DecodeStage::d_valA(uint64_t d_srcA, RegisterFile * regInst, 
                             ExecuteStage * estage, MemoryStage * mstage,
                             M * mreg, W * wreg, D * dreg)
{
    bool error;
    uint64_t D_icode = dreg->geticode()->getOutput();
    if (D_icode == ICALL || D_icode == IJXX)
    {
        return dreg->getvalP()->getOutput();
    }   
    else if(d_srcA == RNONE)
    {
        return 0;
    }
    else if(d_srcA == estage->gete_dstE())
    {
        return estage->gete_valE();
    }
    else if(d_srcA == mreg->getdstM()->getOutput())
    {
        return mstage->get_valM();
    }
    else if(d_srcA == mreg->getdstE()->getOutput())
    {
        return mreg->getvalE()->getOutput();
    }
    else if(d_srcA == wreg->getdstM()->getOutput())
    {
        return wreg->getvalM()->getOutput();
    }
    else if(d_srcA == wreg->getdstE()->getOutput())
    {
        return wreg->getvalE()->getOutput();
    }
    else
    {
        return regInst->readRegister(d_srcA, error);
    }
}

/**
 *  calculates d_valB
 *  @param: d_srcB - value of registerB
 *  @param: regInst - RegisterFile instance needed for readRegister function
 *  @param: estage - ExecuteStage pointer needed for var access
 *  @param: mstage - MemoryStage pointer needed for var access
 *  @param: mreg - M register pointer needed for var access
 *  @param: wreg - W register pointer needed for var access
 *  @return calculated value for d_valB
 */
uint64_t DecodeStage::d_valB(uint64_t d_srcB, RegisterFile * regInst,
                             ExecuteStage * estage, MemoryStage * mstage,
                             M * mreg, W * wreg)
{
    bool error;
    if(d_srcB == RNONE)
    {
        return 0;
    }
    else if(d_srcB == estage->gete_dstE())
    {
        return estage->gete_valE();
    }
    else if(d_srcB == mreg->getdstM()->getOutput())
    {
        return mstage->get_valM();
    }
    else if(d_srcB == mreg->getdstE()->getOutput())
    {
         return mreg->getvalE()->getOutput();
    }
    else if(d_srcB == wreg->getdstM()->getOutput())
    {
        return wreg->getvalM()->getOutput();
    }
    else if(d_srcB == wreg->getdstE()->getOutput())
    {
        return  wreg->getvalE()->getOutput();
    }
    else
    {
         return regInst->readRegister(d_srcB, error);
    }
}

/**
 *  getter for srcA
 *  @return srcA
 */
uint64_t DecodeStage::get_srcA()
{
    return srcA;
}

/**
 *  getter for srcB
 *  @return srcB
 */
uint64_t DecodeStage::get_srcB()
{
    return srcB;
}

/**
 *  calculates E_bubble
 *  @param: ereg - E register pointer needed to obtain vars
 *  @param: estage - ExecuteStage pointer needed to obtain vars
 */
void DecodeStage::calculateControlSignals(E * ereg, ExecuteStage * estage)
{
    uint64_t E_icode = ereg->geticode()->getOutput();
    uint64_t E_dstM = ereg->getdstM()->getOutput();
    uint64_t e_Cnd = estage->get_Cnd();
    E_bubble = (E_icode == IJXX && !e_Cnd)  
               || ((E_icode == IMRMOVQ || E_icode == IPOPQ)
               && (E_dstM == srcA || E_dstM == srcB));
}

