library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use IEEE.std_logic_unsigned.ALL;

entity top is
	port (
		-- Clocks
		clock_50_i			: in    std_logic;

		-- Buttons
		--btn_n_i				: in    std_logic_vector(4 downto 1);

		-- SRAMs (AS7C34096)
		--sram_addr_o			: out   std_logic_vector(18 downto 0)	:= (others => '0');
		--sram_data_io		: inout std_logic_vector(7 downto 0)	:= (others => 'Z');
		--sram_we_n_o			: out   std_logic								:= '1';
		--sram_oe_n_o			: out   std_logic								:= '1';
		
		-- SDRAM	(H57V256)
		SDRAM_A				: out std_logic_vector(12 downto 0);
		SDRAM_DQ				: inout std_logic_vector(15 downto 0);

		SDRAM_BA				: out std_logic_vector(1 downto 0);
		SDRAM_DQMH			: out std_logic;
		SDRAM_DQML			: out std_logic;	

		SDRAM_nRAS			: out std_logic;
		SDRAM_nCAS			: out std_logic;
		SDRAM_CKE			: out std_logic;
		SDRAM_CLK			: out std_logic;
		SDRAM_nCS			: out std_logic;
		SDRAM_nWE			: out std_logic;
	
		-- PS2
		ps2_clk_io			: inout std_logic								:= 'Z';
		ps2_data_io			: inout std_logic								:= 'Z';
		ps2_mouse_clk_io  : inout std_logic								:= 'Z';
		ps2_mouse_data_io : inout std_logic								:= 'Z';

		-- SD Card
		sd_cs_n_o			: out   std_logic								:= 'Z';
		sd_sclk_o			: out   std_logic								:= 'Z';
		sd_mosi_o			: out   std_logic								:= 'Z';
		sd_miso_i			: in    std_logic;

		-- Joysticks
      joy_load_n 	: out   std_logic;
      joy_clk		   : out   std_logic;
      joy_data  		: in    std_logic;
		joyX_p7_o		: out   std_logic								:= '1';

		-- Audio
		AUDIO_L				: out   std_logic								:= '0';
		AUDIO_R				: out   std_logic								:= '0';
		--ear_i					: in    std_logic;
		--mic_o					: out   std_logic								:= '0';

		-- VGA
		VGA_R					: out   std_logic_vector(5 downto 0)	:= (others => '0');
		VGA_G					: out   std_logic_vector(5 downto 0)	:= (others => '0');
		VGA_B					: out   std_logic_vector(5 downto 0)	:= (others => '0');
		VGA_HS				: out   std_logic								:= '1';
		VGA_VS				: out   std_logic								:= '1';

		-- HDMI
		--tmds_o				: out   std_logic_vector(7 downto 0)	:= (others => '0');

		--STM32
		stm_rx_o				: out std_logic		:= 'Z'; -- stm RX pin, so, is OUT on the slave
		stm_tx_i				: in  std_logic		:= 'Z'; -- stm TX pin, so, is IN on the slave
		-- 
		stm_rst_o			: out std_logic		:= 'Z'; -- '0' to hold the microcontroller reset line, to free the SD card
		
		--stm_a15_io			: inout std_logic;
		--stm_b8_io			: inout std_logic		:= 'Z';
		--stm_b9_io			: inout std_logic		:= 'Z';
		
		SPI_SCK				: inout std_logic		:= 'Z';
		SPI_DO				: inout std_logic		:= 'Z';
		SPI_DI				: inout std_logic		:= 'Z';
		SPI_SS2				: inout std_logic		:= 'Z'
	);
end entity;

