module processing (
	input wire clk,
	input wire [7:0] red,
	input wire [7:0] green,
	input wire [7:0] blue,
	input wire [7:0] grey,
	input wire sop,
	input wire packet_video,
	input wire in_valid,
	input wire x,
	input wire y,
	output wire [7:0] red_processed,
	output wire [7:0] green_processed,
	output wire [7:0] blue_processed
);

reg signed [23:0] sobel_x = {8'd1, 8'd0, -8'd1};   //[1, 0, -1]

wire signed [15:0] conv_x;
wire signed [15:0] edge_x;

reg [7:0] edge_buff [2:0];

integer i;
always@(posedge clk) begin
	for (i=2; i>0; i=i-1) begin   //delay line
		edge_buff[i] <= edge_buff[i-1];
	end
	edge_buff[0] <= grey;
end

assign conv_x = sobel_x[0]*edge_buff[0] + sobel_x[1]*edge_buff[1] + sobel_x[2]*edge_buff[2];

always_comb begin
	if (conv_x[15] == 1) edge_x = -conv_x;
	else edge_x = conv_x;
end



assign red_processed = edge_x[7:0]; //first 8 bits
assign green_processed = edge_x[7:0];
assign blue_processed = edge_x[7:0];

endmodule