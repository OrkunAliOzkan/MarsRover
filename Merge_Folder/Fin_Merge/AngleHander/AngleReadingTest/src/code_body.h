#ifndef code_body_h
#define code_body_h
#include <vector>

class fclass
{
    public:
        fclass();
        double computeAngle(int x, int y);
        float vector_multiply(std::vector<float> x, std::vector<float> y);
        void fclass::readings(  int counter_input, 
                                QMC5883LCompass *compass, 
                                float *angle, 
                                float *headingDegrees);
};

extern fclass code_body;

#endif