module processing (
    input wire clk,
    input wire [7:0] red,
    input wire [7:0] green,
    input wire [7:0] blue,
    input wire sop,
    input wire packet_video,
    input wire in_valid,
    input wire x,
    input wire y,

    output wire [7:0] red_processed,
    output wire [7:0] green_processed,
    output wire [7:0] blue_processed

);

reg [3:0][7:0]  r_buff, g_buff, b_buff = 0; //4 pixels
reg [1920:0][7:0] horiz_r_buff, horiz_g_buff, horiz_b_buff; //3 rows + 1 pixel
wire [7:0]  r_blur_x, g_blur_x, b_blur_x;
wire [7:0]  r_blur_y, g_blur_y, b_blur_y;
wire [9:0] r_sum_x, g_sum_x, b_sum_x;   //FF * 4
wire [9:0] r_sum_y, g_sum_y, b_sum_y;

//shift register
integer i;
always@(posedge clk) begin
    if (in_valid) begin
        for (i=3; i>0; i=i-1) begin   //delay line
                r_buff[i] <= r_buff[i-1];
                g_buff[i] <= g_buff[i-1];
                b_buff[i] <= b_buff[i-1];
        end
        r_buff[0] <= red;     //store new pixel
        g_buff[0] <= green;
        b_buff[0] <= blue;
    end
end

//horiz shift register
always@(posedge clk) begin
    if (in_valid) begin
        for (i=1920; i>0; i=i-1) begin   //horizontal delay line
                horiz_r_buff[i] <= horiz_r_buff[i-1];
                horiz_g_buff[i] <= horiz_g_buff[i-1];
                horiz_b_buff[i] <= horiz_b_buff[i-1];
        end
        horiz_r_buff[0] <= r_blur_x;
        horiz_g_buff[0] <= g_blur_x;
        horiz_b_buff[0] <= b_blur_x;
    end
end

assign r_sum_x = r_buff[0] + r_buff[1] + r_buff[2] + r_buff[3];
assign g_sum_x = g_buff[0] + g_buff[1] + g_buff[2] + g_buff[3];
assign b_sum_x = b_buff[0] + b_buff[1] + b_buff[2] + b_buff[3];
assign r_blur_x = r_sum_x / 4; //normalise
assign g_blur_x = g_sum_x / 4;
assign b_blur_x = b_sum_x / 4;

assign r_sum_y = horiz_r_buff[0] + horiz_r_buff[640] + horiz_r_buff[1280] + horiz_r_buff[1920];
assign g_sum_y = horiz_g_buff[0] + horiz_g_buff[640] + horiz_g_buff[1280] + horiz_g_buff[1920];
assign b_sum_y = horiz_b_buff[0] + horiz_b_buff[640] + horiz_b_buff[1280] + horiz_b_buff[1920];
assign r_blur_y = r_sum_y / 4; //normalise
assign g_blur_y = g_sum_y / 4;
assign b_blur_y = b_sum_y / 4;

assign red_processed = r_blur_y;
assign green_processed = g_blur_y;
assign blue_processed = b_blur_y;

endmodule