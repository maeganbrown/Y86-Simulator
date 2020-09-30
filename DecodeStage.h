//class to perform the combinational logic of
//the Decode stage
class DecodeStage: public Stage
{
    private:
        uint64_t srcA;
        uint64_t srcB;
        bool E_bubble;
        void bubbleE(PipeReg ** pregs);
        void normalE(PipeReg ** pregs);
        void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun,
                        uint64_t valC, uint64_t valA, uint64_t valB, 
                        uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB);
        uint64_t d_srcA(uint64_t D_icode, D * dreg);
        uint64_t d_srcB(uint64_t D_icode, D * dreg);
        uint64_t d_dstE(uint64_t D_icode, D * dreg);
        uint64_t d_dstM(uint64_t D_icode, D * dreg);
        uint64_t d_valA(uint64_t d_srcA, RegisterFile * regInst, ExecuteStage * estage,
                        MemoryStage * mstage, M * mreg, W * wreg, D * dreg);
        uint64_t d_valB(uint64_t d_srcB, RegisterFile * regInst, ExecuteStage * estage,
                        MemoryStage * mstage, M * mreg, W * wreg);
        void calculateControlSignals(E * ereg, ExecuteStage * estage);
    public:
        bool doClockLow(PipeReg ** pregs, Stage ** stages);
        void doClockHigh(PipeReg ** pregs);
        uint64_t get_srcA();
        uint64_t get_srcB();
};
