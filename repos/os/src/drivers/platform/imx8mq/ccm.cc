#include <ccm.h>

/******************************
 ** Frac_pll immplementation **
 ******************************/

void Driver::Ccm::Frac_pll::disable()
{
	write<Config_reg_0::Power_down>(1);
}


void Driver::Ccm::Frac_pll::enable()
{
	if (!read<Config_reg_0::Power_down>()) return;

	write<Config_reg_0::Power_down>(0);

	for (unsigned i = 0; i < 0xfffff; i++) {
		if (read<Config_reg_0::Lock_status>()) { break; }
	}
}


Driver::Clock & Driver::Ccm::Frac_pll::_parent() const
{
	Name pname;

	switch (read<Config_reg_0::Ref_sel>()) {
		case Config_reg_0::Ref_sel::REF_CLK_25M:   pname = "25m_ref_clk";      break;
		case Config_reg_0::Ref_sel::REF_CLK_27M:   pname = "27m_ref_clk";      break;
		case Config_reg_0::Ref_sel::HDMI_PHY_27M:  pname = "hdmi_phy_27m_clk"; break;
		case Config_reg_0::Ref_sel::CLK_P_N:       pname = "no_clk";           break;
	};

	return static_cast<Clock_tree_element*>(_tree.first()->find_by_name(pname.string()))->object();
};


void Driver::Ccm::Frac_pll::set_parent(Name parent)
{
	if (parent == "25m_ref_clk") {
		write<Config_reg_0::Ref_sel>(Config_reg_0::Ref_sel::REF_CLK_25M);
		return;
	}
	if (parent == "27m_ref_clk") {
		write<Config_reg_0::Ref_sel>(Config_reg_0::Ref_sel::REF_CLK_27M);
		return;
	}
	if (parent == "hdmi_phy_27m_clk") {
		write<Config_reg_0::Ref_sel>(Config_reg_0::Ref_sel::HDMI_PHY_27M);
		return;
	}
	write<Config_reg_0::Ref_sel>(Config_reg_0::Ref_sel::CLK_P_N);
}


void Driver::Ccm::Frac_pll::set_rate(unsigned long rate)
{
	static constexpr uint32_t fixed_frac = 1 << 24;

	uint64_t r        = rate * 2;
	uint64_t pr       = _parent().get_rate() * 8;
	uint32_t div_int  = (uint32_t)((r / pr) & 0b1111111);
	uint32_t div_frac = (uint32_t)((r - div_int * pr) * fixed_frac) / pr;

	Config_reg_1::access_t v = 0;
	Config_reg_1::Int_div_ctl::set(v, div_int);
	Config_reg_1::Frac_div_ctl::set(v, div_frac);
	write<Config_reg_1>(v);

	write<Config_reg_0::Refclk_div_value>(0);
	write<Config_reg_0::Output_div_value>(0);
	write<Config_reg_0::Newdiv_val>(1);

	/* wait for ack, if powered and not bypassed */
	if (!(read<Config_reg_0::Bypass>() ||
		  read<Config_reg_0::Power_down>())) {
		for (unsigned i = 0; i < 0xfffff; i++) {
			if (read<Config_reg_0::Newdiv_ack>()) { break; }
		}
	}

	write<Config_reg_0::Newdiv_val>(0);
}


unsigned long Driver::Ccm::Frac_pll::get_rate() const
{
	static constexpr uint32_t fixed_frac = 1 << 24;

	uint32_t divq  = (read<Config_reg_0::Output_div_value>() + 1) * 2;
	uint32_t divr  = read<Config_reg_0::Refclk_div_value>();
	uint32_t divff = read<Config_reg_1::Frac_div_ctl>();
	uint32_t divfi = read<Config_reg_1::Int_div_ctl>();

	uint64_t ref = _parent().get_rate() * 8 / divr;

	return (ref * (divfi + 1) / divq) +
	       (ref * divff / fixed_frac / divq);
}


