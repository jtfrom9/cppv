module top;
   reg clk;
   reg n_rst;
   reg [15:0] addr;
   reg [15:0] dwrite;
   wire [15:0] dread;
   reg         req;
   reg         wen;
   wire        ack;
   
   mem m( clk, n_rst,
          addr, dwrite, dread, req, wen,
          ack );
endmodule

module mem(CLK, nRST,
           ADDR, DIN, DOUT, REQ, WEN,
           ACK);

   input CLK;
   input nRST;
   input [15:0] ADDR;
   input [15:0] DIN;
   output [15:0] DOUT;
   input         REQ;
   input         WEN;
   output        ACK;
   
   reg [15:0]    dout;
   reg           ack;
   
   assign DOUT = dout;
   assign ACK = ack;

   parameter memsize = ((16*1024)-1);
     
   reg [7:0]     mem[memsize:0];
   reg           state;

   integer       i;
   initial begin
      for(i=0; i<=memsize; i=i+1)
        mem[i] = 0;
   end
   
   always@(posedge CLK or nRST) begin
      if (~nRST) begin
         dout <= 0;
         ack <= 0;
         state <= 0;
      end
      else begin
         case(state)
           0: begin
              if(REQ) begin
                 if(WEN) 
                    { mem[ ADDR ], mem[ ADDR+1 ] } <= DIN;
                 else 
                   dout <= { mem[ADDR] , mem[ADDR+1] };
                 ack <= 1;
                 state <= 1;
              end
           end
           1: begin
              if(~REQ) begin
                 ack <= 0;
                 state <= 0;
              end
           end
         endcase
      end
   end
   
endmodule


