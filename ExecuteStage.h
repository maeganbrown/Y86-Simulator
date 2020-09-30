//class to perform the combinational logic of
//the Execute stage
class ExecuteStage: public Stage
{
    private:
        uint64_t dstE;
        uint64_t valE;
        uint64_t Cnd;
        bool M_bubble;
        void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t Cnd,
                        uint64_t getvalE, uint64_t getvalA, uint64_t getdstE,
                        uint64_t getdstM);
        uint64_t aluA(int64_t E_icode, uint64_t E_valA, uint64_t E_valC);
        uint64_t aluB(uint64_t E_icode, E * ereg);
        uint64_t aluFun(uint64_t E_icode, E * ereg);
        bool set_cc(uint64_t E_icode, uint64_t m_stat, uint64_t w_stat);
        uint64_t dstEcomponent(uint64_t E_icode, uint64_t e_Cnd, uint64_t E_dstE);
        void CC(bool s, bool z, bool o);
        uint64_t ALU(uint64_t alufun, uint64_t aluA, uint64_t aluB, bool setcc);
        uint64_t cond(uint64_t icode, uint64_t ifun);
        void  calculateControlSignals(uint64_t m_stat, uint64_t w_stat);
    public:
        bool doClockLow(PipeReg ** pregs, Stage ** stages);
        void doClockHigh(PipeReg ** pregs);
        uint64_t gete_dstE();
        uint64_t gete_valE();
        uint64_t get_Cnd();
};
