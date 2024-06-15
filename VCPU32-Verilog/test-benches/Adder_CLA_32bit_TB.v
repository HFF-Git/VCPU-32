
//------------------------------------------------------------------------------------------------------------
//
// Adder_CLA_32bit - Testbench
//
//------------------------------------------------------------------------------------------------------------
// This module is the test bench for the 32-bit carry lookahead adder.
//
//------------------------------------------------------------------------------------------------------------
//
// Adder_CLA_32bit - Testbench
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
`include "../hdl/VCPU32.v"

`timescale 10ns / 1ns

module Adder_CLA_32_TB;

	reg[0:`WORD_LENGTH-1] 	A_TB 	= 0;
	reg[0:`WORD_LENGTH-1] 	B_TB 	= 0;
	reg						C_IN_TB = 0;

	wire[0:`WORD_LENGTH-1]	S_TB;
	wire 					C_TB;

	task setupTest;

		begin

		$dumpfile( "Adder_CLA_32bit_TB.vcd" );
   		$dumpvars( 0, Adder_CLA_32bit_TB );

		end

	endtask

	task applyTest ( 

		input [0:`WORD_LENGTH-1] 	a, 
		input [0:`WORD_LENGTH-1] 	b,
		input 		 				cIn

		);

		begin

			A_TB 	= a;
			B_TB 	= b;
			C_IN_TB = cIn;
			#10 $display( "A: 0x%h, B: 0x%h, cIn: %d -> S: 0x%h, C: %d", A_TB, B_TB, C_IN_TB, S_TB, C_TB );
	
		end

	endtask
	
	Adder_CLA_32bit DUT ( .a( A_TB ), .b( B_TB ), .inC( C_IN_TB ), .s( S_TB ), .outC( C_TB ));

	initial begin

 		setupTest( );

		applyTest( 32'd0,  			32'd0, 	0 );
		applyTest( 32'd10, 			32'd5, 	0 );
		applyTest( 32'd1, 			32'd15, 0 );
		applyTest( 32'hFFFFFFFF, 	32'd1, 	0 );
		
   		#10 $finish;
		
	end

endmodule
