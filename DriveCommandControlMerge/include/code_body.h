#ifndef code_body_h
#define code_body_h
#include <vector>

//#define numChars 1024;

struct MD
{
    byte motion;
    char dx, dy;
    byte squal;
    word shutter;
    byte max_pix;
};

class fclass
{
    public:
        fclass();
        std::vector<float> HTTPGET();
        void HTTPPOST(String receivedChars);
        void OFS_Cartesian
                        (
                            MD md, 
                            int * prescaled_tx, 
                            int * prescaled_ty, 
                            int * total_x, 
                            int * total_y
                        );
        //void OFS_Angular(
        //                MD md, 
        //                float * total_x, 
        //                float * total_y, 
        //                float* abs_theta
        //                );
        //void determineAngle()
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

extern fclass code_body;

#endif