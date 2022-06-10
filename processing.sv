module processing (
	input wire clk,
	input wire [7:0] red,
	input wire [7:0] green,
	input wire [7:0] blue,
	input wire [23:0] grey,
	input wire sop,
	input wire packet_video,
	input wire in_valid,
	input wire x,
	input wire y,

	output wire [7:0] red_processed,
	output wire [7:0] green_processed,
	output wire [7:0] blue_processed

);

wire [15:0] Hue; //0-360.. % 360
wire [7:0] Sat, Val; //0-100, 0-255
wire [7:0] RGB_max, RGB_min;
wire [7:0] RGB_diff;

assign RGB_max = (red>green && red>blue) ? red :
				 (green>blue) ? green : blue;
assign RGB_min = (red<green && red<blue) ? red :
				 (green<blue) ? green : blue;
assign RGB_diff = ((RGB_max-RGB_min) != 0) ? RGB_max-RGB_min : 8'b1;
assign Val = RGB_max;
assign Sat = (Val==0) ? 0 : (100*RGB_diff) / RGB_max;
assign Hue = (RGB_max==red) ? (60*(green-blue)/RGB_diff) % 360 :
			 (RGB_max==green) ? (120+(60*(blue-red)/RGB_diff)) % 360 : 
			 (240+(60*(red-green)/RGB_diff)) % 360;

assign red_processed = (Hue>100 && Hue<155) ? red : grey;
assign green_processed = (Hue>100 && Hue<155) ? green : grey;
assign blue_processed = (Hue>100 && Hue<155) ? blue : grey;


// assign red_processed = (RGB_max==red) ? red : 8'h0;
// assign green_processed = (RGB_max==green) ? green : 8'h0;
// assign blue_processed = (RGB_max==blue) ? blue : 8'h0;







// reg [7:0] r_buff_0, g_buff_0, b_buff_0 [3:0]; //4 pixels
// reg [7:0] r_buff_1, g_buff_1, b_buff_1 [3:0];
// reg [7:0] r_buff_2, g_buff_2, b_buff_2 [3:0];

// reg [7:0] horiz_r_buff_0, horiz_g_buff_0, horiz_b_buff_0 [1920:0]; //3 rows + 1 pixel
// reg [7:0] horiz_r_buff_1, horiz_g_buff_1, horiz_b_buff_1 [1920:0];
// reg [7:0] horiz_r_buff_2, horiz_g_buff_2, horiz_b_buff_2 [1920:0];

// wire [9:0] r_blur_x_0, g_blur_x_0, b_blur_x_0;   //FF * 4
// wire [9:0] r_blur_x_1, g_blur_x_1, b_blur_x_1;
// wire [9:0] r_blur_x_2, g_blur_x_2, b_blur_x_2;

// wire [9:0] r_blur_y_0, g_blur_y_0, b_blur_y_0;   //FF * 4
// wire [9:0] r_blur_y_1, g_blur_y_1, b_blur_y_1;
// wire [9:0] r_blur_y_2, g_blur_y_2, b_blur_y_2;

// //shift register
// integer i;
// always@(posedge clk) begin
// 	if (in_valid) begin
// 		for (i=3; i>0; i=i-1) begin   //delay line
// 				r_buff_0[i] <= r_buff_0[i-1];
// 				g_buff_0[i] <= g_buff_0[i-1];	//first buffer
// 				b_buff_0[i] <= b_buff_0[i-1];

// 				r_buff_1[i] <= r_buff_1[i-1];
// 				g_buff_1[i] <= g_buff_1[i-1];	//second buffer
// 				b_buff_1[i] <= b_buff_1[i-1];

// 				r_buff_2[i] <= r_buff_2[i-1];
// 				g_buff_2[i] <= g_buff_2[i-1];	//third buffer
// 				b_buff_2[i] <= b_buff_2[i-1];
// 		end
// 		r_buff_0[0] <= red;  //store new pixel
// 		g_buff_0[0] <= green;
// 		b_buff_0[0] <= blue;

// 		r_buff_1[0] <= r_blur_x_0;	//feed in from prev buffer
// 		g_buff_1[0] <= g_blur_x_0;
// 		b_buff_1[0] <= b_blur_x_0;

// 		r_buff_2[0] <= r_blur_x_1;
// 		g_buff_2[0] <= g_blur_x_1;
// 		b_buff_2[0] <= b_blur_x_1;
// 	end
// end

// //horiz shift register
// always@(posedge clk) begin
// 	if (in_valid) begin
// 		for (i=1920; i>0; i=i-1) begin   //horizontal delay line
// 				horiz_r_buff_0[i] <= horiz_r_buff_0[i-1];
// 				horiz_g_buff_0[i] <= horiz_g_buff_0[i-1];	//first horiz buffer
// 				horiz_b_buff_0[i] <= horiz_b_buff_0[i-1];
				
