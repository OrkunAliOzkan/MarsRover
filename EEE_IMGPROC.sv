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
parameter BB_COL_DEFAULT = 24'hff0000;
parameter edge_row = 11'd240; //only row to be analysed for edges

logic [7:0]   red, green, blue, grey;
logic [7:0]   red_out, green_out, blue_out;
logic [7:0]	 red_processed, green_processed, blue_processed;
logic red_sector;

logic         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////

// Detect red areas
logic red_detect;
// assign red_detect = red[7] & ~green[7] & ~blue[7];
assign red_detect = red_sector;

// Highlight detected areas
logic [29:0][10:0] edge_list; //for next frame (drawing)
logic [29:0][10:0] measured_list; //stores valid edges belonging to a building
logic filtered_edge_active;
logic [23:0] filtered_0, filtered_1, filtered_edge;
assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4
assign filtered_0 = {red_processed, green_processed, blue_processed};

// Show bounding box
logic bb_active;
assign bb_active = (x==left && y>top && y<bottom) | (x==right && y>top && y<bottom) | (y==top && x>left && x<right) | (y==bottom && x>left && x<right);
assign filtered_1 = (bb_active) ? bb_col :
				   		(y==edge_row-1 | y==edge_row | y==edge_row+1) ? {red, green, blue} : filtered_0;

integer k;
always@(*) begin
	filtered_edge_active = 0;
	for (k=0; k<30; k=k+1) begin
		if (x==edge_list[k]) filtered_edge_active = 1;
	end
end

assign filtered_edge = (filtered_edge_active==0) ? filtered_1 : 
					   (y[3]) ? {24'h21007C} : {24'hB2CDE8}; //alternating navy and white line
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? filtered_edge : {red,green,blue};

always@(posedge clk) begin
	if (y==edge_row+1) begin
		if (x<30) edge_list[x] <= measured_list[x];	//copy list for next frame after edge row
	end
end

//Count valid pixels to tget the image coordinates. Reset and detect packet type on Start of Packet.
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

//Find first and last red pixels
logic [10:0] x_min, y_min, x_max, y_max;
always@(posedge clk) begin
	if (red_detect & in_valid) begin	//Update bounds when the pixel is red
		if (y>240 && y<450 && x>30 && x<610) begin //chop off ceiling and edges
			if (x < x_min) x_min <= x;
			if (x > x_max) x_max <= x;
			if (y < y_min) y_min <= y;
			y_max <= y;
		end
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		x_min <= IMAGE_W-11'h1;
		x_max <= 0;
		y_min <= IMAGE_H-11'h1;
		y_max <= 0;
	end
end

//Process bounding box at the end of the frame.
logic [1:0] msg_state;
logic [10:0] left, right, top, bottom;
logic [7:0] frame_count;
always@(posedge clk) begin
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//Latch edges for display overlay on next frame
		left <= x_min;
		right <= x_max;
		top <= y_min;
		bottom <= y_max;
		
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
			msg_state <= 2'b01;
			frame_count <= MSG_INTERVAL-1;
		end
	end
	
	//Cycle through message writer states once started
	if (msg_state != 2'b00) msg_state <= msg_state + 2'b01;

end
	
//Generate output messages for CPU
logic [31:0] msg_buf_in; 
logic [31:0] msg_buf_out;
logic msg_buf_wr;
logic msg_buf_rd, msg_buf_flush;
logic [7:0] msg_buf_size;
logic msg_buf_empty;

`define RED_BOX_MSG_ID "RBB"

always@(*) begin	//Write words to FIFO as state machine advances
	case(msg_state)
		2'b00: begin
			msg_buf_in = 32'b0;
			msg_buf_wr = 1'b0;
		end
		2'b01: begin
			msg_buf_in = `RED_BOX_MSG_ID;	//Message ID
			msg_buf_wr = 1'b1;
		end
		2'b10: begin
			msg_buf_in = {5'b0, x_min, 5'b0, x_max};	//Red
			msg_buf_wr = 1'b1;
		end
		2'b11: begin
			msg_buf_in = {32'b0};
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