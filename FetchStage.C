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
#include "Memory.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
#include "DecodeStage.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Tools.h"

/**
 *  Names: Maegan Brown + Rachel Jordan
 */


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    F * freg = (F *) pregs[FREG];
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];

    DecodeStage * dstage = (DecodeStage *) stages[DSTAGE];
    ExecuteStage * estage = (ExecuteStage * ) stages[ESTAGE];
    uint64_t f_pc = 0, icode = 0, ifun = 0, valC = 0, valP = 0;
    uint64_t rA = RNONE, rB = RNONE, stat = SAOK;
   
    calculateControlSignals(dreg, ereg, mreg, dstage, estage);
     
    f_pc = selectPC(freg, mreg, wreg);
    Memory * memInstance = Memory::getInstance();
    bool imem_error;
    uint8_t instructionByte = memInstance->getByte(f_pc, imem_error);

    //divide instruction byte into icode + ifun
    icode = Tools::getBits(instructionByte, 4, 7);
    ifun = Tools::getBits(instructionByte, 0, 3);
    bool instr_valid = validIcode(icode);
    if(imem_error)
    {
        icode = INOP;
        ifun = FNONE;
    }
    bool needregid = need_regids(icode);
    bool needvalc = need_valC(icode);
    valP = PCincrement(f_pc, needregid, needvalc);

    //set rA + rB and valC
    if(needregid)
    {
        getRegIds(rA, rB, f_pc, memInstance, imem_error);
    }
    if(needvalc)
    {
        buildValC(valC, f_pc, memInstance, imem_error, needregid);
    }
    uint64_t output = PredictPC(icode, valC, valP);
    stat = f_stat(icode, instr_valid, imem_error);
    freg->getpredPC()->setInput(output);

    setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
    return false;
}

/*
 *  applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
    F * freg = (F *) pregs[FREG];
    
    if (!F_stall)
    {
        freg->getpredPC()->normal();
    }
    if (!D_stall && !D_bubble)
    {
        normalD(pregs);
    }
    else if(D_bubble)
    {
        bubbleD(pregs);
    }
}

/**
 *  causes d registers to bubble
 *  @param: pregs - array of pipeline registers used to get D reg
 */
void FetchStage::bubbleD(PipeReg ** pregs)
{
    D * dreg = (D *) pregs[DREG];
    
    dreg->getstat()->bubble(SAOK);
    dreg->geticode()->bubble(INOP);
    dreg->getifun()->bubble();
    dreg->getrA()->bubble(RNONE);
    dreg->getrB()->bubble(RNONE);
    dreg->getvalC()->bubble();
    dreg->getvalP()->bubble();
}

/**
 *  causes d registers to process normally
 *  @param: pregs - array of pipeline registers used to get D reg
 */
void FetchStage::normalD(PipeReg ** pregs)
{
    D * dreg = (D *) pregs[DREG];   

    dreg->getstat()->normal();
    dreg->geticode()->normal();
    dreg->getifun()->normal();
    dreg->getrA()->normal();
    dreg->getrB()->normal();
    dreg->getvalC()->normal();
    dreg->getvalP()->normal();
}


/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}

/**
 *  predicts what PC will be
 *  @param f_icode - instruction code
 *  @param f_valC - next instruction if we follow the jump/call
 *  @param f_valP - next sequential instructions
 *  @return predicted PC value
 */
uint64_t FetchStage::PredictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
    
    if(f_icode == IJXX || f_icode == ICALL)
    {
        return f_valC;
    }
    else
    {
        return f_valP;
    }
}

/**
 *  increments PC by appropriate num of bytes
 *  @param f_pc - current PC
 *  @param f_needregids - 1 byte for reg ids
 *  @param f_needvalC - 8 bytes for offset/address
 *  @return incremented PC value
 */
uint64_t FetchStage::PCincrement(uint64_t f_pc, bool f_needregids, bool f_needvalC)
{
    uint64_t nextPC = f_pc + 1;
    if(f_needregids)
    {
      nextPC += 1;   
    }
    if(f_needvalC)
    {
        nextPC += 8;
    }

    return nextPC;
}

