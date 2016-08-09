/* Top level module for keypad + UART demo */
module top (
    // input hardware clock (12 MHz)
    input wire hwclk,
    // all LEDs
    output wire led1,
    output wire led2,
    output wire led3,
    output wire led4,
    // UART lines
    output wire tx, 
    input  wire rx
    );

    // input and output to be communicated
    reg [15:0] vinput=16'b0;  // input and output are reserved keywords
    reg [23:0] voutput=24'b0;

    // UART registers
    wire [7:0] uart_rxbyte;
    reg  [7:0] uart_txbyte=8'b0;
    reg uart_send    = 1'b1; // do not send anything by default
    reg uart_receive = 1'b1; // listen
    wire uart_txed;
    wire uart_rxed;

    // LED register
    reg ledval1 = 0;
    reg ledval2 = 0;
    reg ledval3 = 0;
    reg ledval4 = 0;


   // wire[1:0] state;

/*
    assign led3 = ~state[1];
    assign led4 = ~state[0];
*/

    // 9600 Hz clock generation (from 12 MHz)
    parameter period_9600 = 32'd625;
    wire clk_9600;
    reg  clk_9600_reset=0;
    uart_clock clock_9600 (
        .hwclk(hwclk),
        .reset(clk_9600_reset),
        .period(period_9600),
        .clk(clk_9600)
    );

    // UART receiver module designed for
    // 8 bits, no parity, 1 stop bit. 
    uart_rx_8n1 receiver (
        .clk (clk_9600),          // 9600 baud rate clock
        .rx (rx),                 // input UART rx pin
        .recvdata (uart_receive), // allow any incoming bytes
        .rxbyte (uart_rxbyte),    // byte received
        .rxdone (uart_rxed)      // input: rx is finished
    );

    // UART transmitter module designed for
    // 8 bits, no parity, 1 stop bit. 
    uart_tx_8n1 transmitter (
        .clk (clk_9600),       // 9600 baud rate clock
        .tx (tx),              // output UART tx pin
        //.senddata (uart_rxed), // trigger a UART transmit on baud clock
        //.senddata (1'b1), // trigger a UART transmit on baud clock
        .senddata (uart_send), // trigger a UART transmit on baud clock
        .txbyte (uart_txbyte), // byte to be transmitted
        //.txbyte (8'd70), // byte to be transmitted
        .txdone (uart_txed)    // input: tx is finished
    );

    reg [1:0] bytecount=2'b0;

    parameter STATE_RECEIVING   = 2'b01;
    parameter STATE_CALCULATING = 2'b10;
    parameter STATE_SENDING     = 2'b11;

    parameter write_A      = 2'b00;
    parameter write_B      = 2'b01;
    parameter write_AplusB = 2'b10;

    reg state=STATE_RECEIVING;

    always @(posedge clk_9600) begin

        case (state) 

        STATE_RECEIVING: begin
           uart_send <= 0;
           uart_receive <= 1;
           if(uart_rxed) begin
              case (bytecount)
              2'd0:  begin
//             led3=1;
                      vinput[15:8]<=uart_rxbyte;
              end

              2'd1: begin
//              led4=1;
                      vinput[15:8]<=uart_rxbyte;
                      //state<=STATE_CALCULATING;
                      state=STATE_SENDING;
                      uart_receive <= 0;
                      // well, the computation could start here already
              end

              default: begin
                      // should not be reached
                      //state=STATE_CALCULATING;
                      state=STATE_SENDING;
              end
              endcase
              bytecount = bytecount+1;
           led3=0;
           led4=0;
           end
        end

        STATE_CALCULATING: begin
           bytecount <= write_A;
           uart_receive <= 0;
           voutput=vinput[15:8]+vinput[7:0];
           voutput[23:16]=vinput;
           state = STATE_SENDING;
           led3=1;
        end


        STATE_SENDING: begin
           led4=1;

           uart_receive <= 0;
           if (uart_txed) begin

            case (bytecount)

            write_A: begin
               uart_send   <= 1;
               bytecount   <= write_B;
               uart_txbyte <= voutput[23:16];
            end

            write_B: begin
               uart_send   <= 1;
               bytecount   <= write_AplusB;
               uart_txbyte <= voutput[15:8];
            end

            write_AplusB: begin
               uart_send   <= 1;
               bytecount   <= 2'd0;
               uart_txbyte <= voutput[7:0];
               state     = STATE_RECEIVING;
            end

            default: begin
               uart_send <= 0;
               bytecount <= 2'd0;
               state     = STATE_RECEIVING;
            end

            endcase

           end
           //else begin
           //    led4=1;
           //end

        end

        default: begin
            led4=1;
        end

        endcase

    end

    // Wiring
    assign led1=state[0];
    assign led2=state[1];
    //assign led3=bytecount[0];
    //assign led4=bytecount[1];

endmodule
