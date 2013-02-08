module skel();
   reg [15:0] A;
   reg [15:0] B;
   wire [15:0] C;
   reg clk;
   
   //wire [15:0] C;
   //test t( .A(16'h0), .B(16'h0), .C(), .clk(clk));
   test t( A, B, C, clk );

   // initial begin
   //    $load_veriepy_script;
   // end

   initial begin
      $dumpvars;
      //$vcdpluson;
   end
        
endmodule

module test(
            A,
            B,
            C,
            clk);
   
   input [15:0] A;
   input [15:0] B;
   output [15:0] C;
   input        clk;
   
   reg [15:0]    C;
   always @(A or B) begin
      C <= A + B;
   end

   always @(posedge clk)
     $display($time, ": clk posedge");
endmodule


/*
module top;
   reg [15:0] A;
   reg [15:0] B;
   wire [15:0] C;
   reg        clk;
   
   test t( .A(A), .B(B), .C(C), .clk(clk) );
   //test t( .* );

   integer    i;

   initial begin
      clk = 0;
      A = 0; B = 0;

      #100;
      A <= 1; B <= 0;

      #100;
      A = 10; B = 20;

      #100;
      A = 100; B = 300;

      #100;
      $finish;
   end
   
   always #20 clk = ~clk;

   always @(C) 
     $display($time, ": A=%0d, B=%0d, C=%0d",A,B,C);

endmodule

module hoge();
   initial begin
      $display("hello, test.v");
   end
endmodule

     
*/	