Driver::Ccm::Frac_pll::Frac_pll(Name name, addr_t const base, Clock_tree & tree)
: Clock(name, tree), Mmio(base), _tree(tree)
{
	write<Config_reg_0::Bypass>(0);
}


/******************************
 ** Sccg_pll immplementation **
 ******************************/

void Driver::Ccm::Sccg_pll::set_parent(Name) {}


void Driver::Ccm::Sccg_pll::set_rate(unsigned long) { }


unsigned long Driver::Ccm::Sccg_pll::get_rate() const { return 0; }


void Driver::Ccm::Sccg_pll::enable() {}


void Driver::Ccm::Sccg_pll::disable() {}


/********************************
 ** Root_clock immplementation **
 ********************************/

void Driver::Ccm::Root_clock::set_rate(unsigned long rate)
{
	uint32_t pre_div   = 1;
	uint32_t post_div  = 1;
	int      deviation = (int)(~0U >> 1);

	unsigned long parent_rate =
		_ref_clks[read<Target_reg::Ref_sel>()].ref.get_rate();

	for (unsigned pre = 1; pre < (1<<3); pre++) {
		for (unsigned post = 1; post < (1<<6); post++) {
			int diff = (parent_rate / pre) / post - rate;
			if (abs(diff) < abs(deviation)) {
				pre_div   = pre;
				post_div  = post;
				deviation = diff;
			}
		}
	}

	write<Target_reg::Pre_div>(pre_div);
	write<Target_reg::Post_div>(post_div);
};


void Driver::Ccm::Root_clock::set_parent(Name name)
{
	for (unsigned i = 0; i < REF_CLK_MAX; i++) {
		if (_ref_clks[i].ref.name() == name) {
			write<Target_reg::Ref_sel>(i);
			return;
		}
	}

	warning("Reference clock ", name, " cannot be set");
}


unsigned long Driver::Ccm::Root_clock::get_rate() const
{
	unsigned long parent_rate =
		_ref_clks[read<Target_reg::Ref_sel>()].ref.get_rate();
	return (parent_rate / read<Target_reg::Pre_div>()) / read<Target_reg::Post_div>();
}


void Driver::Ccm::Root_clock::enable() { write<Target_reg::Enable>(1); }


void Driver::Ccm::Root_clock::disable() { write<Target_reg::Enable>(0); }


/**************************
 ** Gate immplementation **
 **************************/

void Driver::Ccm::Gate::enable()  { write<Ccgr>(0x3); }


void Driver::Ccm::Gate::disable() { write<Ccgr>(0x0); }


/*******************
 ** CCM interface **
 *******************/

