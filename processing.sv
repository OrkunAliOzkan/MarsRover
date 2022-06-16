module processing (
	input wire clk,
	input wire [7:0] red,
	input wire [7:0] green,
	input wire [7:0] blue,
	input wire [7:0] grey, /////////////////////////////////////
	input wire sop,
	input wire packet_video,
	input wire in_valid,
	input wire x,
	input wire y,
	output wire [7:0] red_processed,
	output wire [7:0] green_processed,
	output wire [7:0] blue_processed,
	output wire red_sector
);

/////////////////////////////////////////////////////Blur

reg [7:0] r_buff_0 [3:0]; //4 pixels
reg [7:0] g_buff_0 [3:0];
reg [7:0] b_buff_0 [3:0];
reg [7:0] r_buff_1 [3:0];
reg [7:0] g_buff_1 [3:0];
reg [7:0] b_buff_1 [3:0];
reg [7:0] r_buff_2 [3:0];
reg [7:0] g_buff_2 [3:0];
reg [7:0] b_buff_2 [3:0];
reg [15:0] r_blur_x_0, g_blur_x_0, b_blur_x_0;
reg [15:0] r_blur_x_1, g_blur_x_1, b_blur_x_1;
reg [15:0] r_blur_x_2, g_blur_x_2, b_blur_x_2;

//shift register
integer i;
always@(posedge clk) begin
    if (in_valid) begin
        for (i=3; i>0; i=i-1) begin   //delay line
                r_buff_0[i] <= r_buff_0[i-1];
                g_buff_0[i] <= g_buff_0[i-1];   //first buffer
                b_buff_0[i] <= b_buff_0[i-1];
                r_buff_1[i] <= r_buff_1[i-1];
                g_buff_1[i] <= g_buff_1[i-1];   //second buffer
                b_buff_1[i] <= b_buff_1[i-1];
                r_buff_2[i] <= r_buff_2[i-1];
                g_buff_2[i] <= g_buff_2[i-1];   //third buffer
                b_buff_2[i] <= b_buff_2[i-1];
        end
        r_buff_0[0] <= red;  //store new pixel
        g_buff_0[0] <= green;
        b_buff_0[0] <= blue;
        r_buff_1[0] <= r_blur_x_0;  //feed in from prev buffer
        g_buff_1[0] <= g_blur_x_0;
        b_buff_1[0] <= b_blur_x_0;
        r_buff_2[0] <= r_blur_x_1;
        g_buff_2[0] <= g_blur_x_1;
        b_buff_2[0] <= b_blur_x_1;
    end
end

wire [7:0] red_b, green_b, blue_b, grey_b;//, red_bb, green_bb, blue_bb;
// assign red_b = r_buff_0[0];//////////////////
// assign green_b = g_buff_0[0];////////////////
// assign blue_b = b_buff_0[0];///////////////////
// assign red_bb = r_buff_0[1];//////////////////
// assign green_bb = g_buff_0[1];////////////////
// assign blue_bb = b_buff_0[1];///////////////////
// assign grey_b = green_b[7:1] + red_b[7:2] + blue_b[7:2];//////////////////

always@(posedge clk) begin
	r_blur_x_0 <= (r_buff_0[0] + r_buff_0[1] + r_buff_0[2] + r_buff_0[3]) >> 2; // sum/4
	g_blur_x_0 <= (g_buff_0[0] + g_buff_0[1] + g_buff_0[2] + g_buff_0[3]) >> 2;
	b_blur_x_0 <= (b_buff_0[0] + b_buff_0[1] + b_buff_0[2] + b_buff_0[3]) >> 2;
	r_blur_x_1 <= (r_buff_1[0] + r_buff_1[1] + r_buff_1[2] + r_buff_1[3]) >> 2;
	g_blur_x_1 <= (g_buff_1[0] + g_buff_1[1] + g_buff_1[2] + g_buff_1[3]) >> 2;
	b_blur_x_1 <= (b_buff_1[0] + b_buff_1[1] + b_buff_1[2] + b_buff_1[3]) >> 2;
	r_blur_x_2 <= (r_buff_2[0] + r_buff_2[1] + r_buff_2[2] + r_buff_2[3]) >> 2;
	g_blur_x_2 <= (g_buff_2[0] + g_buff_2[1] + g_buff_2[2] + g_buff_2[3]) >> 2;
	b_blur_x_2 <= (b_buff_2[0] + b_buff_2[1] + b_buff_2[2] + b_buff_2[3]) >> 2;

	red_b <= r_blur_x_2[7:0];
	green_b <= g_blur_x_2[7:0];
	blue_b <= b_blur_x_2[7:0];
	grey_b <= green_b[7:1] + red_b[7:2] + blue_b[7:2]; //Grey = green/2 + red/4 + blue/4
