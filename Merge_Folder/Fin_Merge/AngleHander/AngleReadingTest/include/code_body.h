#ifndef code_body_h
#define code_body_h
#include <vector>

class fclass
{
    public:
        fclass();
        std::vector<float> HTTPGET();
        void HTTPPOST();
        double computeAngle(int x, int y);
        float vector_multiply(
                            std::vector<float> x, 
                            std::vector<float> y
                            );
        void readings(
                    int counter_input, 
                    QMC5883LCompass compass, 
                    float *angle, 
                    float *headingDegrees,
                    int *distance_x,
                    int *distance_y,
                    int *total_x1,
                    int *total_y1,
                    int *total_x,
                    int *total_y
                    );
        int convTwosComp(int b);
        void mousecam_reset();
        int mousecam_init();
        void mousecam_write_reg(
                                int *reg, 
                                int *val
                                );
        int mousecam_read_reg(int reg);
        void mousecam_read_motion(struct MD *p);
};

struct MD
{
    byte motion;
    char dx, dy;
    byte squal;
    word shutter;
    byte max_pix;
};

extern fclass code_body;

#endif