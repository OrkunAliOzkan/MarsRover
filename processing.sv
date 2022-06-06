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

reg [12:0][7:0]  r_ring, g_ring, b_ring; //previous input values
reg [3:0]   count_input;
wire [7:0]  r_blur_x, g_blur_x, b_blur_x;
wire [18:0] r_sum_x, g_sum_x, b_sum_x;

//ring buffer
always@(posedge clk) begin  
    if (sop || x==0) begin          //set buffer to 0
        r_ring <= 104'h0;
        g_ring <= 104'h0;
        b_ring <= 104'h0;
        count_input <= 0;
    end
    else if (packet_video) begin
        if (count_input == 12) count_input <= 0; // % 13
        else count_input <= count_input + 1;
        r_ring[count_input] <= red;     //store new pixel
        g_ring[count_input] <= green;
        b_ring[count_input] <= blue;
    end
end

reg [12:0][8:0] gauss = {9'd1, 9'd6, 9'd28, 9'd89, 9'd205, 9'd338, 9'd400, 9'd338, 9'd205, 9'd89, 9'd28, 9'd6, 9'd1};   //gaussian kernel, 9*13 width

assign r_sum_x = gauss[0]*r_ring[count_input % 13] + gauss[1]*r_ring[(count_input+1) % 13] + gauss[2]*r_ring[(count_input+2) % 13] + gauss[3]*r_ring[(count_input+3) % 13] + gauss[4]*r_ring[(count_input+4) % 13] + gauss[5]*r_ring[(count_input+5) % 13] + gauss[6]*r_ring[(count_input+6) % 13] + gauss[7]*r_ring[(count_input+7) % 13] + gauss[8]*r_ring[(count_input+8) % 13] + gauss[9]*r_ring[(count_input+9) % 13] + gauss[10]*r_ring[(count_input+10) % 13] + gauss[11]*r_ring[(count_input+11) % 13] + gauss[12]*r_ring[(count_input+12) % 13];
assign g_sum_x = gauss[0]*g_ring[count_input % 13] + gauss[1]*g_ring[(count_input+1) % 13] + gauss[2]*g_ring[(count_input+2) % 13] + gauss[3]*g_ring[(count_input+3) % 13] + gauss[4]*g_ring[(count_input+4) % 13] + gauss[5]*g_ring[(count_input+5) % 13] + gauss[6]*g_ring[(count_input+6) % 13] + gauss[7]*g_ring[(count_input+7) % 13] + gauss[8]*g_ring[(count_input+8) % 13] + gauss[9]*g_ring[(count_input+9) % 13] + gauss[10]*g_ring[(count_input+10) % 13] + gauss[11]*g_ring[(count_input+11) % 13] + gauss[12]*g_ring[(count_input+12) % 13];
assign b_sum_x = gauss[0]*b_ring[count_input % 13] + gauss[1]*b_ring[(count_input+1) % 13] + gauss[2]*b_ring[(count_input+2) % 13] + gauss[3]*b_ring[(count_input+3) % 13] + gauss[4]*b_ring[(count_input+4) % 13] + gauss[5]*b_ring[(count_input+5) % 13] + gauss[6]*b_ring[(count_input+6) % 13] + gauss[7]*b_ring[(count_input+7) % 13] + gauss[8]*b_ring[(count_input+8) % 13] + gauss[9]*b_ring[(count_input+9) % 13] + gauss[10]*b_ring[(count_input+10) % 13] + gauss[11]*b_ring[(count_input+11) % 13] + gauss[12]*b_ring[(count_input+12) % 13];

assign r_blur_x = r_sum_x / 1734; //normalise
assign g_blur_x = g_sum_x / 1734;
assign b_blur_x = b_sum_x / 1734;


assign red_processed = r_blur_x;
assign green_processed = g_blur_x;
assign blue_processed = b_blur_x;

endmodule