end

///////////////////////////////////////////////////HSV

reg signed [15:0] Hue_0, Hue; //0-360.. % 360
reg [7:0] Sat, Val; //0-100, 0-255
reg [7:0] RGB_max, RGB_min;
reg [7:0] RGB_diff;

// reg [7:0] Sat_next, Val_next; //0-100, 0-255
// reg alternator = 0;
// always@(posedge clk) alternator <= (alternator) ? 0 : 1; //flips alternator between 0 & 1 to avoid doing %2

// always@(posedge clk) begin	//pipelined HSV at half resolution to spread divides
// 	if (alternator) begin	//val and sat half
// 		if (red_b>green_b && red_b>blue_b) RGB_max <= red_b;
// 			else if (green_b>blue_b) RGB_max <= green_b;
// 			else RGB_max <= blue_b;
// 		if (red_b<green_b && red_b<blue_b) RGB_max <= red_b;
// 			else if (green_b<blue_b) RGB_max <= green_b;
// 			else RGB_max <= blue_b;
// 		if ((RGB_max-RGB_min) != 0) RGB_diff <= RGB_max - RGB_min;
// 			else RGB_diff <= 8'b1;
// 		Val_next <= RGB_max;
// 		if (Val_next==0) Sat_next <= 0;
// 			else Sat_next <= (100*RGB_diff) / RGB_max;
// 	end else begin	//hue half
// 		Sat <= Sat_next;
// 		Val <= Val_next;
// 		if (RGB_max==red_bb) Hue_0 <= 60*(green_bb-blue_bb) / RGB_diff;
// 			else if (RGB_max==green_bb) Hue_0 <= 120+(60*(blue_bb-red_bb) / RGB_diff);
// 		else Hue_0 <= 240+(60*(red_bb-green_bb) / RGB_diff);
// 			if (Hue_0<0) Hue <= Hue_0 + 360;
// 		else Hue <= Hue_0;
// 	end
// end

// always@(posedge clk) begin
// 	if (red_b>green_b && red_b>blue_b) RGB_max <= red_b;
// 		else if (green_b>blue_b) RGB_max <= green_b;
// 		else RGB_max <= blue_b;
// 	if (red_b<green_b && red_b<blue_b) RGB_max <= red_b;
// 		else if (green_b<blue_b) RGB_max <= green_b;
// 		else RGB_max <= blue_b;
// 	if ((RGB_max-RGB_min) != 0) RGB_diff <= RGB_max - RGB_min;
// 		else RGB_diff <= 8'b1;
// 	Val <= RGB_max;
// 	if (Val==0) Sat <= 0;
// 		else Sat <= (100*RGB_diff) / RGB_max;
// 	if (RGB_max==red_b) Hue_0 <= 60*(green_b-blue_b) / RGB_diff;
// 		else if (RGB_max==green_b) Hue_0 <= 120+(60*(blue_b-red_b) / RGB_diff);
// 		else Hue_0 <= 240+(60*(red_b-green_b) / RGB_diff);
// 	if (Hue_0<0) Hue <= Hue_0 + 360;
// 		else Hue <= Hue_0;

assign RGB_max = (red_b>green_b && red_b>blue_b) ? red_b :
                 (green_b>blue_b) ? green_b : blue_b;
assign RGB_min = (red_b<green_b && red_b<blue_b) ? red_b :
                 (green_b<blue_b) ? green_b : blue_b;
assign RGB_diff = ((RGB_max-RGB_min) != 0) ? RGB_max-RGB_min : 8'b1;
assign Val = RGB_max;
assign Sat = (Val==0) ? 0 : (100*RGB_diff) / RGB_max;
assign Hue_0 = (RGB_max==red_b) ? (60*(green_b-blue_b)/RGB_diff):
             (RGB_max==green_b) ? (120+(60*(blue_b-red_b)/RGB_diff)): 
             (240+(60*(red_b-green_b)/RGB_diff));
assign Hue = (Hue_0<0) ? Hue_0 + 360 : Hue_0;

assign red_sector = ((Hue>5 && Hue<15) && (Sat>40)) && (Val>20 && Val<200);

assign red_processed = (red_sector) ? red_b : grey_b;
assign green_processed = (red_sector) ? green_b : grey_b;
assign blue_processed = (red_sector) ? blue_b : grey_b;

// assign red_processed = grey_b;
// assign green_processed = grey_b;
// assign blue_processed = grey_b;


endmodule