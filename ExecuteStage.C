#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "Tools.h"
#include "ConditionCodes.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

/**
 *  Names: Maegan Brown + Rachel Jordan
 */

bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{

    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];
    uint64_t stat = ereg->getstat()->getOutput(), icode = ereg->geticode()->getOutput();
    uint64_t valA = ereg->getvalA()->getOutput(), dstM = ereg->getdstM()->getOutput();
    uint64_t ifun = ereg->getifun()->getOutput();
    dstE = ereg->getdstE()->getOutput(); 
    valE = ereg->getvalC()->getOutput();
    
    uint64_t m_stat = mstage->getm_stat();
    uint64_t w_stat = wreg->getstat()->getOutput();
    Cnd = cond(icode, ifun);
    uint64_t alua = aluA(icode, valA, valE);
    uint64_t alub = aluB(icode, ereg);
    uint64_t alufun = aluFun(icode, ereg);
    bool setcc = set_cc(icode, m_stat, w_stat);
    calculateControlSignals(m_stat, w_stat);
    valE = ALU(alufun, alua, alub, setcc);
    
    dstE = dstEcomponent(icode, Cnd, dstE);

    setMInput(mreg, stat, icode, Cnd, valE, valA, dstE, dstM);

    return false;
}

void ExecuteStage::doClockHigh(PipeReg ** pregs)
{

    M * mreg = (M *) pregs[MREG];
    if(M_bubble)
    {
        mreg->getstat()->bubble(SAOK);
        mreg->geticode()->bubble(INOP);
        mreg->getCnd()->bubble();
        mreg->getvalE()->bubble();
        mreg->getvalA()->bubble();
        mreg->getdstE()->bubble(RNONE);
        mreg->getdstM()->bubble(RNONE);
    }
    else
    {
        mreg->getstat()->normal();
        mreg->geticode()->normal();
        mreg->getCnd()->normal();
        mreg->getvalE()->normal();
        mreg->getvalA()->normal();
        mreg->getdstE()->normal();
        mreg->getdstM()->normal();
    }

}

void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t Cnd,
                            uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM)
{
    mreg->getstat()->setInput(stat);
    mreg->geticode()->setInput(icode);
    mreg->getCnd()->setInput(Cnd);
    mreg->getvalE()->setInput(valE);
    mreg->getvalA()->setInput(valA);
    mreg->getdstE()->setInput(dstE);
    mreg->getdstM()->setInput(dstM);
}

/**
 *  ALU A component
 *  @param: E_icode - icode from E register
 *  @param: E_valA - valA from E register
 *  @param: E_valC - valC from E register
 *  @return regA value; return valC val if needed
 */
uint64_t ExecuteStage::aluA(int64_t E_icode, uint64_t E_valA, uint64_t E_valC)
{
    if (E_icode == IRRMOVQ || E_icode == IOPQ)
    {
        return E_valA;
    }
    else if (E_icode == IIRMOVQ || E_icode == IRMMOVQ || E_icode == IMRMOVQ)
    {
        return E_valC;
    }
    else if (E_icode == ICALL || E_icode == IPUSHQ)
    {
        return -8;
    }
    else if (E_icode == IRET || E_icode == IPOPQ)
    {
        return 8;
    }
    else
    {
        return 0;
    }
}

/*
 *  ALU B component
 *  @param: E_icode - icode from E register
 *  @param: ereg - E register pointer neeeded to obtain vars
 *  @return regB value; return 0 if no regB
 */
uint64_t ExecuteStage::aluB(uint64_t E_icode, E * ereg)
{
    if (E_icode == IRMMOVQ || E_icode == IMRMOVQ || E_icode == IOPQ
        || E_icode == ICALL || E_icode == IPUSHQ || E_icode == IRET
        || E_icode == IPOPQ)
    {
        return ereg->getvalB()->getOutput();
    }
    else if (E_icode == IRRMOVQ || E_icode == IIRMOVQ)
    {
        return 0;
    }
    else
    {
        return 0;
    }
}

/*
 *  ALU fun. component
 *  @param: E_icode - icode from E register
 *  @param: ereg - E register pointer needed to obtain vars
 *  @return opq function code
 */
uint64_t ExecuteStage::aluFun(uint64_t E_icode, E * ereg)
{
    if( E_icode == IOPQ)
    {
        return ereg->getifun()->getOutput();
    }
    else
    {
        return ADDQ;
    }
}

/*
 *  set_cc component
 *  @param: E_icode - instruction code in E register
 *  @param: m_stat - stat val from MemoryStage
 *  @param: w_stat - stat val from WritebackStage
 *  @return true if condition codes need to be set
 */
