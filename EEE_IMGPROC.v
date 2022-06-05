module EEE_IMGPROC(
	// global clock & reset
	clk,
	reset_n,
	
	// mm slave
	s_chipselect,
	s_read,
	s_write,
	s_readdata,
	s_writedata,
	s_address,

	// stream sink
	sink_data,
	sink_valid,
	sink_ready,
	sink_sop,
	sink_eop,
	
	// streaming source
	source_data,
	source_valid,
	source_ready,
	source_sop,
	source_eop,
	
	// conduit
	mode
);


// global clock & reset
input	clk;
input	reset_n;

// mm slave
input							s_chipselect;
input							s_read;
input							s_write;
output	reg	[31:0]	s_readdata;
input	[31:0]				s_writedata;
input	[2:0]					s_address;

// SINK = INPUT
// streaming sink
input	[23:0]            	sink_data;
input								sink_valid;
output							sink_ready;
input								sink_sop;
input								sink_eop;

// SOURCE = OUTPUT
// streaming source
output	[23:0]			  	   source_data;
output								source_valid;
input									source_ready;
output								source_sop;
output								source_eop;

// conduit export
input                         mode;

////////////////////////////////////////////////////////////////////////
//
parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 256;
parameter MSG_INTERVAL = 6;
parameter BB_COL_DEFAULT = 24'h00ff00;


wire [7:0]   red, green, blue, grey;
wire [7:0]   red_out, green_out, blue_out;
reg [55:0] 	r_ring, g_ring, b_ring; //(previous values + 1) * 8
reg [2:0]	count_input;
wire [7:0] 	r_blur_x, g_blur_x, b_blur_x;
wire [13:0]	r_sum_x, g_sum_x, b_sum_x;
// wire signed [8:0] r_diff_x, g_diff_x, b_diff_x; //adding new pixel, subtracting old pixel

wire         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////

// Detect red areas
wire red_detect;
assign red_detect = red[7] & ~green[7] & ~blue[7];
//wire [7:0] white;
//assign white = (green + blue);
//assign red_detect = (red - white) > 0;


//buffer values
always@(posedge clk) begin	
	if (sop || x==0) begin			//set buffer to 0
		r_ring <= 56'h0;
		g_ring <= 56'h0;
		b_ring <= 56'h0;
		count_input <= 0;
	end
	else if (packet_video) begin
		if (count_input == 6) count_input <= 0;	// % 7
		else count_input <= count_input + 1;
		r_ring[count_input*8+7 -:8] <= red;		//store new pixel
		g_ring[count_input*8+7 -:8] <= green;
		b_ring[count_input*8+7 -:8] <= blue;
	end
end

