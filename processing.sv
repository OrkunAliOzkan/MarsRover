module processing (
    input wire clk,
    input wire [7:0] red,
    input wire [7:0] green,
    input wire [7:0] blue,
    input wire sop,
    input wire packet_video,
    input wire x,
    input wire y,

    output wire [7:0] red_processed,
    output wire [7:0] green_processed,
    output wire [7:0] blue_processed

);

reg [3:0][7:0]  r_buff, g_buff, b_buff = 0; //previous input values
wire [7:0]  r_blur_x, g_blur_x, b_blur_x;
wire [9:0] r_sum_x, g_sum_x, b_sum_x;   //FF * 4

//shift register
integer i;
always@(posedge clk) begin
    for (i=3; i>0; i=i-1) begin   //delay line
            r_buff[i] <= r_buff[i-1];
            g_buff[i] <= g_buff[i-1];
            b_buff[i] <= b_buff[i-1];
    end
    r_buff[0] <= red;     //store new pixel
    g_buff[0] <= green;
    b_buff[0] <= blue;
end

assign r_sum_x = r_buff[0] + r_buff[1] + r_buff[2] + r_buff[3];
assign g_sum_x = g_buff[0] + g_buff[1] + g_buff[2] + g_buff[3];
assign b_sum_x = b_buff[0] + b_buff[1] + b_buff[2] + b_buff[3];
assign r_blur_x = r_sum_x / 4; //normalise
assign g_blur_x = g_sum_x / 4;
assign b_blur_x = b_sum_x / 4;


assign red_processed = r_blur_x;
assign green_processed = g_blur_x;
assign blue_processed = b_blur_x;

endmodule