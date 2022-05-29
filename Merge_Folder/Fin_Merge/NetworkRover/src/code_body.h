#ifndef code_body_h
#define code_body_h

class fclass
{
    public:
        fclass();
        void test_code();
        String HTTPGET();
        //void HTTPPOST();
        void ARM();
        void DISARM();
        void RCW(       int DutyCycle);
        void RCCW(      int DutyCycle);
        void LCW(       int DutyCycle);
        void LCCW(      int DutyCycle);
        void RSTOP();
        void LSTOP();
        void Forward(   int Duration,int DutyCycle);
        void Backward(  int Duration,int DutyCycle);
        void SpinCW(    int Duration,int DutyCycle);
        void SpinCCW(   int Duration,int DutyCycle);
        void ForwardL(  int Duration,int DutyCycle);
        void ForwardR(  int Duration,int DutyCycle);
        void BackwardL( int Duration,int DutyCycle);
        void BackwardR( int Duration,int DutyCycle);
};

extern fclass code_body;

#endif