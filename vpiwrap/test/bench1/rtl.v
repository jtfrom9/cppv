module top;
   reg clk;
   integer cycle;
   
   reg     vonly;

   integer count;
   
   initial begin
      count = 0;
      
      if(! $value$plusargs("cycle=%d", cycle))
        cycle = 1 << 22;

      $display("cycle = %0d",$unsigned(cycle));
      
      if ($test$plusargs("vclkgen")) begin
         clk = 0;
         fork: clkgen
            forever begin
               #10 clk = ~clk;
            end
         join
         //$finish;
      end
   end

   always@(posedge clk) begin
      count = count + 1;
      if(count % 1000000==0)
        $display($time, "  count=%0d", count);
   end

   initial begin
      wait($unsigned(count) >= $unsigned(cycle));
      $finish;
   end
   
endmodule



