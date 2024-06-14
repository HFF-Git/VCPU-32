//------------------------------------------------------------------------------------------------------------
//
// Double shift right 48bit -- Testbench
//
//------------------------------------------------------------------------------------------------------------
// The double shift right test bench. We are passed two words, concatenate them and shift right by the shift
// amount.
//
//------------------------------------------------------------------------------------------------------------
//
// Shift/Merge Decode Unit -- Testbench
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

`timescale 1ns/1ns

module DoubleShiftRight_48bit_TB;

   	reg[0:23]  	A_TB, B_TB;
   	reg[0:4]   	SA_TB;
   	wire[0:23] 	Y_TB;

	task setupTest;

		begin

		$dumpfile( "DoubleShiftRight_48bit_TB.vcd" );
   		$dumpvars( 0, DoubleShiftRight_48bit_TB );

		end

	endtask

	task applyTest ( );

		begin
	
		end

	endtask

	DoubleShiftRight_48bit DUT ( .a( A_TB ), .b( B_TB ), .sa( SA_TB ), .y( Y_TB ));
		
	initial begin

 		setupTest( );
   		
   		A_TB 	= 24'h00FF0F;
		B_TB 	= 24'h000FFF;
		SA_TB 	= 5;
 		#10 $display( "A: 0x%h, B: 0x%h, SA: %d -> Y: 0x%h", A_TB, B_TB, SA_TB, Y_TB );	

   		#50 $finish;
		
	end

endmodule
