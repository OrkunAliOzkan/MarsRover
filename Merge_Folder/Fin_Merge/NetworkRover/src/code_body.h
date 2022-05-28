#ifndef code_body_h
#define code_body_h

class fclass
{
    public:
        fclass();
        void test_code();
        String HTTPGET();
        //void HTTPPOST();
        void arm();
        void disarm();
        void RightCW(int Speed);
        void RightCCW(int Speed);
        void LeftCW(int Speed);
        void LeftCCW(int Speed);
        void RightStop();
        void LeftStop();
        void RightTurn_Spot(unsigned long duration,int Speed);
        void LeftTurn_Spot(unsigned long duration,int Speed);
        void Forward(int duration,int Speed);
        void Backward(unsigned long duration,int Speed);

};

extern fclass code_body;

#endif