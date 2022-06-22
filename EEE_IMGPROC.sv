// synthesis VERILOG_INPUT_VERSION SYSTEMVERILOG_2005

module EEE_IMGPROC(
	// global clock & reset
    input	clk,
    input	reset_n,

    // mm slave
    input							s_chipselect,
    input							s_read,
    input							s_write,
    output	logic	[31:0]	s_readdata,
    input	[31:0]				s_writedata,
    input	[2:0]					s_address,


    // streaming sink
    input	[23:0]            	sink_data,
    input								sink_valid,
    output							sink_ready,
    input								sink_sop,
    input								sink_eop,

    // streaming source
    output	[23:0]			  	   source_data,
    output						        source_valid,
    input									source_ready,
    output								source_sop,
    output								source_eop,

    // conduit export
    input                         mode
	
);

////////////////////////////////////////////////////////////////////////
//
parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 256;
parameter MSG_INTERVAL = 6;
parameter BB_COL_DEFAULT = 24'h4CFF00;
parameter edge_row = 11'd215; //only row to be analysed for edges

logic [7:0]   red, green, blue, grey;
logic [7:0]   red_out, green_out, blue_out;
logic [7:0]	 red_processed, green_processed, blue_processed;
logic red_sector, green_sector, blue_sector, lime_sector, yellow_sector, pink_sector;

logic         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////

// Highlight detected areas
logic [29:0][10:0] edge_list; //for next frame (drawing)
logic [29:0][10:0] measured_list; //stores valid edges belonging to a building
logic filtered_edge_active; //is this an edge coord?
logic [23:0] filtered_0, filtered_1, filtered_edge; //drawing pipeline

assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4

assign filtered_0 = {red_processed, green_processed, blue_processed}; //take output from processing.sv

// Show bounding box
logic r_active, g_active, b_active, l_active, y_active, p_active; //check bounding boxes against coords
assign r_active = (x==r_left && y>r_top && y<r_bottom) | (x==r_right && y>r_top && y<r_bottom) | (y==r_top && x>r_left && x<r_right) | (y==r_bottom && x>r_left && x<r_right);
assign g_active = (x==g_left && y>g_top && y<g_bottom) | (x==g_right && y>g_top && y<g_bottom) | (y==g_top && x>g_left && x<g_right) | (y==g_bottom && x>g_left && x<g_right);
assign b_active = (x==b_left && y>b_top && y<b_bottom) | (x==b_right && y>b_top && y<b_bottom) | (y==b_top && x>b_left && x<b_right) | (y==b_bottom && x>b_left && x<b_right);
assign l_active = (x==l_left && y>l_top && y<l_bottom) | (x==l_right && y>l_top && y<l_bottom) | (y==l_top && x>l_left && x<l_right) | (y==l_bottom && x>l_left && x<l_right);
assign y_active = (x==y_left && y>y_top && y<y_bottom) | (x==y_right && y>y_top && y<y_bottom) | (y==y_top && x>y_left && x<y_right) | (y==y_bottom && x>y_left && x<y_right);
assign p_active = (x==p_left && y>p_top && y<p_bottom) | (x==p_right && y>p_top && y<p_bottom) | (y==p_top && x>p_left && x<p_right) | (y==p_bottom && x>p_left && x<p_right);
assign filtered_1 = (r_active) ? {24'hFF0000} : (g_active) ? {24'h267F00} : (b_active) ? {24'h351AA8} : (l_active) ? {24'h00FF21} : 
					(y_active) ? {24'hFFF300} : (p_active) ? {24'hFF60E9} : filtered_0;	//coloured bounding boxes

integer k;
always@(*) begin
	filtered_edge_active = 0;
	for (k=0; k<30; k=k+1) begin
		if (x==edge_list[k] && x!=0) filtered_edge_active = 1; //check if it's an edge coord for drawing
	end
end

assign filtered_edge = (filtered_edge_active==0) ? filtered_1 : 
					   (y[3]) ? {24'h21007C} : {24'hB2CDE8}; //alternating navy and white line
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? filtered_edge : {red,green,blue}; //switch views

always@(posedge clk) begin
	if (y==479) begin
		if (x<30) edge_list[x] <= measured_list[x];	//use pixel X as counter to copy list for next frame
	end
end

