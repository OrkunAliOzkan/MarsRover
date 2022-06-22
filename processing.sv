// synthesis VERILOG_INPUT_VERSION SYSTEMVERILOG_2005

module processing (
	input logic clk,
	input logic [7:0] red,
	input logic [7:0] green,
	input logic [7:0] blue,
	input logic sop,
	input logic packet_video,
	input logic in_valid,
	input logic [10:0] x,
	input logic [10:0] y,
    input logic [10:0] edge_row,

	output logic [7:0] red_processed,
	output logic [7:0] green_processed,
	output logic [7:0] blue_processed,
	output logic red_sector,
	output logic green_sector,
	output logic blue_sector,
	output logic lime_sector,
	output logic yellow_sector,
	output logic pink_sector,
	output logic [29:0][10:0] measured_list
);

/////////////////////////////////////////////////////Blur

logic [3:0][7:0] r_buff_0; //4 pixels
logic [3:0][7:0] g_buff_0;
logic [3:0][7:0] b_buff_0;
logic [3:0][7:0] r_buff_1;
logic [3:0][7:0] g_buff_1;
logic [3:0][7:0] b_buff_1;
logic [3:0][7:0] r_buff_2;
logic [3:0][7:0] g_buff_2;
logic [3:0][7:0] b_buff_2;
logic [15:0] r_blur_x_0, g_blur_x_0, b_blur_x_0;
logic [15:0] r_blur_x_1, g_blur_x_1, b_blur_x_1;
logic [15:0] r_blur_x_2, g_blur_x_2, b_blur_x_2;

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

logic [7:0] grey, red_b, green_b, blue_b, grey_b;

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
	grey <= green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4
	grey_b <= green_b[7:1] + red_b[7:2] + blue_b[7:2];
end

///////////////////////////////////////////////////HSV

logic signed [15:0] Hue_0;
logic [15:0] Hue; //0-360.. % 360
logic [7:0] Sat, Val; //0-100, 0-255
logic [7:0] RGB_max, RGB_min;
logic [7:0] RGB_diff;

assign RGB_max = (blue_b>green_b && blue_b>red_b) ? blue_b :
                 (green_b>red_b) ? green_b : red_b;
assign RGB_min = (blue_b<green_b && blue_b<red_b) ? blue_b :
                 (green_b<red_b) ? green_b : red_b;
assign RGB_diff = ((RGB_max-RGB_min) != 0) ? RGB_max-RGB_min : 8'b1;
assign Val = RGB_max;
assign Sat = (Val==0) ? 0 : (100*RGB_diff) / RGB_max;
assign Hue_0 = (RGB_max==red_b) ? (60*(green_b-blue_b)/RGB_diff):
             (RGB_max==green_b) ? (120+(60*(blue_b-red_b)/RGB_diff)): 
             (240+(60*(red_b-green_b)/RGB_diff));
assign Hue = (Hue_0<0) ? Hue_0 + 360 :
			 (Hue_0>360) ? Hue_0 - 360 : Hue_0;

assign red_sector = ((Hue>=0 && Hue<15) || (Hue>358 && Hue<=360)) && (Sat>40);
assign green_sector = (Hue>165 && Hue<180) && (Sat>20);
assign blue_sector = (Hue>215 && Hue<240) && (Sat>35);
assign lime_sector = (Hue>120 && Hue<150) && (Sat>20 && Sat<70) && (Val>100);
assign yellow_sector = (Hue>45 && Hue<65) && (Sat>35) && (Val>150);
assign pink_sector = (Hue>330 && Hue<348) && (Sat>20 && Sat<70) && (Val>120);

////////////////////////////////////////////////////////edge

logic signed [2:0][8:0] edge_buff;
logic signed [8:0] conv_x, edge_x;
logic [7:0] edge_sharp;

always@(posedge clk) begin
	if (x==0) edge_buff <= {27'b0};
    if (in_valid) begin
        for (i=2; i>0; i=i-1) begin   //delay line
            edge_buff[i] <= edge_buff[i-1];
        end
        edge_buff[0] <= {1'b0, grey};
    	conv_x = edge_buff[0] - edge_buff[2]; //[1, 0, -1]
    end
end

assign edge_x = (conv_x<0) ? -conv_x : conv_x; //abs
assign edge_sharp = (edge_x>40) ? 8'hff : 0;	//sharpen

logic [7:0] measured_count;
logic [10:0] edge_prev;

integer k;
always@(posedge clk) begin
	if (sop) begin	//reset at frame start
		edge_prev <= 0;
		measured_count <= 0;
		measured_list <= 330'b0;
	end
	if (in_valid) begin
		if (y==edge_row) begin
			if (x>30 && x<610) begin
				if (edge_sharp==8'hff) begin
					if (edge_prev==0) begin	//first edge
						measured_list[measured_count] <= x;
						measured_count <= measured_count + 1;
					end
					else if (x-edge_prev > 2) begin	//small gap rejection
							measured_list[measured_count] <= x;
							measured_count <= measured_count + 1;
					end
					edge_prev <= x;	//next edge prev if 255
				end
			end
		end
	end
end

logic [7:0]	 red_col, green_col, blue_col;

assign red_col = (red_sector || green_sector || blue_sector || lime_sector || yellow_sector || pink_sector) ? red_b : grey_b;
assign green_col = (red_sector || green_sector || blue_sector || lime_sector || yellow_sector || pink_sector) ? green_b : grey_b;
assign blue_col = (red_sector || green_sector || blue_sector || lime_sector || yellow_sector || pink_sector) ? blue_b : grey_b;

assign red_processed =  (y==edge_row-2 | y==edge_row-1 | y==edge_row | y==edge_row+1 | y==edge_row+2 ) ? edge_sharp : red_col;
assign green_processed = (y==edge_row-2 | y==edge_row-1 | y==edge_row | y==edge_row+1 | y==edge_row+2 ) ? edge_sharp : green_col;
assign blue_processed = (y==edge_row-2 | y==edge_row-1 | y==edge_row | y==edge_row+1 | y==edge_row+2 ) ? edge_sharp : blue_col;

endmodule