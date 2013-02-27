
module test(clk, count);
   input clk;
   output [31:0] count;
   // initial begin
   //    $display("Hello World"); 
   //    $finish; 
   // end

   reg [31:0]    count;

   reg           A /*verilator public_flat*/;
   
   initial begin
      count = 0;
   end
   
   always@(posedge clk) begin
      count <= count + 1;
      A <= count[1];
   end
   
endmodule


