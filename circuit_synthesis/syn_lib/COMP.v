`timescale 1ns / 1ps

module COMP
#(
	parameter N=8
)
(
	input [N-1:0] A,
	input [N-1:0] B,
	output 	O
);

	wire C[N:0];

	assign C[0] = 1'b1;
	assign O = C[N];
	
	genvar g, h;
	
	localparam MAX_LOOP = 512;
		
	generate
	if(N < MAX_LOOP)
	begin
		for(g=0;g<N;g=g+1)
		begin:FAINST
			FA FA_ (
				.A(A[g]), 
				.B(~B[g]), 
				.CI(C[g]), 
				.S(), 
				.CO(C[g+1])
			);
		end
	end
	else
	begin
		for(h=0;h<N/MAX_LOOP;h=h+1)
		begin:FA_INST_0
			for(g=0;g<MAX_LOOP;g=g+1)
			begin:FA_INST_1
				FA FA_ (
					.A(A[h*MAX_LOOP + g]), 
					.B(~B[h*MAX_LOOP + g]), 
					.CI(C[h*MAX_LOOP + g]), 
					.S(), 
					.CO(C[h*MAX_LOOP + g +1])
				);
			end
		end
		for(g=(N/MAX_LOOP)*MAX_LOOP;g <N;g=g+1)
		begin:FA_INST_1
			FA FA_ (
				.A(A[g]), 
				.B(~B[g]), 
				.CI(C[g]), 
				.S(), 
				.CO(C[g+1])
			);
		end
	end
	endgenerate

endmodule

