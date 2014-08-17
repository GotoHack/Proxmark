//-----------------------------------------------------------------------------
// Test bench for the FPGA stuff.
//
// Jonathan Westhues, March 2006
//-----------------------------------------------------------------------------

module fpga_tb;

    reg spck, mosi, ncs;
    wire miso;
    reg pck0, ck_1356meg, ck_1356megb;
    wire pwr_lo, pwr_hi, pwr_oe1, pwr_oe2, pwr_oe3, pwr_oe4;
    reg [7:0] adc_d;
    wire adc_clk, adc_noe;
    reg ssp_dout;
    wire ssp_frame, ssp_din, ssp_clk;

    fpga dut(
        spck, miso, mosi, ncs,
        pck0, ck_1356meg, ck_1356megb,
        pwr_lo, pwr_hi, pwr_oe1, pwr_oe2, pwr_oe3, pwr_oe4,
        adc_d, adc_clk, adc_noe,
        ssp_frame, ssp_din, ssp_dout, ssp_clk
    );

    initial
        begin
            # 0 adc_d = 8'h9d;
        end

    initial
        begin
            # 0 spck = 0; mosi = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 10 spck = 1; # 10 spck = 0;
            # 5 spck = 0;
            # 5 mosi = 1;                   // conf_word[7] = 1 (lo-freq)
            # 5 spck = 1; # 10 spck = 0;
            # 5 mosi = 1;                   // conf_word[6] = 1 (carrier on)
            # 5 spck = 1; # 10 spck = 0;
            # 5 mosi = 1;                   // conf_word[5] = 1 (125 kHz)
            # 5 spck = 1; # 10 spck = 0;
            # 5 mosi = 0;                   // conf_word[4] = 0
            # 5 spck = 1; # 10 spck = 0;
            # 5 mosi = 0;                   // conf_word[3] = 0
            # 5 spck = 1; # 10 spck = 0;
            # 5 mosi = 0;                   // conf_word[2] = 0
            # 5 spck = 1; # 10 spck = 0;
            # 5 mosi = 0;                   // conf_word[1] = 0
            # 5 spck = 1; # 10 spck = 0;
            # 5 mosi = 0;                   // conf_word[0] = 0
            # 5 spck = 1; # 10 spck = 0;
            // and leave spck = 0 forever

            pck0 = 0;
            forever # 10 pck0 = !pck0;
        end

endmodule
