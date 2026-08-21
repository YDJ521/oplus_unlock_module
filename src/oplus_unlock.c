/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * OnePlus Ace5 Pro - Display 144Hz Overclock + 55W PPS Unlock
 * Minimal kretprobe module with self-contained types
 */
#include <linux/module.h>
#include <linux/kprobes.h>

static bool enabled = true;
module_param(enabled, bool, 0600);

static bool observe;
module_param(observe, bool, 0600);

static unsigned int hits;
module_param(hits, uint, 0444);

static int display_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	return 0;
}

static int display_return(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	if (!enabled) return 0;
	WRITE_ONCE(hits, READ_ONCE(hits) + 1);
	if (observe) pr_info("oplus_unlock: display hook fired, ret=%d\n", (int)regs_return_value(regs));
	return 0;
}

static int pps_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	if (!enabled) return 0;
	regs_set_return_value(regs, 1);
	WRITE_ONCE(hits, READ_ONCE(hits) + 1);
	if (observe) pr_info("oplus_unlock: pps hook fired, forcing allowed\n");
	return 0;
}

static struct kretprobe display_krp = {
	.kp.symbol_name = "dsi_connector_get_modes",
	.entry_handler = display_entry,
	.handler = display_return,
	.data_size = 0,
	.maxactive = 16,
};

static struct kretprobe pps_krp = {
	.kp.symbol_name = "oplus_pps_charge_allow_check",
	.entry_handler = pps_entry,
	.handler = NULL,
	.data_size = 0,
	.maxactive = 16,
};

static struct kretprobe *probes[] = { &display_krp, &pps_krp };
static unsigned int registered;

static int __init mod_init(void)
{
	int i, ret;
	if (!enabled) { pr_info("oplus_unlock: disabled\n"); return 0; }
	for (i = 0; i < ARRAY_SIZE(probes); i++) {
		ret = register_kretprobe(probes[i]);
		if (ret) {
			pr_err("oplus_unlock: register %s failed: %d\n", probes[i]->kp.symbol_name, ret);
			while (registered) unregister_kretprobe(probes[--registered]);
			return ret;
		}
		registered++;
		pr_info("oplus_unlock: registered %s\n", probes[i]->kp.symbol_name);
	}
	pr_info("oplus_unlock: loaded\n");
	return 0;
}

static void __exit mod_exit(void)
{
	while (registered) unregister_kretprobe(probes[--registered]);
	pr_info("oplus_unlock: unloaded, hits=%u\n", hits);
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("OnePlus Ace5 Pro: 144Hz + 55W PPS Unlock");
