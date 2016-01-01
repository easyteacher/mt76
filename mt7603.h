/*
 * Copyright (C) 2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __MT7603_H
#define __MT7603_H

#include "mt76.h"
#include "mt7603_regs.h"

#define MT7603_MAX_INTERFACES	4
#define MT7603_WTBL_SIZE	128
#define MT7603_WTBL_RESERVED	(MT7603_WTBL_SIZE - 1)

#define MT7603_FIRMWARE_E1	"mt7603_e1.bin"
#define MT7603_FIRMWARE_E2	"mt7603_e2.bin"

#define MT7603_EEPROM_SIZE	1024

enum {
	MT7603_REV_E1 = 0x00,
	MT7603_REV_E2 = 0x10
};

enum mt7603_bw {
	MT_BW_20,
	MT_BW_40,
	MT_BW_80,
};

struct mt7603_mcu {
	struct mutex mutex;

	wait_queue_head_t wait;
	struct sk_buff_head res_q;

	struct mt76_queue q_rx;
	u32 msg_seq;

	bool running;
};

struct mt7603_vif {
	u8 idx;

	struct mt76_wcid group_wcid;
};

struct mt7603_sta {
	struct mt76_wcid wcid; /* must be first */
};

struct mt7603_dev {
	struct mt76_dev mt76; /* must be first */

	struct mutex mutex;
	struct cfg80211_chan_def chandef;

	u32 irqmask;
	spinlock_t irq_lock;

	u32 rxfilter;

	u8 vif_mask;
	unsigned long wcid_mask[MT7603_WTBL_SIZE / BITS_PER_LONG];

	u8 rx_chains;
	u8 tx_chains;

	u8 rssi_offset[3];

	struct mt7603_mcu mcu;
	struct mt76_queue q_rx;

	struct net_device napi_dev;
	struct napi_struct napi;

	struct tasklet_struct tx_tasklet;
	struct tasklet_struct rx_tasklet;
	struct tasklet_struct pre_tbtt_tasklet;
};

extern const struct ieee80211_ops mt7603_ops;

u32 mt7603_reg_map(struct mt7603_dev *dev, u32 addr);

struct mt7603_dev *mt7603_alloc_device(struct device *pdev);
irqreturn_t mt7603_irq_handler(int irq, void *dev_instance);

int mt7603_register_device(struct mt7603_dev *dev);
int mt7603_dma_init(struct mt7603_dev *dev);
void mt7603_dma_cleanup(struct mt7603_dev *dev);
int mt7603_mcu_init(struct mt7603_dev *dev);
int mt7603_tx_queue_mcu(struct mt7603_dev *dev, enum mt76_txq_id qid,
			struct sk_buff *skb);
int mt7603_tx_queue_skb(struct mt76_dev *cdev, struct mt76_queue *q,
			struct sk_buff *skb, struct mt76_txwi_cache *t,
			struct mt76_wcid *wcid, struct ieee80211_sta *sta);

void mt7603_set_irq_mask(struct mt7603_dev *dev, u32 clear, u32 set);

static inline void mt7603_irq_enable(struct mt7603_dev *dev, u32 mask)
{
	mt7603_set_irq_mask(dev, 0, mask);
}

static inline void mt7603_irq_disable(struct mt7603_dev *dev, u32 mask)
{
	mt7603_set_irq_mask(dev, mask, 0);
}

void mt7603_mac_start(struct mt7603_dev *dev);
void mt7603_mac_stop(struct mt7603_dev *dev);
int mt7603_mac_fill_rx(struct mt7603_dev *dev, struct sk_buff *skb);

int mt7603_set_channel(struct mt7603_dev *dev, struct cfg80211_chan_def *def);
int mt7603_mcu_set_channel(struct mt7603_dev *dev);
int mt7603_mcu_reg_read(struct mt7603_dev *dev, u32 reg, u32 *val, bool rf);
int mt7603_mcu_set_eeprom(struct mt7603_dev *dev);
int mt7603_mcu_exit(struct mt7603_dev *dev);

void mt7603_wtbl_init(struct mt7603_dev *dev, int idx, const u8 *addr);
void mt7603_wtbl_clear(struct mt7603_dev *dev, int idx);

int mt7603_mac_write_txwi(struct mt76_dev *mdev, void *txwi_ptr,
			  struct sk_buff *skb, struct mt76_wcid *wcid,
			  struct ieee80211_sta *sta);

#endif
