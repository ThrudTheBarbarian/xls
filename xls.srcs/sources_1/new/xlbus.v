`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/06/2019 12:18:57 PM
// Design Name: 
// Module Name: xlbus
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
//
// Register map
//  $0	32-bit control register
//  $4	2x16-bit values for start/end of 8-bit space to take over
//  $8	32-bit value of address in DDR to provide data from
// 	$C	32-bit status register
//
//  repeat 16 times, once for each of 16 spaces, registers end at $FC
//
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////
`include "include.v"


module xlbus
	(
    input					clk,				// Module clock
    input					rst_n,				// Module reset
    
    input					a8_clk,				// Atari clock
    input 		[15:0]		a8_addr,			// Atari address bus
    input		[7:0]		a8_data,			// Atari data bus
    input					a8_rw,				// read (1) or write (0) cycle
    input					a8_halt,			// Antic has the bus
    input					a8_refresh,			// Access is a refresh cycle
    
    output reg				a8_rst_n,			// Active-low reset
    output reg				a8_extsel_n,		// External decoding select
   	output reg				a8_mpd,				// Math Pack Disable
   	output reg				a8_irq,				// Interrupt request
   	
   	output					gen_cctl_n,			// Generated, address is $D5xx
   	output					gen_d1xx_n,			// generated, address is $D1xx
   	output					gen_s4_n,			// Generated, address is $8000-$9FFF
   	output					gen_s5_n			// Generated, address is $A000-$BFFF
    );
    
    ////////////////////////////////////////////////////////////////////////////
    // Handle cartidge flags etc via continuous assignment
    ////////////////////////////////////////////////////////////////////////////
    wire hi					= a8_addr[15:8];
    
    assign gen_cctl_n		= (hi == 8'hD5) 					? 1'b0 : 1'b1;
    assign gen_d1xx_n		= (hi == 8'hD1)						? 1'b0 : 1'b1;
    assign gen_s4_n			= (hi >= 8'h80) && (hi <= 8'h9F)	? 1'b0 : 1'b1;
    assign gen_s5_n			= (hi >= 8'hA0) && (hi <= 8'hBF)	? 1'b0 : 1'b1;
    
   	////////////////////////////////////////////////////////////////////////////
	// State handling.
	////////////////////////////////////////////////////////////////////////////
	reg		[`BUS_STATEW:0]		busState;			// Current bus state
 	reg							lastClock;			// Last A8 clock value
 	reg		[`CYCLECOUNT-1:0]	cycleCount;			// Cycles into bus access
 	
    ////////////////////////////////////////////////////////////////////////////
    // Start the state machine
    ////////////////////////////////////////////////////////////////////////////
    always @ (posedge(clk))
    	begin
    		if (rst_n == 1'b0)
    			begin
    				busState		<= `RESET;
    				lastClock		<= 1'b0;
    			end
    		else
    			begin
    				case (busState)
 						////////////////////////////////////////////////////////
						// RESET is the starting state
						////////////////////////////////////////////////////////
						`RESET:
							begin
								busState			<= `WAIT_A8_CLOCK_LOW;
								lastClock			<= a8_clk;
							end
						
 						////////////////////////////////////////////////////////
						// Idle here, waiting for the clock to transition low
						////////////////////////////////////////////////////////
						`WAIT_A8_CLOCK_LOW:
							begin
								cycleCount			<= `CYCLECOUNT'h0;
								if (lastClock == 1'b1 && a8_clk == 1'b0)
									begin
										busState	<= `WAIT_VALID_ADDRESS;
									end
							end
						
 						////////////////////////////////////////////////////////
						// Wait for the address lines to be valid on the bus
						////////////////////////////////////////////////////////
						`WAIT_VALID_ADDRESS:
							begin
								cycleCount			<= cycleCount + 1;
								if (cycleCount == `CYCLES_ADDRVALID)
									begin
										busState	<= `EXTERNAL_ACCESS_CHECK;
									end
							end
							
 						////////////////////////////////////////////////////////
						// Determine if we want to assert /MPD and/or /EXTSEL
						////////////////////////////////////////////////////////
						`EXTERNAL_ACCESS_CHECK:
							begin
								cycleCount			<= cycleCount + 1;
								
							end
					endcase	
    			end
    	end
    	
    
endmodule
