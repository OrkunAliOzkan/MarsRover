#ifndef code_body_h
#define code_body_h

class fclass
{
    public:
        fclass();
        void test_code();
        String HTTPGET();
        //void HTTPPOST();
        //////////////////////////////////////////////////////////////////////
        void ARM();
        void DISARM();
        void RCW(       int DutyCycle);
        void RCCW(      int DutyCycle);
        void LCW(       int DutyCycle);
        void LCCW(      int DutyCycle);
        void RSTOP();
        void LSTOP();
        void Forward(   int Duration, int DutyCycleLeft, int DutyCycleRight);
        void Backward(  int Duration, int DutyCycleLeft, int DutyCycleRight);
        void SpinCW(    int Duration, int DutyCycleLeft, int DutyCycleRight);
        void SpinCCW(   int Duration, int DutyCycleLeft, int DutyCycleRight);
        void ForwardL(  int Duration, int DutyCycleLeft, int DutyCycleRight);
        void ForwardR(  int Duration, int DutyCycleLeft, int DutyCycleRight);
        void BackwardL( int Duration, int DutyCycleLeft, int DutyCycleRight);
        void BackwardR( int Duration, int DutyCycleLeft, int DutyCycleRight);
        //////////////////////////////////////////////////////////////////////
        int convTwosComp(int b);
        void mousecam_reset();
        int mousecam_init();
        void mousecam_write_reg(int reg, int val);
        int mousecam_read_reg(int reg);
        void mousecam_read_motion(struct MD *p);
        int mousecam_frame_capture(byte *pdata);
        char asciiart(int k);
        //////////////////////////////////////////////////////////////////////
};

extern fclass code_body;

#endif