Driver::Ccm::Ccm(Genode::Env & env) : env(env)
{
	video_pll1_clk.enable();

	/* set VIDEO PLL */
	video_pll1_clk.set_parent(m27_ref_clk.name());
	video_pll1_clk.set_rate(593999999);

	audio_pll1_clk.disable();
	audio_pll2_clk.disable();
	video_pll1_clk.disable();
	gpu_pll_clk.disable();
	vpu_pll_clk.disable();
	system_pll3_clk.disable();
	video_pll2_clk.disable();

	/* turn off all unnecessary root clocks */
	arm_m4_clk_root.disable();
	vpu_a53_clk_root.disable();
	gpu_core_clk_root.disable();
	gpu_shader_clk_root.disable();
	enet_axi_clk_root.disable();
	nand_usdhc_bus_clk_root.disable();
	vpu_bus_clk_root.disable();
	display_axi_clk_root.disable();
	display_apb_clk_root.disable();
	display_rtrm_clk_root.disable();
	usb_bus_clk_root.disable();
	gpu_axi_clk_root.disable();
	gpu_ahb_clk_root.disable();
	audio_ahb_clk_root.disable();
	mipi_dsi_esc_rx_clk_root.disable();
	vpu_g1_clk_root.disable();
	vpu_g2_clk_root.disable();
	display_dtrc_clk_root.disable();
	display_dc8000_clk_root.disable();
	pcie1_ctrl_clk_root.disable();
	pcie1_phy_clk_root.disable();
	pcie1_aux_clk_root.disable();
	dc_pixel_clk_root.disable();
	lcdif_pixel_clk_root.disable();
	sai1_clk_root.disable();
	sai2_clk_root.disable();
	sai3_clk_root.disable();
	sai4_clk_root.disable();
	sai5_clk_root.disable();
	sai6_clk_root.disable();
	spdif1_clk_root.disable();
	spdif2_clk_root.disable();
	enet_ref_clk_root.disable();
	enet_timer_clk_root.disable();
	enet_phy_ref_clk_root.disable();
	nand_clk_root.disable();
	qspi_clk_root.disable();
	usdhc1_clk_root.disable();
	usdhc2_clk_root.disable();
	i2c1_clk_root.disable();
	i2c2_clk_root.disable();
	i2c3_clk_root.disable();
	i2c4_clk_root.disable();
	uart2_clk_root.disable();
	uart3_clk_root.disable();
	uart4_clk_root.disable();
	usb_core_ref_clk_root.disable();
	usb_phy_ref_clk_root.disable();
	ecspi1_clk_root.disable();
	ecspi2_clk_root.disable();
	pwm1_clk_root.disable();
	pwm2_clk_root.disable();
	pwm3_clk_root.disable();
	pwm4_clk_root.disable();
	gpt1_clk_root.disable();
	gpt2_clk_root.disable();
	gpt3_clk_root.disable();
	gpt4_clk_root.disable();
	gpt5_clk_root.disable();
	gpt6_clk_root.disable();
	trace_clk_root.disable();
	wdog_clk_root.disable();
	wrclk_clk_root.disable();
	ipp_do_clko1clk_root.disable();
	ipp_do_clko2_clk_root.disable();
	mipi_dsi_core_clk_root.disable();
	mipi_dsi_phy_ref_clk_root.disable();
	mipi_dsi_dbi_clk_root.disable();
	old_mipi_dsi_esc_clk_root.disable();
	mipi_csi1_core_clk_root.disable();
	mipi_csi1_phy_ref_clk_root.disable();
	mipi_csi1_esc_clk_root.disable();
	mipi_csi2_core_clk_root.disable();
	mipi_csi2_phy_ref_clk_root.disable();
	mipi_csi2_esc_clk_root.disable();
	pcie2_ctrl_clk_root.disable();
	pcie2_phy_clk_root.disable();
	pcie2_aux_clk_root.disable();
	ecspi3_clk_root.disable();
	old_mipi_dsi_esc_rx_clk_root.disable();
	display_hdmi_clk_root.disable();

	/* set certain reference clocks */
	ahb_clk_root.set_parent("system_pll1_div6");
	nand_usdhc_bus_clk_root.set_parent("system_pll1_div3");
	audio_ahb_clk_root.set_parent("system_pll2_div2");
	pcie1_ctrl_clk_root.set_parent("system_pll2_div5");
	pcie1_phy_clk_root.set_parent("system_pll2_div10");
	pcie2_ctrl_clk_root.set_parent("system_pll2_div5");
	pcie2_phy_clk_root.set_parent("system_pll2_div10");
	mipi_csi1_core_clk_root.set_parent("system_pll1_div3");
	mipi_csi1_phy_ref_clk_root.set_parent("system_pll2_clk");
	mipi_csi1_esc_clk_root.set_parent("system_pll1_clk");
	mipi_csi2_core_clk_root.set_parent("system_pll1_div3");
	mipi_csi2_phy_ref_clk_root.set_parent("system_pll2_clk");
	mipi_csi2_esc_clk_root.set_parent("system_pll1_clk");

	/* increase NOC clock for better DDR performance */
	/*NOC_CLK_ROOT to 800 M*/
	//Genode::log("NOC_CLK_ROOT ", read<Target_root<NOC_CLK_ROOT>>());
}
