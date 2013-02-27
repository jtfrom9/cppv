/*verilator lint_off STMDLY*/
module top;
   reg clk;
   integer cycle;
   
   reg     vonly;

   integer count;
   
   initial begin
      clk = 0;
   end

   always #10 clk = ~clk;

   always@(posedge clk) begin
      count = count + 1;
      //if(count % 1000000==0)
        //$display($time, "  count=%0d", count);
   end

   initial begin
      wait($unsigned(count) >= $unsigned(cycle));
      $finish;
   end
   
endmodule



