
`timescale 1 ns / 1 ps

	module axixl_v1_0 #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface s_axi
		parameter integer C_s_axi_DATA_WIDTH	= 32,
		parameter integer C_s_axi_ADDR_WIDTH	= 8,

		// Parameters of Axi Master Bus Interface m_axi
		parameter integer C_m_axi_ADDR_WIDTH	= 32,
		parameter integer C_m_axi_DATA_WIDTH	= 32
	)
	(
		// Users to add ports here

		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface s_axi
		input wire  s_axi_aclk,
		input wire  s_axi_aresetn,
		input wire [C_s_axi_ADDR_WIDTH-1 : 0] s_axi_awaddr,
		input wire [2 : 0] s_axi_awprot,
		input wire  s_axi_awvalid,
		output wire  s_axi_awready,
		input wire [C_s_axi_DATA_WIDTH-1 : 0] s_axi_wdata,
		input wire [(C_s_axi_DATA_WIDTH/8)-1 : 0] s_axi_wstrb,
		input wire  s_axi_wvalid,
		output wire  s_axi_wready,
		output wire [1 : 0] s_axi_bresp,
		output wire  s_axi_bvalid,
		input wire  s_axi_bready,
		input wire [C_s_axi_ADDR_WIDTH-1 : 0] s_axi_araddr,
		input wire [2 : 0] s_axi_arprot,
		input wire  s_axi_arvalid,
		output wire  s_axi_arready,
		output wire [C_s_axi_DATA_WIDTH-1 : 0] s_axi_rdata,
		output wire [1 : 0] s_axi_rresp,
		output wire  s_axi_rvalid,
		input wire  s_axi_rready,

		// Ports of Axi Master Bus Interface m_axi
		input wire  m_axi_aclk,
		input wire  m_axi_aresetn,
		output wire [C_m_axi_ADDR_WIDTH-1 : 0] m_axi_awaddr,
		output wire [2 : 0] m_axi_awprot,
		output wire  m_axi_awvalid,
		input wire  m_axi_awready,
		output wire [C_m_axi_DATA_WIDTH-1 : 0] m_axi_wdata,
		output wire [C_m_axi_DATA_WIDTH/8-1 : 0] m_axi_wstrb,
		output wire  m_axi_wvalid,
		input wire  m_axi_wready,
		input wire [1 : 0] m_axi_bresp,
		input wire  m_axi_bvalid,
		output wire  m_axi_bready,
		output wire [C_m_axi_ADDR_WIDTH-1 : 0] m_axi_araddr,
		output wire [2 : 0] m_axi_arprot,
		output wire  m_axi_arvalid,
		input wire  m_axi_arready,
		input wire [C_m_axi_DATA_WIDTH-1 : 0] m_axi_rdata,
		input wire [1 : 0] m_axi_rresp,
		input wire  m_axi_rvalid,
		output wire  m_axi_rready
	);
		
	// Instantiation of Axi Bus Interface s_axi
	axixl_v1_0_s_axi # ( 
		.C_S_AXI_DATA_WIDTH(C_s_axi_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_s_axi_ADDR_WIDTH)
	) axixl_v1_0_s_axi_inst (
		.S_AXI_ACLK(s_axi_aclk),
		.S_AXI_ARESETN(s_axi_aresetn),
		.S_AXI_AWADDR(s_axi_awaddr),
		.S_AXI_AWPROT(s_axi_awprot),
		.S_AXI_AWVALID(s_axi_awvalid),
		.S_AXI_AWREADY(s_axi_awready),
		.S_AXI_WDATA(s_axi_wdata),
		.S_AXI_WSTRB(s_axi_wstrb),
		.S_AXI_WVALID(s_axi_wvalid),
		.S_AXI_WREADY(s_axi_wready),
		.S_AXI_BRESP(s_axi_bresp),
		.S_AXI_BVALID(s_axi_bvalid),
		.S_AXI_BREADY(s_axi_bready),
		.S_AXI_ARADDR(s_axi_araddr),
		.S_AXI_ARPROT(s_axi_arprot),
		.S_AXI_ARVALID(s_axi_arvalid),
		.S_AXI_ARREADY(s_axi_arready),
		.S_AXI_RDATA(s_axi_rdata),
		.S_AXI_RRESP(s_axi_rresp),
		.S_AXI_RVALID(s_axi_rvalid),
		.S_AXI_RREADY(s_axi_rready)
	);

	
	// Add user logic here
	wire 									write_en;				// Enable a write operation to DDR
	wire	[C_m_axi_ADDR_WIDTH-1:0]		write_addr;				// Address to write to
	wire	[C_m_axi_DATA_WIDTH-1:0]		write_data;				// Data to write
	wire									write_complete;			// Write has completed
	
	wire 									read_en;				// Enable a read operation from DDR
	wire	[C_m_axi_ADDR_WIDTH-1:0]		read_addr;				// Address to read from
	wire	[C_m_axi_DATA_WIDTH-1:0]		read_data;				// Where to read to
	wire									read_complete;			// Read has completed
	
	// User logic ends

// Instantiation of Axi Bus Interface m_axi
	axixl_v1_0_m_axi # ( 
		.C_M_AXI_ADDR_WIDTH(C_m_axi_ADDR_WIDTH),
		.C_M_AXI_DATA_WIDTH(C_m_axi_DATA_WIDTH)
	) axixl_v1_0_m_axi_inst (
		.writeEnable(write_en),
		.opAddress(write_en ? write_addr : read_addr),
		.writeData(write_data),
		.readData(read_data),
		.writeComplete(write_complete),
		.readEnable(read_en),
		.readComplete(read_complete),
		.M_AXI_ACLK(m_axi_aclk),
		.M_AXI_ARESETN(m_axi_aresetn),
		.M_AXI_AWADDR(m_axi_awaddr),
		.M_AXI_AWPROT(m_axi_awprot),
		.M_AXI_AWVALID(m_axi_awvalid),
		.M_AXI_AWREADY(m_axi_awready),
		.M_AXI_WDATA(m_axi_wdata),
		.M_AXI_WSTRB(m_axi_wstrb),
		.M_AXI_WVALID(m_axi_wvalid),
		.M_AXI_WREADY(m_axi_wready),
		.M_AXI_BRESP(m_axi_bresp),
		.M_AXI_BVALID(m_axi_bvalid),
		.M_AXI_BREADY(m_axi_bready),
		.M_AXI_ARADDR(m_axi_araddr),
		.M_AXI_ARPROT(m_axi_arprot),
		.M_AXI_ARVALID(m_axi_arvalid),
		.M_AXI_ARREADY(m_axi_arready),
		.M_AXI_RDATA(m_axi_rdata),
		.M_AXI_RRESP(m_axi_rresp),
		.M_AXI_RVALID(m_axi_rvalid),
		.M_AXI_RREADY(m_axi_rready)
	);


	endmodule
