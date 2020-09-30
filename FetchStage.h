//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      bool F_stall;
      bool D_stall;
      bool D_bubble;
      void bubbleD(PipeReg ** pregs);
      void normalD(PipeReg ** pregs);
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      uint64_t PredictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
      uint64_t PCincrement(uint64_t f_pc, bool f_needregids, bool needvalC);
      uint64_t selectPC(F * freg, M * mreg, W * wreg);
      bool need_regids(uint64_t f_icode);
      bool need_valC(uint64_t f_icode);
      void getRegIds(uint64_t & rA, uint64_t & rB, uint64_t f_pc, Memory * memInst, bool & error);
      void buildValC(uint64_t & valC, uint64_t f_pc, Memory * memInst, bool & error, bool needregid);
      bool validIcode(uint64_t f_icode);
      uint64_t f_stat(uint64_t f_icode, bool instr_valid, bool mem_error);
      void calcF_stall(D * dreg, E * ereg, M * mreg, DecodeStage * dstage);
      void calcD_stall(E * ereg, DecodeStage * dstage);
      void calcD_bubble(D * dreg, E * ereg, M * mreg,
                        DecodeStage * dstage, ExecuteStage * estage);
      void calculateControlSignals(D * dreg, E * ereg, M * mreg,
                                   DecodeStage * dstage, ExecuteStage * estage);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