//Count valid pixels to get the image coordinates. Reset and detect packet type on Start of Packet.
logic [10:0] x, y;
logic packet_video;
always@(posedge clk) begin
	if (sop) begin
		x <= 11'h0;
		y <= 11'h0;
		packet_video <= (blue[3:0] == 3'h0);
	end
	else if (in_valid) begin
		if (x == IMAGE_W-1) begin
			x <= 11'h0;
			y <= y + 11'h1;
		end
		else begin
			x <= x + 11'h1;
		end
	end
end

//Find first and last coloured pixels
logic [10:0] r_x_min, r_y_min, r_x_max, r_y_max;
logic [10:0] g_x_min, g_y_min, g_x_max, g_y_max;
logic [10:0] b_x_min, b_y_min, b_x_max, b_y_max;
logic [10:0] l_x_min, l_y_min, l_x_max, l_y_max;
logic [10:0] y_x_min, y_y_min, y_x_max, y_y_max;
logic [10:0] p_x_min, p_y_min, p_x_max, p_y_max;

always@(posedge clk) begin
	if (in_valid) begin
		if (y>240 && y<450 && x>30 && x<610) begin //chop off ceiling and edges
			if (red_sector) begin	//Update bounds when the pixel is red
				if (x < r_x_min) r_x_min <= x;
				if (x > r_x_max) r_x_max <= x;
				if (y < r_y_min) r_y_min <= y;
				r_y_max <= y;
			end else if (green_sector) begin
				if (x < g_x_min) g_x_min <= x;
				if (x > g_x_max) g_x_max <= x;
				if (y < g_y_min) g_y_min <= y;
				g_y_max <= y;
			end else if (blue_sector) begin
				if (x < b_x_min) b_x_min <= x;
				if (x > b_x_max) b_x_max <= x;
				if (y < b_y_min) b_y_min <= y;
				b_y_max <= y;
			end else if (lime_sector) begin
				if (x < l_x_min) l_x_min <= x;
				if (x > l_x_max) l_x_max <= x;
				if (y < l_y_min) l_y_min <= y;
				l_y_max <= y;
			end else if (yellow_sector) begin
				if (x < y_x_min) y_x_min <= x;
				if (x > y_x_max) y_x_max <= x;
				if (y < y_y_min) y_y_min <= y;
				y_y_max <= y;
			end else if (pink_sector) begin
				if (x < p_x_min) p_x_min <= x;
				if (x > p_x_max) p_x_max <= x;
				if (y < p_y_min) p_y_min <= y;
				p_y_max <= y;
			end
		end
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		r_x_min <= IMAGE_W-11'h1;
		r_x_max <= 0;
		r_y_min <= IMAGE_H-11'h1;
		r_y_max <= 0;
		g_x_min <= IMAGE_W-11'h1;
		g_x_max <= 0;
		g_y_min <= IMAGE_H-11'h1;
		g_y_max <= 0;
		b_x_min <= IMAGE_W-11'h1;
		b_x_max <= 0;
		b_y_min <= IMAGE_H-11'h1;
		b_y_max <= 0;
		l_x_min <= IMAGE_W-11'h1;
		l_x_max <= 0;
		l_y_min <= IMAGE_H-11'h1;
		l_y_max <= 0;
		y_x_min <= IMAGE_W-11'h1;
		y_x_max <= 0;
		y_y_min <= IMAGE_H-11'h1;
		y_y_max <= 0;
		p_x_min <= IMAGE_W-11'h1;
		p_x_max <= 0;
		p_y_min <= IMAGE_H-11'h1;
		p_y_max <= 0;
	end
end

//Process bounding box at the end of the frame.
logic [4:0] msg_state;
logic [10:0] r_left, r_right, r_top, r_bottom;
logic [10:0] g_left, g_right, g_top, g_bottom;
logic [10:0] b_left, b_right, b_top, b_bottom;
logic [10:0] l_left, l_right, l_top, l_bottom;
logic [10:0] y_left, y_right, y_top, y_bottom;
logic [10:0] p_left, p_right, p_top, p_bottom;
logic [7:0] frame_count;

always@(posedge clk) begin
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//Latch edges for display overlay on next frame
		r_left <= r_x_min;
		r_right <= r_x_max;
		r_top <= r_y_min;
		r_bottom <= r_y_max;
		g_left <= g_x_min;
		g_right <= g_x_max;
		g_top <= g_y_min;
		g_bottom <= g_y_max;
		b_left <= b_x_min;
		b_right <= b_x_max;
		b_top <= b_y_min;
		b_bottom <= b_y_max;
		l_left <= l_x_min;
		l_right <= l_x_max;
		l_top <= l_y_min;
		l_bottom <= l_y_max;
		y_left <= y_x_min;
		y_right <= y_x_max;
		y_top <= y_y_min;
		y_bottom <= y_y_max;
		p_left <= p_x_min;
		p_right <= p_x_max;
		p_top <= p_y_min;
		p_bottom <= p_y_max;
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
			msg_state <= 5'b1;
			frame_count <= MSG_INTERVAL-1;
		end
	end
	
	//Cycle through message writer states once started
	if (msg_state == 5'b10111) msg_state <= 5'b1;
	else if (msg_state != 5'b0) msg_state <= msg_state + 5'b1;

end
	
//Generate output messages for CPU
logic [31:0] msg_buf_in; 
logic [31:0] msg_buf_out;
logic msg_buf_wr;
logic msg_buf_rd, msg_buf_flush;
logic [7:0] msg_buf_size;
logic msg_buf_empty;


always@(*) begin	//Write words to FIFO as state machine advances
	case(msg_state)
		5'b0: begin
			msg_buf_in = 32'b0;
			msg_buf_wr = 1'b0;
		end
		5'b1: begin
			msg_buf_in = 32'hAAAAAAAA;	//Message start
			msg_buf_wr = 1'b1;
		end
		5'b10: begin
			msg_buf_in = {5'b0, r_x_min, 5'b0, r_x_max};	//Red
			msg_buf_wr = 1'b1;
		end
		5'b11: begin
			msg_buf_in = {5'b0, g_x_min, 5'b0, g_x_max};	//Green
			msg_buf_wr = 1'b1;
		end
		5'b100: begin
			msg_buf_in = {5'b0, b_x_min, 5'b0, b_x_max};	//Blue
			msg_buf_wr = 1'b1;
		end
		5'b101: begin
			msg_buf_in = {5'b0, l_x_min, 5'b0, l_x_max};	//Lime
			msg_buf_wr = 1'b1;
		end
		5'b110: begin
			msg_buf_in = {5'b0, y_x_min, 5'b0, y_x_max};	//Yellow
			msg_buf_wr = 1'b1;
		end
		5'b111: begin
			msg_buf_in = {5'b0, p_x_min, 5'b0, p_x_max};	//Pink
			msg_buf_wr = 1'b1;
		end
		5'b1000: begin
			msg_buf_in = {32'hBBBBBBBB};	//Message start
			msg_buf_wr = 1'b1;
		end
		5'b1001: begin
			msg_buf_in = {5'b0, edge_list[0], 5'b0, edge_list[1]};	//edge_list begins
			msg_buf_wr = 1'b1;
		end
		5'b1010: begin
			msg_buf_in = {5'b0, edge_list[2], 5'b0, edge_list[3]};
			msg_buf_wr = 1'b1;
		end
		5'b1011: begin
			msg_buf_in = {5'b0, edge_list[4], 5'b0, edge_list[5]};
			msg_buf_wr = 1'b1;
		end
		5'b1100: begin
			msg_buf_in = {5'b0, edge_list[6], 5'b0, edge_list[7]};
			msg_buf_wr = 1'b1;
		end
		5'b1101: begin
			msg_buf_in = {5'b0, edge_list[8], 5'b0, edge_list[9]};
			msg_buf_wr = 1'b1;
		end
		5'b1110: begin
			msg_buf_in = {5'b0, edge_list[10], 5'b0, edge_list[11]};
			msg_buf_wr = 1'b1;
		end
		5'b1111: begin
			msg_buf_in = {5'b0, edge_list[12], 5'b0, edge_list[13]};
			msg_buf_wr = 1'b1;
		end
		5'b10000: begin
			msg_buf_in = {5'b0, edge_list[14], 5'b0, edge_list[15]};
			msg_buf_wr = 1'b1;
		end
		5'b10001: begin
			msg_buf_in = {5'b0, edge_list[16], 5'b0, edge_list[17]};
			msg_buf_wr = 1'b1;
		end
		5'b10010: begin
			msg_buf_in = {5'b0, edge_list[18], 5'b0, edge_list[19]};
			msg_buf_wr = 1'b1;
		end
		5'b10011: begin
			msg_buf_in = {5'b0, edge_list[20], 5'b0, edge_list[21]};
			msg_buf_wr = 1'b1;
		end
		5'b10100: begin
			msg_buf_in = {5'b0, edge_list[22], 5'b0, edge_list[23]};
			msg_buf_wr = 1'b1;
		end
		5'b10101: begin
			msg_buf_in = {5'b0, edge_list[24], 5'b0, edge_list[25]};
			msg_buf_wr = 1'b1;
		end
		5'b10110: begin
			msg_buf_in = {5'b0, edge_list[26], 5'b0, edge_list[27]};
			msg_buf_wr = 1'b1;
		end
		5'b10111: begin
			msg_buf_in = {5'b0, edge_list[28], 5'b0, edge_list[29]};
			msg_buf_wr = 1'b1;
		end
	endcase
end

processing img_processing (
	.clk (clk),
	.red(red),
	.green(green),
	.blue(blue),
	.sop(sop),
	.packet_video(packet_video),
	.in_valid(in_valid),
	.x(x),
	.y(y),
	.edge_row(edge_row),

	.red_processed(red_processed),
	.green_processed(green_processed),
	.blue_processed(blue_processed),
	.red_sector(red_sector),
	.green_sector(green_sector),
	.blue_sector(blue_sector),
	.lime_sector(lime_sector),
	.yellow_sector(yellow_sector),
	.pink_sector(pink_sector),
	.measured_list(measured_list)
);


//Output message FIFO
MSG_FIFO	MSG_FIFO_inst (
	.clock (clk),
	.data (msg_buf_in),
	.rdreq (msg_buf_rd),
	.sclr (~reset_n | msg_buf_flush),
	.wrreq (msg_buf_wr),
	.q (msg_buf_out),
	.usedw (msg_buf_size),
	.empty (msg_buf_empty)
	);


//Streaming registers to buffer video signal
STREAM_REG #(.DATA_WIDTH(26)) in_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(sink_ready),
	.valid_out(in_valid),
	.data_out({red,green,blue,sop,eop}),
	.ready_in(out_ready),
	.valid_in(sink_valid),
	.data_in({sink_data,sink_sop,sink_eop})
);

STREAM_REG #(.DATA_WIDTH(26)) out_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(out_ready),
	.valid_out(source_valid),
	.data_out({source_data,source_sop,source_eop}),
	.ready_in(source_ready),
	.valid_in(in_valid),
	.data_in({red_out, green_out, blue_out, sop, eop})
);


/////////////////////////////////
/// Memory-mapped port		 /////
/////////////////////////////////

// Addresses
`define REG_STATUS    			0
`define READ_MSG    				1
`define READ_ID    				2
`define REG_BBCOL					3

//Status register bits
// 31:16 - unimplemented
// 15:8 - number of words in message buffer (read only)
// 7:5 - unused
// 4 - flush message buffer (write only - read as 0)
// 3:0 - unused


// Process write

logic  [7:0]   reg_status;
logic	[23:0]	bb_col;

always @ (posedge clk)
begin
	if (~reset_n)
	begin
		reg_status <= 8'b0;
		bb_col <= BB_COL_DEFAULT;
	end
	else begin
		if(s_chipselect & s_write) begin
		   if      (s_address == `REG_STATUS)	reg_status <= s_writedata[7:0];
		//    if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
		end
	end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);


// Process reads
logic read_d; //Store the read signal for correct updating of the message buffer

// Copy the requested word to the output port when there is a read.
always @ (posedge clk)
begin
   if (~reset_n) begin
	   s_readdata <= {32'b0};
		read_d <= 1'b0;
	end
	
	else if (s_chipselect & s_read) begin
		if   (s_address == `REG_STATUS) s_readdata <= {16'b0,msg_buf_size,reg_status};
		if   (s_address == `READ_MSG) s_readdata <= {msg_buf_out};
		if   (s_address == `READ_ID) s_readdata <= 32'h1234EEE2;
		if   (s_address == `REG_BBCOL) s_readdata <= {8'h0, bb_col};
	end
	
	read_d <= s_read;
end

//Fetch next word from message buffer after read from READ_MSG
assign msg_buf_rd = s_chipselect & s_read & ~read_d & ~msg_buf_empty & (s_address == `READ_MSG);
						


endmodule