/**
 *  chooses next PC value
 *  @param freg - F reg pointer used to obtain predPC
 *  @param mreg - M reg pointer used to obtain valA
 *  @param wreg - W reg pointer used to obtain valM
 *  @return next PC value
 */
uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg)
{
    if(mreg->geticode()->getOutput() == IJXX && !(mreg->getCnd()->getOutput()))
    {
        return mreg->getvalA()->getOutput();
    }
    else if(wreg->geticode()->getOutput() == IRET)
    {
        return wreg->getvalM()->getOutput();
    }
    else
    {
        return freg->getpredPC()->getOutput();
    }
}

/**
 *  determines if reg ids are part of instruction
 *  @param: f_icode - icode of instruction in Fetch Stage
 *  @return true if reg ids are part of instruction
 */
bool FetchStage::need_regids(uint64_t f_icode)
{

   return f_icode == IRRMOVQ || f_icode == IOPQ || f_icode == IPUSHQ 
        || f_icode == IPOPQ || f_icode == IIRMOVQ || f_icode == IRMMOVQ 
        || f_icode == IMRMOVQ;
}

/**
 *  determines if instr has an offset/addr
 *  @param f_icode - icode of instruction in Fetch Stage
 *  @return true if instr has an offset/addr
 */
bool FetchStage::need_valC(uint64_t f_icode)
{
    return f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ 
         || f_icode == IJXX || f_icode == ICALL;
}

/**
 *  obtains the register IDs
 *  @param: rA - var that holds reg A value
 *  @param: rB - var that holds reg B value
 *  @param: f_pc - address of current instruction
 *  @param: memInst - instance of Memory class -- used to access getByte
 *  @param: error - true if error
 */
void FetchStage::getRegIds(uint64_t & rA, uint64_t & rB, uint64_t f_pc, Memory * memInst, bool & error)
{
    uint8_t registerByte =  memInst->getByte(f_pc + 1, error);
    rB = Tools::getBits(registerByte, 0, 3);
    rA = Tools::getBits(registerByte, 4, 7);
    
}

/**
 *  builds valC
 *  @param: valC - var that holds the built offset/addr
 *  @param: f_pc - address of current instruction
 *  @param: memInst - instance of Memory class -- used to access getByte
 *  @param: error - true if error
 *  @param: needregid - true if instr includes register ids
 */
void FetchStage::buildValC(uint64_t & valC, uint64_t f_pc, Memory * memInst, bool & error, bool needregid)
{
    uint8_t bytes[8];

    if(needregid)
    {
        for (uint64_t i = f_pc + 2, j = 0; i < (f_pc + 10); i++, j++)
        {
            bytes[j] = memInst->getByte(i, error);
        }

        valC = Tools::buildLong(bytes);
    }
    else 
    {
        for (uint64_t i = f_pc + 1, j = 0; i < (f_pc + 9); i++, j++)
        {
            bytes[j] = memInst->getByte(i, error);
        }

        valC = Tools::buildLong(bytes);
    }
}

/**
 *  determines if icode is valid
 *  @param: f_icode - instruction code in fetch stage
 *  @return true if icode is valid
 */
bool FetchStage::validIcode(uint64_t f_icode)
{
    return f_icode == INOP || f_icode == IHALT || f_icode == IRRMOVQ
        || f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ
        || f_icode == IOPQ || f_icode == IJXX || f_icode == ICALL 
        || f_icode == IRET || f_icode == IPUSHQ || f_icode == IPOPQ;
}

/**
 *  determines f_stat value
 *  @param: f_icode - instruction code in fetch stage
 *  @param: instr_valid - true if instr_valid
 *  @param: mem_error - true if there is an error
 *  @return determined f_stat value
 */
uint64_t FetchStage::f_stat(uint64_t f_icode, bool instr_valid, bool mem_error)
{   
    if(mem_error)
    {
        return SADR;
    }
    else if(!instr_valid)
    {
        return SINS;
    }
    else if(f_icode == IHALT)
    {   
        return SHLT;
    }
    else 
    {
        return SAOK;
    }
}