bool ExecuteStage::set_cc(uint64_t E_icode, uint64_t m_stat, uint64_t w_stat)
{
   return (E_icode == IOPQ) && (m_stat != SADR) && (m_stat != SINS) && (m_stat != SHLT)
           && (w_stat != SADR) && (w_stat != SINS) && (w_stat != SHLT);
}

/*
 *  dstE component
 *  @param: E_icode - instruction code in E register
 *  @param: e_Cnd - Cnd val from ExecuteStage
 *  @param: E_dstE - dstE val from E register
 *  @return value calculated for dstE
 */
uint64_t ExecuteStage::dstEcomponent(uint64_t E_icode, uint64_t e_Cnd, uint64_t E_dstE)
{
    if(E_icode == IRRMOVQ && !e_Cnd)
    {
        return RNONE;
    }
    else
    {
        return E_dstE;
    }
}

/*
 *  If the set_cc component returns true then the CC component
 *  will be used to set the condition codes 
 *  @param: s - sign flag val
 *  @param: z - zero flag val
 *  @param: o - overflow flag val
 */
void ExecuteStage::CC(bool s, bool z, bool o)
{
    bool error;
    ConditionCodes * cc = ConditionCodes::getInstance();
    cc->setConditionCode(s, SF, error);
    cc->setConditionCode(z, ZF, error);
    cc->setConditionCode(o, OF, error);
}

/**
 *  performs ALU logic
 *  @param: alufun - opq instruction function number
 *  @param: aluA - regA value
 *  @param: aluB - regB value
 *  @param: setcc - true if condition codes need to be set
 *  @return output of operation
 */
uint64_t ExecuteStage::ALU(uint64_t alufun, uint64_t aluA, uint64_t aluB, bool setcc)
{
    uint64_t output;
    bool sign;
    bool zero;
    bool overflow = 0;

    switch(alufun)
    {
        case ADDQ:
            output = aluA + aluB;
            overflow = Tools::addOverflow(aluA, aluB);
            break;
        case SUBQ:
            output = aluB - aluA;
            overflow = Tools::subOverflow(aluA, aluB);
            break;
        case ANDQ:
            output = aluA & aluB;
            break;
        case XORQ:
            output = aluA ^ aluB;
            break;
        default: return 0;
    }

    sign = Tools::sign(output);
    zero = (output == 0);

    if (setcc)
    {
        CC(sign, zero, overflow);
    }

    return output;
}

/**
 *  getter for dstE
 *  @return dstE
 */
uint64_t ExecuteStage::gete_dstE()
{
    return dstE;
}

/**
 *  getter for valE
 *  @return valE
 */
uint64_t ExecuteStage::gete_valE()
{
    return valE;
}

/**
 *  getter for Cnd
 *  @return Cnd
 */
uint64_t ExecuteStage::get_Cnd()
{
    return Cnd;
}

/**
 *  obtains the value of e_Cnd
 *
 *  @param icode for identifying IJXX and/or ICMOVXX instructions
 *  @param ifun for differentiating b/w different functions
 *  @return the value of e_Cnd
 */
uint64_t ExecuteStage::cond(uint64_t icode, uint64_t ifun)
{
    ConditionCodes * cc = ConditionCodes::getInstance();
    bool error;
    bool sf = cc->getConditionCode(SF, error);
    bool of = cc->getConditionCode(OF, error);
    bool zf = cc->getConditionCode(ZF, error);
    if(icode != IJXX && icode != ICMOVXX)
    {
        return 0;
    }
    else
    {
        switch(ifun)
        {
            case UNCOND:
                return 1;
                break;
            case LESSEQ:
                return (sf ^ of) | zf;
                break;
            case LESS:
                return (sf ^ of);
                break;
            case EQUAL:
                return zf;
                break;
            case NOTEQUAL:
                return !zf;
                break;
            case GREATEREQ:
                return !(sf ^ of);
                break;
            case GREATER:
                return !(sf ^ of) & !zf;
                break;
            default: 
                return 0;
        }
    }
}

/**
 *  sets the value of M_bubble
 *  @param: m_stat - stat value from memory stage
 *  @param: w_stat - stat value from writeback stage
 */
void ExecuteStage::calculateControlSignals(uint64_t m_stat, uint64_t w_stat)
{
    M_bubble = (m_stat == SADR) || (m_stat == SINS) || (m_stat == SHLT)
            || (w_stat == SADR) || (w_stat == SINS) || (w_stat == SHLT);
}



