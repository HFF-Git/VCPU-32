//------------------------------------------------------------------------------------------------------------
//
// CPU24 - Testbench
//
//------------------------------------------------------------------------------------------------------------
// This module is the test bench for the CPU24 verilog file. It is primarily used to have a way to compile 
// the CPU file.
//
//------------------------------------------------------------------------------------------------------------
//
// CPU24 - Testbench
// Copyright (C) 2022 - 2024 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation, either version 3 of the License,
// or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details. You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------------------------------------
`include "../hdl/CPU24.v"

`timescale 10ns / 1ns

module CPU24_TB;

	reg 	CLK_TB 	= 1;
	reg     RST_TB 	= 0;

	task setupTest;

		begin

		$dumpfile( "CPU24_TB.vcd" );
   		$dumpvars( 0, CPU24_TB );

		end

	endtask

	task resetCpu;

		begin

				RST_TB = 1'b0;
			#2 	RST_TB = 1'b1;

		end

	endtask

	task applyTest;

		begin
	
		end

	endtask

	CPU24 DUT ( .clk( CLK_TB ), .rst( RST_TB ));

	always begin
		
		#1 CLK_TB = ~ CLK_TB;
	end

	initial begin

		setupTest( );
		resetCpu( );

   		#20 $finish;
		
	end

endmodule