/**
 *  calculate F_stall
 *  @param: dreg - D register pointer used to obtain D_icode
 *  @param: ereg - E register pointer used to obtain E_icode + E_dstm
 *  @param: mreg - M register pointer used to obtain M_icode
 *  @param: dstage - DecodeStage pointer used to obtain srcA + srcB
 */
void FetchStage::calcF_stall(D * dreg, E * ereg, M * mreg,  DecodeStage * dstage)
{
    uint64_t D_icode = dreg->geticode()->getOutput();
    uint64_t E_icode = ereg->geticode()->getOutput();
    uint64_t M_icode = mreg->geticode()->getOutput();
    uint64_t E_dstM = ereg->getdstM()->getOutput();   
    uint64_t d_srcA = dstage->get_srcA();
    uint64_t d_srcB = dstage->get_srcB();
    
    F_stall = ((E_icode == IMRMOVQ || E_icode == IPOPQ)
               && (E_dstM == d_srcA || E_dstM == d_srcB)) 
               || (IRET == D_icode || IRET == E_icode 
               || IRET == M_icode);
}

/**
 *  calculate D_stall
 *  @param: ereg - E register pointer used to obtain E_icode + E_dstM
 *  @param: dstage - DecodeStage pointer used to obtain srcA + srcB
 */
void FetchStage::calcD_stall(E * ereg, DecodeStage * dstage)
{
    uint64_t E_icode = ereg->geticode()->getOutput();
    uint64_t E_dstM = ereg->getdstM()->getOutput();
    uint64_t d_srcA = dstage->get_srcA();
    uint64_t d_srcB = dstage->get_srcB();

    D_stall = (E_icode == IMRMOVQ || E_icode == IPOPQ)
               && (E_dstM == d_srcA || E_dstM == d_srcB);
}

/**
 *  calculate D_bubble
 *  @param: dreg - D register pointer used to obtain D_icode
 *  @param: ereg - E register pointer used to obtain E_icode + E_dstM
 *  @param: mreg - M register pointer used to obtain M_icode
 *  @param: dstage - DecodeStage pointer used to obtain srcA + srcB
 *  @param: estage - ExecuteStage pointer used to obtain e_Cnd
 */
void FetchStage::calcD_bubble(D * dreg, E * ereg, M * mreg, 
                              DecodeStage * dstage, ExecuteStage * estage)
{
    uint64_t D_icode = dreg->geticode()->getOutput();
    uint64_t E_icode = ereg->geticode()->getOutput();
    uint64_t M_icode = mreg->geticode()->getOutput();
    uint64_t e_Cnd = estage->get_Cnd();
    uint64_t E_dstM = ereg->getdstM()->getOutput();
    uint64_t d_srcA = dstage->get_srcA();
    uint64_t d_srcB = dstage->get_srcB();

    D_bubble = (E_icode == IJXX && !e_Cnd) 
               || (!((E_icode == IMRMOVQ || E_icode == IPOPQ) 
               && (E_dstM == d_srcA || E_dstM == d_srcB))
               && (IRET == D_icode || IRET == E_icode
               || IRET == M_icode));
}


/**
 *  calculate F_stall, D_stall, and D_bubble
 *  @param: dreg - D register pointer needed by calcF_stall + calcD_bubble
 *  @param: ereg - E register pointer needed by all 3 functions called here
 *  @param: mreg - M register pointer needed by calcF_stall + calcD_bubble
 *  @param: dstage - DecodeStage pointer needed by all 3 functions
 *  @param: estage - ExecuteStage pointer needed by calcF_stall + calcD_bubble
 */
void FetchStage::calculateControlSignals(D * dreg, E * ereg, M * mreg,
                                         DecodeStage * dstage, ExecuteStage * estage)
{
    calcF_stall(dreg, ereg, mreg, dstage);
    calcD_stall(ereg, dstage);
    calcD_bubble(dreg, ereg, mreg, dstage, estage);
}