architecture Behavior of top is

	type config_array is array(natural range 15 downto 0) of std_logic_vector(7 downto 0);

	function to_slv(s: string) return std_logic_vector is 
        constant ss: string(1 to s'length) := s; 
        variable answer: std_logic_vector(1 to 8 * s'length); 
        variable p: integer; 
        variable c: integer; 
    begin 
        for i in ss'range loop
            p := 8 * i;
            c := character'pos(ss(i));
            answer(p - 7 to p) := std_logic_vector(to_unsigned(c,8)); 
        end loop; 
        return answer; 
    end function; 

--	component vga is
--	port
--	(
--		-- pixel clock
--		pclk			: in std_logic;
--
--		-- enable/disable scanlines
--		scanlines	: in std_logic;
--		
--		-- output to VGA screen
--		hs	: out std_logic;
--		vs	: out std_logic;
--		r	: out std_logic_vector(3 downto 0);
--		g	: out std_logic_vector(3 downto 0);
--		b	: out std_logic_vector(3 downto 0);
--		blank : out std_logic
--		
--		--debug
--		--joy_i	: in std_logic_vector(11 downto 0)
--	);
--	end component;
	
	
	component joydecoder is
	port
	(		
    -------------------------------------------
    clk : in std_logic;   --si reloj de entrada en este caso 1.3888Mhz 
    joy_data  : in std_logic; --datos serializados patilla viene de la patilla 9 integrado
    joy_clk : out std_logic;  --va a patilla 11 integrado
    joy_load_n : out std_logic; --este reloj negado se usa directamente en las patillas 12 y 13
    -------------------------------------------
    joy1up  : out std_logic;
    joy1down : out std_logic;
    joy1left : out std_logic;
    joy1right : out std_logic;
    joy1fire1 : out std_logic;
    joy1fire2 : out std_logic;

    joy2up : out std_logic;
    joy2down : out std_logic;
    joy2left : out std_logic;
    joy2right : out std_logic;
    joy2fire1 : out std_logic;
    joy2fire2 : out std_logic
	);
	end component;
		
	component rgb_mist is
	port
	(
		-- pixel clock
		pclk			: in std_logic;

		-- enable/disable scanlines
		--scanlines	: in std_logic;
		white_noise	: in std_logic;
		-- output to VGA screen
		hs	: out std_logic;
		vs	: out std_logic;
		r	: out std_logic_vector(3 downto 0);
		g	: out std_logic_vector(3 downto 0);
		b	: out std_logic_vector(3 downto 0)
		--blank : out std_logic
		
		--debug
		--joy_i	: in std_logic_vector(11 downto 0)
	);
	end component;
	
	   component scandoubler is
	port
	(
		-- pixel clock
		clk_x2		: in std_logic;
		clk_pix		: in std_logic;
		
		scanlines	: in std_logic_vector(1 downto 0);
      scandoubler_disable : in std_logic;
		-- enable/disable scanlines 
		--scanlines	: in std_logic;
		
		-- output to VGA screen
		hs_in		: in std_logic;
		vs_in		: in std_logic;
		r_in		: in std_logic_vector(5 downto 0);
		g_in		: in std_logic_vector(5 downto 0);
		b_in		: in std_logic_vector(5 downto 0);
		
		hs_out	: out std_logic;
		vs_out	: out std_logic;
		r_out		: out std_logic_vector(5 downto 0);
		g_out		: out std_logic_vector(5 downto 0);
		b_out		: out std_logic_vector(5 downto 0)
		
	);
	end component;	

	
	component osd is
	generic
	(
		OSD_VISIBLE 	: std_logic_vector(1 downto 0) := (others=>'0');
		OSD_X_OFFSET 	: std_logic_vector(9 downto 0) := (others=>'0');
		OSD_Y_OFFSET 	: std_logic_vector(9 downto 0) := (others=>'0');
		OSD_COLOR    	: std_logic_vector(2 downto 0) := (others=>'0')
	);
	port
	(
		-- OSDs pixel clock, should be synchronous to cores pixel clock to
		-- avoid jitter.
		pclk		: in std_logic;

		-- SPI interface
		sck		: in std_logic;
		ss			: in std_logic;
		sdi		: in std_logic;
		sdo		: out std_logic;

		-- VGA signals coming from core
		red_in 	: in std_logic_vector(4 downto 0);
		green_in : in std_logic_vector(4 downto 0);
		blue_in 	: in std_logic_vector(4 downto 0);
		hs_in		: in std_logic;
		vs_in		: in std_logic;
		
		-- VGA signals going to video connector
		red_out	: out std_logic_vector(4 downto 0);
		green_out: out std_logic_vector(4 downto 0);
		blue_out	: out std_logic_vector(4 downto 0);
		hs_out 	: out std_logic;
		vs_out 	: out std_logic;
		
		-- Data in
		data_in 	: in std_logic_vector(7 downto 0);
		
		--data pump to sram
		pump_active_o	: out std_logic;
		sram_a_o			: out std_logic_vector(18 downto 0);
		sram_d_o			: out std_logic_vector(7 downto 0);
		sram_we_n_o		: out std_logic;
		config_buffer_o: out config_array
	
	);
	end component;
	


	-- clocks
	signal clk100				: std_logic;		
	signal clk100n				: std_logic;
	signal clock_10			: std_logic;	
	signal clock_20			: std_logic;
	signal pll_locked			: std_logic;	
	signal pixel_clock		: std_logic;		
	signal clk_dvi				: std_logic;		
	signal pMemClk				: std_logic;		
	signal clock_div_q		: unsigned(7 downto 0) 				:= (others => '0');	
--	signal sega_clk			: std_logic;
	
	-- Reset 
	signal reset_s				: std_logic;		-- Reset geral	
	signal power_on_s			: std_logic_vector(7 downto 0)	:= (others => '1');
	signal btn_reset_s		: std_logic;
		
	
	-- Video
	signal video_r_s				: std_logic_vector(3 downto 0)	:= (others => '0');
	signal video_g_s				: std_logic_vector(3 downto 0)	:= (others => '0');
	signal video_b_s				: std_logic_vector(3 downto 0)	:= (others => '0');
	signal video_hsync_n_s		: std_logic								:= '1';
	signal video_vsync_n_s		: std_logic								:= '1';
  	
	signal osd_r_s				: std_logic_vector(3 downto 0)	:= (others => '0');
	signal osd_g_s				: std_logic_vector(3 downto 0)	:= (others => '0');
	signal osd_b_s				: std_logic_vector(3 downto 0)	:= (others => '0');

	signal info_r_s				: std_logic_vector(3 downto 0)	:= (others => '0');
	signal info_g_s				: std_logic_vector(3 downto 0)	:= (others => '0');
	signal info_b_s				: std_logic_vector(3 downto 0)	:= (others => '0');
	
	-- VGA
	signal vga_r_s				: std_logic_vector( 3 downto 0);
	signal vga_g_s				: std_logic_vector( 3 downto 0);
	signal vga_b_s				: std_logic_vector( 3 downto 0);
	signal vga_hsync_n_s 		: std_logic;
	signal vga_vsync_n_s 		: std_logic;
	signal vga_blank_s 			: std_logic;
	signal scandoubler_disable : std_logic								:= '0';  -- 0 = HVSync 31KHz, 1 = CSync 15KHz
	signal white_noise_change  : std_logic								:= '0'; 
	signal white_noise			: std_logic								:= '0';  -- 0 = Not Activated, 1 = Activated

	signal vga_r_rgb				: std_logic_vector( 4 downto 0);
	signal vga_g_rgb				: std_logic_vector( 4 downto 0);
	signal vga_b_rgb				: std_logic_vector( 4 downto 0);
	signal vga_hsync_n_rgb 	   : std_logic;	
	signal vga_vsync_n_rgb 		: std_logic;
	signal cs 						: std_logic;
	signal hs 						: std_logic;
	signal vs 						: std_logic;
	
	signal hsync_n_o : std_logic; 

	
	-- Keyboard
	signal keys_s			: std_logic_vector( 7 downto 0) := (others => '1');	
	signal FKeys_s			: std_logic_vector(12 downto 1);
	
	-- joystick
	signal joy1_s			: std_logic_vector(15 downto 0) := (others => '1'); 
	signal joy2_s			: std_logic_vector(15 downto 0) := (others => '1'); 
	signal joyP7_s			: std_logic;
	
	--Neptuno
	signal joy1_up_i		: std_logic;
	signal joy1_down_i	: std_logic;
	signal joy1_left_i	: std_logic;
	signal joy1_right_i	: std_logic;
	signal joy1_p6_i		: std_logic;
	signal joy1_p9_i		: std_logic;
	signal joy2_up_i		: std_logic;
	signal joy2_down_i	: std_logic;
	signal joy2_left_i	: std_logic;
	signal joy2_right_i	: std_logic;
	signal joy2_p6_i		: std_logic;
	signal joy2_p9_i		: std_logic;		
	signal reset_stm_change : std_logic;
	
	-- config string
	constant STRLEN		: integer := 1;
--	constant CONF_STR		: std_logic_vector((STRLEN * 8)-1 downto 0) := to_slv("P,config.ini");
	constant CONF_STR		: std_logic_vector(7 downto 0) := X"00";
	
	signal config_buffer_s : config_array;
	
	-- keyboard
	signal kbd_intr      : std_logic;
	signal kbd_scancode  : std_logic_vector(7 downto 0);
		

	-- Teclas neptUNO
	
	signal fn_pulse : std_logic_vector(7 downto 0);		
	signal fn_toggle : std_logic_vector(7 downto 0);		
	
		
begin	
	btnscl: entity work.debounce
	generic map (
		counter_size	=> 16
	)
	port map (
		clk_i				=> pixel_clock,
		button_i			=> not fn_pulse(0), 
		result_o			=> btn_reset_s
	);
		
	process (pixel_clock)
	begin
	
		if rising_edge(pixel_clock) then
		
			if btn_reset_s = '0' then
				power_on_s <= (others=>'1');
			end if;
			
			if power_on_s /= x"00" then
				reset_s <= '1';
				stm_rst_o <= '0'; 
				power_on_s <= power_on_s - 1;
			else
				reset_s <= '0';
				stm_rst_o <= 'Z'; 
			end if;
			
		end if;
	end process;
  
	U00 : work.pll
	  port map(
		inclk0   => clock_50_i,              
		c0       => pixel_clock,             -- 25.200Mhz
		c1       => clock_10,             -- 10.000Mhz
		c2       => clock_20              -- 20.000Mhz
	  );

	--generate a black screen with proper sync VGA timing
--	vga1 : vga 
--	port map
--	(
--		pclk     => pixel_clock,
--
--		scanlines => '0',
--		
--		hs    	=> video_hsync_n_s,
--		vs    	=> video_vsync_n_s,
--		r     	=> video_r_s,
--		g     	=> video_g_s,
--		b     	=> video_b_s,
--		blank 	=> vga_blank_s
--		
--	);
	  
	vga1 : rgb_mist 
	port map
	(
		pclk     => clock_10,
		
		white_noise => white_noise,
				
		hs    	=> video_hsync_n_s,
		vs    	=> video_vsync_n_s,
		r     	=> video_r_s,
		g     	=> video_g_s,
		b     	=> video_b_s
	);


	  

	osd1 : osd 
	generic map
	(	
		--STRLEN => STRLEN,
		OSD_VISIBLE => "01",
		OSD_COLOR => "001", -- RGB
		OSD_X_OFFSET => "0000010010", -- 50
		OSD_Y_OFFSET => "0000001111"  -- 15
	)
	port map
	(
		pclk        => clock_10,

		-- spi for OSD
		sdi        => SPI_DI,
		sck        => SPI_SCK,
		ss         => SPI_SS2,
		sdo        => SPI_DO,
		
		red_in     => video_r_s & '0',
		green_in   => video_g_s & '0',
		blue_in    => video_b_s & '0',
		hs_in      => video_hsync_n_s,
		vs_in      => video_vsync_n_s,

		red_out(4 downto 1)    => osd_r_s,
		green_out(4 downto 1)  => osd_g_s,
		blue_out(4 downto 1)   => osd_b_s,
		hs_out     => vga_hsync_n_s,
		vs_out     => vga_vsync_n_s ,

		data_in		=> keys_s,
	--	conf_str		=> CONF_STR,
		
		config_buffer_o=> config_buffer_s
	);
   
	info1 : work.core_info 
	generic map
	(
		xOffset => 183, --380,
		yOffset => 239  --408
	)
	port map
	(
		clk_i 	=> clock_10,
		
		r_i 		=> osd_r_s,
		g_i 		=> osd_g_s,
		b_i 		=> osd_b_s,
		hSync_i 	=> vga_hsync_n_s,
		vSync_i 	=> vga_vsync_n_s ,

		r_o 		=> info_r_s,
		g_o 		=> info_g_s,
		b_o 		=> info_b_s,
		
		core_char1_s => "000001",  -- V 1.05 for the core
		core_char2_s => "000000",
		core_char3_s => "000101",

		stm_char1_s => unsigned(config_buffer_s(0)(5 downto 0)), 	
		stm_char2_s => unsigned(config_buffer_s(1)(5 downto 0)),
		stm_char3_s => unsigned(config_buffer_s(2)(5 downto 0))
	);
	
	info2 : work.core_copyright
	generic map
	(
		xOffset => 153, --320,
		yOffset => 246 -- 420
	)
	port map
	(
		clk_i 	=> clock_10,
		
		r_i 		=> info_r_s,
		g_i 		=> info_g_s,
		b_i 		=> info_b_s,
		hSync_i 	=> vga_hsync_n_s,
		vSync_i 	=> vga_vsync_n_s ,

		r_o 		=> vga_r_s,
		g_o 		=> vga_g_s,
		b_o 		=> vga_b_s
	);
			  

	
--	kb: entity work.ps2keyb
--	port map (
--		enable_i			=> '1',
--		clock_i			=> pixel_clock,
--		clock_ps2_i		=> clock_div_q(1),
--		reset_i			=> reset_s,
--		--
--		ps2_clk_io		=> ps2_clk_io,
--		ps2_data_io		=> ps2_data_io,
--		--
--		keys_o			=> keys_s,
--		functionkeys_o	=> FKeys_s
--
--	);
--	
--	-- Keyboard clock
--	process(pixel_clock)
--	begin
--		if rising_edge(pixel_clock) then 
--			clock_div_q <= clock_div_q + 1;
--		end if;
--	end process;
	
	-- get scancode from keyboard
	keyboard : entity work.io_ps2_keyboard
	port map (
	  clk       => pixel_clock,
	  kbd_clk   => ps2_clk_io,
	  kbd_dat   => ps2_data_io,
	  interrupt => kbd_intr,
	  scancode  => kbd_scancode
	);

	-- translate scancode to joystick
--	joystick : entity work.kbd_joystick
--	generic map 
--	(
--		osd_cmd		=> "111"
--	)
--	port map 
--	(
--		clk         => pixel_clock,
--		kbdint      => kbd_intr,
--		kbdscancode => std_logic_vector(kbd_scancode), 
--		osd_o			=> keys_s,
--		   
--		joystick_0 	=> joy1_p6_i & joy1_p9_i & joy1_up_i & joy1_down_i & joy1_left_i & joy1_right_i,
--		joystick_1	=> joy2_p6_i & joy2_p9_i & joy2_up_i & joy2_down_i & joy2_left_i & joy2_right_i,
--
--		-- joystick_0 and joystick_1 should be swapped
--		joyswap 		=> '0',
--
--		-- player1 and player2 should get both joystick_0 and joystick_1
--		oneplayer	=> '1',
--
--		-- tilt, coin4-1, start4-1
--		controls   => open,
--
--		-- fire12-1, up, down, left, right
--
--		player1    => joy1_s,
--		player2    => joy2_s,
--
--		-- sega joystick
--		sega_clk  	=>  vga_hsync_n_s,
--		sega_strobe	=> menu_joyp7_s
--	);


	joystick_ser : joydecoder
	port map
	(		
    -----------------------------------------
    clk 				=> clock_20,
    joy_data		=> joy_data,
    joy_clk			=> joy_clk,
    joy_load_n		=> joy_load_n,
    -----------------------------------------
    joy1up			=> joy1_up_i,
    joy1down		=> joy1_down_i,
    joy1left		=> joy1_left_i,
    joy1right		=> joy1_right_i,
    joy1fire1		=> joy1_p6_i,
    joy1fire2		=> joy1_p9_i,

    joy2up			=> joy2_up_i,
    joy2down		=> joy2_down_i,
    joy2left		=> joy2_left_i,
    joy2right		=> joy2_right_i,
    joy2fire1		=> joy2_p6_i,
    joy2fire2		=> joy2_p9_i
	);	


	joystick : entity work.kbd_joystick_atari
	generic map 
	(
		osd_cmd		=> "111"
	)
	port map 
	(
		clk         => pixel_clock,
		kbdint      => kbd_intr,
		kbdscancode => std_logic_vector(kbd_scancode), 
		osd_o			=> keys_s,
		   
		joystick_0 	=> not (joy1_p6_i & joy1_p9_i & joy1_up_i & joy1_down_i & joy1_left_i & joy1_right_i),
		joystick_1	=> not (joy2_p6_i & joy2_p9_i & joy2_up_i & joy2_down_i & joy2_left_i & joy2_right_i),

		-- joystick_0 and joystick_1 should be swapped
		joyswap 		=> '0',

		-- player1 and player2 should get both joystick_0 and joystick_1
		oneplayer	=> '1',

		-- tilt, coin4-1, start4-1
		controls   => open,

		-- fire12-1, up, down, left, right

		player1    => joy1_s,
		player2    => joy2_s,

		-- sega joystick
--		sega_clk  	=>  hs, --vga_hsync_n_s,
--		sega_strobe	=> menu_joyp7_s, 
--		changeScandoubler	=> changeScandoubler, 
--		esc_reset	=> esc_reset, 
--		f1_whitenoise	=> white_noise_change,
--		f2_resetstm    => reset_stm_change,
		
		fn_pulse        => fn_pulse,
		fn_toggle 	    => fn_toggle

	);	
	
	
		
		process(pixel_clock)
		begin
			AUDIO_L <= '0';
			AUDIO_R <= '0';

			if fn_toggle(3) = '1' then				
				stm_rst_o <= '0'; 
			else
				stm_rst_o <= 'Z'; 
			end if;
		end process;
	

-- Cambiar entre 15khz y 31khz
process (fn_pulse(1)) --changescandoubler
begin
  if rising_edge(fn_pulse(1)) then
    scandoubler_disable <= not scandoubler_disable;	 
  end if;
end process;

process (fn_pulse(2))
begin
  if rising_edge(fn_pulse(2)) then
    white_noise <= not white_noise;
  end if;
end process;



	scandoubler1 : scandoubler 
	port map
	(
	
	   clk_x2	   => clock_20, 
		clk_pix		=> clock_10,
	
		scanlines  => "00",
      scandoubler_disable => scandoubler_disable,		
		
		hs_in      => vga_hsync_n_s,
		vs_in      => vga_vsync_n_s,
		r_in    	  => vga_r_s & "00",
		g_in    	  => vga_g_s & "00",
		b_in    	  => vga_b_s & "00",
		
--		hs_out     => vga_hsync_n_rgb,
--		vs_out     => vga_vsync_n_rgb,
--		r_out      => vga_r_rgb,
--		g_out      => vga_g_rgb,
--		b_out      => vga_b_rgb
		hs_out     => vga_hsync_n_rgb,
		vs_out     => vga_vsync_n_rgb,
		r_out(5 downto 1)      => vga_r_rgb,
		g_out(5 downto 1)      => vga_g_rgb,
		b_out(5 downto 1)      => vga_b_rgb
	);			

--	sega_clk <= vga_hsync_n_rgb when scandoubler_disable = '1' else vga_hsync_n_s;
			
	VGA_R <= (vga_r_s & "00") when scandoubler_disable = '1' else vga_r_rgb & "0"; 
	VGA_G <= (vga_g_s & "00") when scandoubler_disable = '1' else vga_g_rgb & "0"; 
	VGA_B <= (vga_b_s & "00") when scandoubler_disable = '1' else vga_b_rgb & "0"; 
	
	cs <= not(vga_hsync_n_s xor vga_vsync_n_s) when scandoubler_disable = '1' else not (vga_hsync_n_rgb xor vga_vsync_n_rgb);
	hs <= vga_hsync_n_s when scandoubler_disable = '1' else vga_hsync_n_rgb; 
	vs <= vga_vsync_n_s when scandoubler_disable = '1' else vga_vsync_n_rgb;

	VGA_HS <= cs  when scandoubler_disable = '1' else hs;
	VGA_VS <= '1' when scandoubler_disable = '1' else vs;
	

end architecture;