reg [34:0] gauss = {5'd1, 5'd6, 5'd15, 5'd20, 5'd15, 5'd6, 5'd1};	//gaussian kernel

//convolution count_input+(offset+1) % 7 (for wraparound), but 8 bits for each
assign r_sum_x = gauss[4:0]*r_ring[(count_input*8+7) % 56 -:8] + gauss[9:5]*r_ring[(count_input*8+15) % 56 -:8] + gauss[14:10]*r_ring[(count_input*8+23) % 56 -:8] + gauss[19:15]*r_ring[(count_input*8+31) % 56 -:8] + gauss[24:20]*r_ring[(count_input*8+39) % 56 -:8] + gauss[29:25]*r_ring[(count_input*8+47) % 56 -:8] + gauss[34:30]*r_ring[(count_input*8+55) % 56 -:8];
assign g_sum_x = gauss[4:0]*g_ring[(count_input*8+7) % 56 -:8] + gauss[9:5]*g_ring[(count_input*8+15) % 56 -:8] + gauss[14:10]*g_ring[(count_input*8+23) % 56 -:8] + gauss[19:15]*g_ring[(count_input*8+31) % 56 -:8] + gauss[24:20]*g_ring[(count_input*8+39) % 56 -:8] + gauss[29:25]*g_ring[(count_input*8+47) % 56 -:8] + gauss[34:30]*g_ring[(count_input*8+55) % 56 -:8];
assign b_sum_x = gauss[4:0]*b_ring[(count_input*8+7) % 56 -:8] + gauss[9:5]*b_ring[(count_input*8+15) % 56 -:8] + gauss[14:10]*b_ring[(count_input*8+23) % 56 -:8] + gauss[19:15]*b_ring[(count_input*8+31) % 56 -:8] + gauss[24:20]*b_ring[(count_input*8+39) % 56 -:8] + gauss[29:25]*b_ring[(count_input*8+47) % 56 -:8] + gauss[34:30]*b_ring[(count_input*8+55) % 56 -:8];


// Find boundary of cursor box

// Highlight detected areas
wire [23:0] red_high;
assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4
assign red_high  =  red_detect ? {8'hff, 8'h0, 8'h0} : {grey, grey, grey};

wire [11:0] r_blur, g_blur, b_blur;
wire [23:0] blur;
assign r_blur_x = r_sum_x / 64;	//normalise
assign g_blur_x = g_sum_x / 64;
assign b_blur_x = b_sum_x / 64;
assign r_blur = r_blur_x;
assign g_blur = g_blur_x;
assign b_blur = b_blur_x;

assign blur = {r_blur[7:0], g_blur[7:0], b_blur[7:0]};

// Show bounding box
wire [23:0] new_image;
wire bb_active;
assign bb_active = (x == left) | (x == right) | (y == top) | (y == bottom);
//assign new_image = bb_active ? bb_col : red_high;
assign new_image = blur;

// Switch output pixels depending on mode switch
// Don't modify the start-of-packet word - it's a packet discriptor
// Don't modify data in non-video packets
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image : {red,green,blue};

//Count valid pixels to tget the image coordinates. Reset and detect packet type on Start of Packet.
reg [10:0] x, y;
reg packet_video;
always@(posedge clk) begin
	if (sop) begin
		x <= 11'h0;
		y <= 11'h0;
		packet_video <= (blue[3:0] == 3'h0);
	end
	else if (in_valid) begin		// This scans each row. Increments row when x is at the final pixel.
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
reg [10:0] x_min, y_min, x_max, y_max;
always@(posedge clk) begin
	if (red_detect & in_valid) begin	//Update bounds when the pixel is red
		if (x < x_min) x_min <= x;
		if (x > x_max) x_max <= x;
		if (y < y_min) y_min <= y;
		y_max <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		x_min <= IMAGE_W-11'h1;
		x_max <= 0;
		y_min <= IMAGE_H-11'h1;
		y_max <= 0;
	end
end

//Process bounding box at the end of the frame.
reg [1:0] msg_state;
reg [10:0] left, right, top, bottom;
reg [7:0] frame_count;
always@(posedge clk) begin						
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//Latch edges for display overlay on next frame
		left <= x_min;
		right <= x_max;
		top <= y_min;
		bottom <= y_max;
		
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin // MESSAGE_BUF_MAX = 256 (constant)
			msg_state <= 2'b01;
			frame_count <= MSG_INTERVAL-1;	//MSG_INTERVAL = 6 (constant)
		end
	end
	
	//Cycle through message writer states once started
	if (msg_state != 2'b00) msg_state <= msg_state + 2'b01;

end
	
//Generate output messages for CPU
reg [31:0] msg_buf_in; 
wire [31:0] msg_buf_out;
reg msg_buf_wr;
wire msg_buf_rd, msg_buf_flush;
wire [7:0] msg_buf_size;
wire msg_buf_empty;

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
			msg_buf_in = {5'b0, x_min, 5'b0, y_min};	//Top left coordinate
			msg_buf_wr = 1'b1;
		end
		2'b11: begin
			msg_buf_in = {5'b0, x_max, 5'b0, y_max}; //Bottom right coordinate
			msg_buf_wr = 1'b1;
		end
	endcase
end


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
STREAM_REG #(.DATA_WIDTH(26)) in_reg ( //Preprocessed image
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(sink_ready),
	.valid_out(in_valid),
	.data_out({red,green,blue,sop,eop}),
	.ready_in(out_ready),
	.valid_in(sink_valid),
	.data_in({sink_data,sink_sop,sink_eop})
);



STREAM_REG #(.DATA_WIDTH(26)) out_reg ( //Postprocessed image
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

reg  [7:0]   reg_status;
reg	[23:0]	bb_col;

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
		   if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
		end
	end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);


// Process reads
reg read_d; //Store the read signal for correct updating of the message buffer

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

