#ifndef SRC_LIBRARY_OS_NETWORK_HPP_
#define SRC_LIBRARY_OS_NETWORK_HPP_

#include <stdlib.h>
#include <vector>

typedef enum { IFACE_TYPE_ETHERNET, IFACE_TYPE_WIRELESS } IFACE_TYPE;

typedef struct {
	int id;
	char description[1024];
	unsigned char mac_address[8];
	unsigned char ipv4_address[4];
	IFACE_TYPE type;
} OsAdapterInfo;

bool getAdapterInfos(std::vector<OsAdapterInfo>& adapterInfos);

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* To get defns of NI_MAXSERV and NI_MAXHOST */
#endif
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <unordered_map>
#include <string.h>
#include <memory.h>


using namespace std;

size_t mstrnlen_s(const char *szptr, size_t maxsize);
bool getAdapterInfos(vector<OsAdapterInfo> &adapterInfos);

// strnln_s is not well supported and strlen is marked unsafe..
size_t mstrnlen_s(const char *szptr, size_t maxsize) {
	if (szptr == nullptr) {
		return 0;
	}
	size_t count = 0;
	while (*szptr++ && maxsize--) {
		count++;
	}
	return count;
}


/**
 *
 * @param adapterInfos
 * @param adapter_info_size
 * @return
 */
 bool getAdapterInfos(vector<OsAdapterInfo> &adapterInfos) {
	unordered_map<string, OsAdapterInfo> adapterByName;

	bool f_return = true;
	struct ifaddrs *ifaddr, *ifa;
	int family, n = 0;
	unsigned int if_num, if_max;

	if (getifaddrs(&ifaddr) == -1) {
		//LOG_WARN("getifaddrs failed == -1");
		return false;
	}

	for (ifa = ifaddr, n = 0, if_num = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == NULL || (ifa->ifa_flags & IFF_LOOPBACK) != 0) {
			continue;
		}
		string if_name(ifa->ifa_name, mstrnlen_s(ifa->ifa_name, NI_MAXHOST));
		// if_name_position = ifname_position(ifnames, ifa->ifa_name, if_num);
		// interface name not seen en advance
		OsAdapterInfo *currentAdapter;
		// FIXME not working
		if (adapterByName.find(if_name) == adapterByName.end()) {
			OsAdapterInfo newAdapter;
			memset(&newAdapter, 0, sizeof(OsAdapterInfo));
			strncpy(&newAdapter.description[0], ifa->ifa_name, NI_MAXHOST-1);
			adapterByName[if_name] = newAdapter;
		}
		auto it = adapterByName.find(if_name);
		currentAdapter = &it->second;
		family = ifa->ifa_addr->sa_family;
		/* Display interface name and family (including symbolic
		 form of the latter for the common families) */
#ifdef _DEBUG
		printf("%-8s %s (%d)\n", ifa->ifa_name,
			   (family == AF_PACKET) ? "AF_PACKET"
									 : (family == AF_INET) ? "AF_INET" : (family == AF_INET6) ? "AF_INET6" : "???",
			   family);
#endif
		/* For an AF_INET* interface address, display the address
		 * || family == AF_INET6*/
		if (family == AF_INET) {
			struct sockaddr_in *s1 = (struct sockaddr_in *)ifa->ifa_addr;
			in_addr_t iaddr = s1->sin_addr.s_addr;
			currentAdapter->ipv4_address[0] = (iaddr & 0x000000ff);
			currentAdapter->ipv4_address[1] = (iaddr & 0x0000ff00) >> 8;
			currentAdapter->ipv4_address[2] = (iaddr & 0x00ff0000) >> 16;
			currentAdapter->ipv4_address[3] = (iaddr & 0xff000000) >> 24;

		} else if (family == AF_PACKET && ifa->ifa_data != NULL) {
			struct sockaddr_ll *s1 = (struct sockaddr_ll *)ifa->ifa_addr;
			int i;
			for (i = 0; i < 6; i++) {
				currentAdapter->mac_address[i] = s1->sll_addr[i];
#ifdef _DEBUG
				printf("%02x:", s1->sll_addr[i]);
#endif
			}
#ifdef _DEBUG
			printf("\t %s\n", ifa->ifa_name);
#endif
		}
	}
	freeifaddrs(ifaddr);

	// FIXME sort by eth , enps, wlan
	if (adapterByName.size() == 0) {
		f_return = false;
	} else {
		f_return = true;
		adapterInfos.reserve(adapterByName.size());
		for (auto &it : adapterByName) {
			adapterInfos.push_back(it.second);
		}
	}
	return f_return;
}

#endif /* SRC_LIBRARY_OS_NETWORK_HPP_ */

