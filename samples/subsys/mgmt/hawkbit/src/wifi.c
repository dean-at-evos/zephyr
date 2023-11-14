/*
 * Copyright (c) 2023 Evos
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>

#include <zephyr/kernel.h>

#include <zephyr/net/wifi_mgmt.h>

static struct net_mgmt_event_callback mgmt_cb;

/* Semaphore to indicate a lease has been acquired */
static K_SEM_DEFINE(got_address, 0, 1);

LOG_MODULE_REGISTER(wifi);

static void handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface)
{

	switch (mgmt_event) {
	case NET_EVENT_IPV4_DHCP_START:
		LOG_INF("DHCP Start... %u", mgmt_event);
		break;

	case NET_EVENT_IPV4_DHCP_BOUND:
		LOG_INF("DHCP Bound... %u", mgmt_event);
		char buf[NET_IPV4_ADDR_LEN];

		LOG_INF("Your address: %s",
			(net_addr_ntop(AF_INET, &iface->config.dhcpv4.requested_ip, buf,
				       sizeof(buf))));
		LOG_INF("Lease time: %u seconds", iface->config.dhcpv4.lease_time);
		LOG_INF("Subnet: %s", (net_addr_ntop(AF_INET, &iface->config.ip.ipv4->netmask, buf,
						     sizeof(buf))));
		LOG_INF("Router: %s",
			(net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, buf, sizeof(buf))));
		k_sem_give(&got_address);
		break;

	default:
		LOG_DBG("IF Event %u", mgmt_event);
	}
}

/**
 * WiFi connect starts dhcp wait for a lease to be acquired.
 */
void app_wifi_startup(void)
{
	struct net_if *iface;

	net_mgmt_init_event_callback(&mgmt_cb, handler,
				     NET_EVENT_IPV4_DHCP_START | NET_EVENT_IPV4_DHCP_BOUND);
	net_mgmt_add_event_callback(&mgmt_cb);
	LOG_INF("WiFi startup waiting...");
	/* Wait for a lease */
	k_sem_take(&got_address, K_FOREVER);
	LOG_INF("WiFi startup finished.");
}
