module top();
   reg [15:0] A;
   reg [15:0] B;
   wire [15:0] C;
   reg clk;
   wire clko;

   test t( A, B, C, clk, clko );

   initial begin
      $dumpvars;
      //$vcdpluson;
   end
endmodule

module test(
            A,
            B,
            C,
            clk,
            clko);

   input [15:0] A;
   input [15:0] B;
   output [15:0] C;
   input         clk;
   output       clko;

   reg [15:0]    C;
   always @(A or B) begin
      C <= A + B;
   end

   assign clko = ~clk;

   // always @* begin
   //   $display($time, ": clko=%0d",clko);
   // end

endmodule