// 				horiz_r_buff_1[i] <= horiz_r_buff_1[i-1];
// 				horiz_g_buff_1[i] <= horiz_g_buff_1[i-1];	//second horiz buffer
// 				horiz_b_buff_1[i] <= horiz_b_buff_1[i-1];
				
// 				horiz_r_buff_2[i] <= horiz_r_buff_2[i-1];
// 				horiz_g_buff_2[i] <= horiz_g_buff_2[i-1];	//third horiz buffer
// 				horiz_b_buff_2[i] <= horiz_b_buff_2[i-1];
// 		end
// 		horiz_r_buff_0[0] <= r_blur_x_2;	//feed in from last small buffer
// 		horiz_g_buff_0[0] <= g_blur_x_2;
// 		horiz_b_buff_0[0] <= b_blur_x_2;
		
// 		horiz_r_buff_1[0] <= r_blur_y_0;
// 		horiz_g_buff_1[0] <= g_blur_y_0;
// 		horiz_b_buff_1[0] <= b_blur_y_0;
		
// 		horiz_r_buff_2[0] <= r_blur_y_1;
// 		horiz_g_buff_2[0] <= g_blur_y_1;
// 		horiz_b_buff_2[0] <= b_blur_y_1;
// 	end
// end

// assign r_blur_x_0 = (r_buff_0[0] + r_buff_0[1] + r_buff_0[2] + r_buff_0[3]) >> 2;
// assign g_blur_x_0 = (g_buff_0[0] + g_buff_0[1] + g_buff_0[2] + g_buff_0[3]) >> 2;
// assign b_blur_x_0 = (b_buff_0[0] + b_buff_0[1] + b_buff_0[2] + b_buff_0[3]) >> 2;

// assign r_blur_x_1 = (r_buff_1[0] + r_buff_1[1] + r_buff_1[2] + r_buff_1[3]) >> 2;
// assign g_blur_x_1 = (g_buff_1[0] + g_buff_1[1] + g_buff_1[2] + g_buff_1[3]) >> 2;
// assign b_blur_x_1 = (b_buff_1[0] + b_buff_1[1] + b_buff_1[2] + b_buff_1[3]) >> 2;

// assign r_blur_x_2 = (r_buff_2[0] + r_buff_2[1] + r_buff_2[2] + r_buff_2[3]) >> 2;
// assign g_blur_x_2 = (g_buff_2[0] + g_buff_2[1] + g_buff_2[2] + g_buff_2[3]) >> 2;
// assign b_blur_x_2 = (b_buff_2[0] + b_buff_2[1] + b_buff_2[2] + b_buff_2[3]) >> 2;

// assign r_blur_y_0 = (horiz_r_buff_0[0] + horiz_r_buff_0[640] + horiz_r_buff_0[1280] + horiz_r_buff_0[1920]) >> 2;
// assign g_blur_y_0 = (horiz_g_buff_0[0] + horiz_g_buff_0[640] + horiz_g_buff_0[1280] + horiz_g_buff_0[1920]) >> 2;
// assign b_blur_y_0 = (horiz_b_buff_0[0] + horiz_b_buff_0[640] + horiz_b_buff_0[1280] + horiz_b_buff_0[1920]) >> 2;

// assign r_blur_y_1 = (horiz_r_buff_1[0] + horiz_r_buff_1[640] + horiz_r_buff_1[1280] + horiz_r_buff_1[1920]) >> 2;
// assign g_blur_y_1 = (horiz_g_buff_1[0] + horiz_g_buff_1[640] + horiz_g_buff_1[1280] + horiz_g_buff_1[1920]) >> 2;
// assign b_blur_y_1 = (horiz_b_buff_1[0] + horiz_b_buff_1[640] + horiz_b_buff_1[1280] + horiz_b_buff_1[1920]) >> 2;

// assign r_blur_y_2 = (horiz_r_buff_2[0] + horiz_r_buff_2[640] + horiz_r_buff_2[1280] + horiz_r_buff_2[1920]) >> 2;
// assign g_blur_y_2 = (horiz_g_buff_2[0] + horiz_g_buff_2[640] + horiz_g_buff_2[1280] + horiz_g_buff_2[1920]) >> 2;
// assign b_blur_y_2 = (horiz_b_buff_2[0] + horiz_b_buff_2[640] + horiz_b_buff_2[1280] + horiz_b_buff_2[1920]) >> 2;

// assign red_processed = r_blur_y_2[7:0]; //first 8 bits
// assign green_processed = g_blur_y_2[7:0];
// assign blue_processed = b_blur_y_2[7:0];

endmodule