#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <stdlib.h>
#include <string.h>




//#define BASEPORT 0xc120   // 0x378 /* lp1 */


//host_features = 0x711fffe3
//after nego = 0x100f9821
#define BASEPORT baseaddr

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long  uint64_t;


//#define VIRTIO_PCI_CONFIG(hw) (((hw)->use_msix) ? 24 : 20)
#define VIRTIO_PCI_CONFIG (24)

uint32_t baseaddr;
uint32_t host_features;
uint32_t guest_features;
uint32_t nego_features;

#define ETHER_ADDR_LEN  6 /**< Length of Ethernet address. */
#define ETHER_MIN_MTU   68 /**< Minimum MTU for IPv4 packets, see RFC 791. */


#define VIRTIO_PCI_HOST_FEATURES  0  
#define VIRTIO_PCI_GUEST_FEATURES 4  
#define VIRTIO_PCI_QUEUE_PFN      8  
#define VIRTIO_PCI_QUEUE_NUM      12 
#define VIRTIO_PCI_QUEUE_SEL      14 
#define VIRTIO_PCI_QUEUE_NOTIFY   16 
#define VIRTIO_PCI_STATUS         18 
#define VIRTIO_PCI_ISR		  	  19 

// Only if MSIX is enabled: 

#define VIRTIO_MSI_CONFIG_VECTOR  20
#define VIRTIO_MSI_QUEUE_VECTOR	  22





#ifndef offsetof
#define offsetof(t, m) ((size_t) &((t *)0)->m)
#endif

struct virtio_net_conf {
	/* The config defining mac address (if VIRTIO_NET_F_MAC) */
	uint8_t    mac[ETHER_ADDR_LEN];
	/* See VIRTIO_NET_F_STATUS and VIRTIO_NET_S_* above */
	uint16_t   status;
	uint16_t   max_virtqueue_pairs;
	uint16_t   mtu;
} __attribute__((packed));

#define VIRTIO_NET_F_CSUM		0	/* Host handles pkts w/ partial csum */
#define VIRTIO_NET_F_GUEST_CSUM	1	/* Guest handles pkts w/ partial csum */
#define VIRTIO_NET_F_MTU		3	/* Initial MTU advice. */
#define VIRTIO_NET_F_MAC		5	/* Host has given MAC address. */
#define VIRTIO_NET_F_GSO        6   /* Host handles pkts w/ any GSO type */
#define VIRTIO_NET_F_GUEST_TSO4	7	/* Guest can handle TSOv4 in. */
#define VIRTIO_NET_F_GUEST_TSO6	8	/* Guest can handle TSOv6 in. */
#define VIRTIO_NET_F_GUEST_ECN	9	/* Guest can handle TSO[6] w/ ECN in. */
#define VIRTIO_NET_F_GUEST_UFO	10	/* Guest can handle UFO in. */
#define VIRTIO_NET_F_HOST_TSO4	11	/* Host can handle TSOv4 in. */
#define VIRTIO_NET_F_HOST_TSO6	12	/* Host can handle TSOv6 in. */
#define VIRTIO_NET_F_HOST_ECN	13	/* Host can handle TSO[6] w/ ECN in. */
#define VIRTIO_NET_F_HOST_UFO	14	/* Host can handle UFO in. */




#define VIRTIO_NET_F_MRG_RXBUF	15	/* Host can merge receive buffers. */
#define VIRTIO_NET_F_STATUS		16	/* virtio_net_config.status available */
#define VIRTIO_NET_F_CTRL_VQ	17	/* Control channel available */
#define VIRTIO_NET_F_CTRL_RX	18	/* Control channel RX mode support */
#define VIRTIO_NET_F_CTRL_VLAN	19	/* Control channel VLAN filtering */
#define VIRTIO_NET_F_CTRL_RX_EXTRA 20	/* Extra RX mode control support */
#define VIRTIO_NET_F_GUEST_ANNOUNCE 21	/* Guest can announce device on the network */
#define VIRTIO_NET_F_MQ		22			/* Device supports Receive Flow Steering */
#define VIRTIO_NET_F_CTRL_MAC_ADDR 23	/* Set MAC address */

/* Do we get callbacks when the ring is completely used, even if we've  suppressed them? */
#define VIRTIO_F_NOTIFY_ON_EMPTY	24

/* Can the device handle any descriptor layout? */
#define VIRTIO_F_ANY_LAYOUT		27

/* We support indirect buffer descriptors */
#define VIRTIO_RING_F_INDIRECT_DESC	28

#define VIRTIO_F_VERSION_1		32
#define VIRTIO_F_IOMMU_PLATFORM	33


struct virtio_net_conf n_config;


struct virtio_feature{
	char *name;
	char *title;
	uint8_t value;
};

int with_feature(uint32_t features, uint32_t bit)
{
	return (features & (1ULL << bit)) != 0;
}

struct virtio_feature virt_f[] =
{
	{"csum", 		"Host handles pkts w/ partial csum",			VIRTIO_NET_F_CSUM},
	{"guest_csum", 	"Guest handles pkts w/ partial csum",			VIRTIO_NET_F_GUEST_CSUM},
	{"mtu",			"Initial MTU advice.",							VIRTIO_NET_F_MTU},	  			// 3
	{"mac",			"Host has given MAC address.",					VIRTIO_NET_F_MAC},	  			// 5
	{"gso",			"Host handles pkts w/ any GSO type", 			VIRTIO_NET_F_GSO}, 				//6
	{"guest_tso4",	"Guest can handle TSOv4 in",					VIRTIO_NET_F_GUEST_TSO4},
	{"guest_tso6",	"Guest can handle TSOv6 in.",					VIRTIO_NET_F_GUEST_TSO6},
	{"guest_ecn",	"Guest can handle TSO[6] w/ ECN in",			VIRTIO_NET_F_GUEST_ECN},
	{"guest_ufo",	"Guest can handle UFO in.",						VIRTIO_NET_F_GUEST_UFO},
	{"host_tso4",	"Host can handle TSOv4 in", 					VIRTIO_NET_F_HOST_TSO4},
	{"host_tso6",	"Host can handle TSOv6 in", 					VIRTIO_NET_F_HOST_TSO6},		//12
	{"host_ecn",	"Host can handle TSO[6] w/ ECN in", 			VIRTIO_NET_F_HOST_ECN},
	{"host_ufo",	"Host can handle UFO in", 						VIRTIO_NET_F_HOST_UFO},
	{"merge_rxbuf",	"Host can merge receive buffers",				VIRTIO_NET_F_MRG_RXBUF},
	{"status",		"virtio_net_config.status available",			VIRTIO_NET_F_STATUS},
	{"ctrl_vq",		"Control channel available",					VIRTIO_NET_F_CTRL_VQ},
	{"ctrl_rx",		"Control channel RX mode support",				VIRTIO_NET_F_CTRL_RX},
	{"ctrl_vlan",	"Control channel VLAN filtering ",				VIRTIO_NET_F_CTRL_VLAN},
	{"ctrl_rx_extra","Extra RX mode control support",				VIRTIO_NET_F_CTRL_RX_EXTRA},   //20
	{"guest_announce","Guest can announce device on the network", 	VIRTIO_NET_F_GUEST_ANNOUNCE},
	{"mq",			"Device supports Receive Flow Steering", VIRTIO_NET_F_MQ},	
	{"ctrl_mac_addr","Set MAC address", VIRTIO_NET_F_CTRL_MAC_ADDR},	//23
	{"on_emtpy",		"get callbacks when the ring is completely used",	VIRTIO_F_NOTIFY_ON_EMPTY},	 			//24
	{"any_layout",		"Can the device handle any descriptor layout",			VIRTIO_F_ANY_LAYOUT},					//27
	{"indirect_desc",	"We support indirect buffer descriptors ", 			VIRTIO_RING_F_INDIRECT_DESC},		//28
	{"version_1",		"VIRTIO_F_VERSION_1", 	VIRTIO_F_VERSION_1},				//32	
	{"iommu_platform",	"VIRTIO_F_IOMMU_PLATFORM",		VIRTIO_F_IOMMU_PLATFORM},		//33
};

void dump_feautre(char *name, uint32_t features)
{
	int i = 0;

	printf("name:%s,features:0x%lx\n", name, features);
	for(i = 0; i < sizeof(virt_f) / sizeof(struct virtio_feature); i++){
		if(with_feature(features, virt_f[i].value))
			printf("    %-20s %s\n", virt_f[i].name, virt_f[i].title);
	}
}

void ioport_read(off_t offset, void *data, size_t len)
{
	uint8_t *d;
	int size;
	uint32_t reg = BASEPORT + offset;

	//printf("read offset:0x%x,len:%d\n", offset, len);
	for (d = data; len > 0; d += size, reg += size, len -= size) {
		if (len >= 4) {
			size = 4;
			*(uint32_t *)d = inl(reg);
			usleep(100000);
		} else if (len >= 2) {
			size = 2;
			*(uint16_t *)d = inw(reg);
			usleep(100000);
		} else {
			size = 1;
			*d = inb(reg);
			usleep(100000);
		}
	}
}

/*
Status byte for guest to report progress.
#define VIRTIO_CONFIG_STATUS_RESET	0x00
#define VIRTIO_CONFIG_STATUS_ACK		0x01
#define VIRTIO_CONFIG_STATUS_DRIVER    0x02
#define VIRTIO_CONFIG_STATUS_DRIVER_OK 0x04
#define VIRTIO_CONFIG_STATUS_FEATURES_OK 0x08
#define VIRTIO_CONFIG_STATUS_FAILED    0x80
*/

void read_status(void)
{
	uint8_t status;
	ioport_read(VIRTIO_PCI_STATUS, &status, 1);
	printf("read status:0x%x\n", status);
}

void read_config(uint32_t features)
{
	struct virtio_net_conf *config = &n_config;
	off_t offset = 0;
	
	memset(config, 0x0, sizeof(struct virtio_net_conf));

	printf("\nread pci config...\n");

	if (!with_feature(features, VIRTIO_NET_F_CTRL_VQ)){
		printf("not support VIRTIO_NET_F_CTRL_VQ\n");
		return;
	}

	if(with_feature(features, VIRTIO_NET_F_MAC)){
		offset = offsetof(struct virtio_net_conf, mac) + VIRTIO_PCI_CONFIG;
		ioport_read(offset, &config->mac, sizeof(config->mac));
		printf("read MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
			config->mac[0], config->mac[1],
			config->mac[2], config->mac[3],
			config->mac[4], config->mac[5]);
	}
	
	if (with_feature(features, VIRTIO_NET_F_STATUS)) {
		offset = offsetof(struct virtio_net_conf, status) + VIRTIO_PCI_CONFIG;
		ioport_read(offset, &config->status, sizeof(config->status));
		printf("config->status=%d\n", config->status);
	} else {
		printf("VIRTIO_NET_F_STATUS is not supported\n");
		config->status = 0;
	}

	if (with_feature(features, VIRTIO_NET_F_MQ)) {
		offset = offsetof(struct virtio_net_conf, max_virtqueue_pairs) + VIRTIO_PCI_CONFIG;
		ioport_read(offset, &config->max_virtqueue_pairs, sizeof(config->max_virtqueue_pairs));
		printf("config->max_virtqueue_pairs=%d\n",config->max_virtqueue_pairs);		
	} else {
		printf("VIRTIO_NET_F_MQ is not supported\n");
		config->max_virtqueue_pairs = 1;
	}

	if (with_feature(features, VIRTIO_NET_F_MTU)) {
		offset = offsetof(struct virtio_net_conf, mtu) + VIRTIO_PCI_CONFIG;
		ioport_read(offset, &config->mtu, sizeof(config->mtu));

		if (config->mtu < ETHER_MIN_MTU) {
			printf("invalid max MTU value (%u)",
					config->mtu);
			return ;
		}
	}else{
		printf("not support MUT\n");
	}
}


void hex_dump(void)
{
	int i = 0;
	uint8_t data;
	uint32_t value32;
	uint16_t value16;
	uint8_t value8;
	
	for(i = 0; i < 32 ; i++){
		data = inb(BASEPORT + i);
		printf("%-2x ", data);
		if(!((i+ 1) % 8))
			printf("\n");
	}

	printf("\n");

	ioport_read(VIRTIO_PCI_HOST_FEATURES, &value32, 4);
	printf("HOST_FEATURES: 0x%x\n", value32);
	ioport_read(VIRTIO_PCI_GUEST_FEATURES, &value32, 4);
	printf("GUEST_FEATURES: 0x%x\n", value32);
	
	ioport_read(VIRTIO_PCI_QUEUE_PFN, &value32, 4);
	printf("QUEUE_PFN: 0x%x\n", value32);
	ioport_read(VIRTIO_PCI_QUEUE_NUM, &value16, 2);
	printf("QUEUE_NUM: 0x%x\n", value16);	
	ioport_read(VIRTIO_PCI_QUEUE_SEL, &value16, 2);
	printf("QUEUE_SEL: 0x%x\n", value16);	

	ioport_read(VIRTIO_PCI_QUEUE_NOTIFY, &value16, 2);
	printf("QUEUE_NOTIFY: 0x%x\n", value16);	

	ioport_read(VIRTIO_PCI_STATUS, &value8, 1);
	printf("STATUS: 0x%x\n", value8);	

	ioport_read(VIRTIO_PCI_ISR, &value8, 1);
	printf("ISR: 0x%x\n", value8);	

	ioport_read(VIRTIO_MSI_CONFIG_VECTOR, &value16, 2);
	printf("MSI_CONFIG_VECTOR: 0x%x\n", value16);	

	ioport_read(VIRTIO_MSI_QUEUE_VECTOR, &value16, 2);
	printf("MSI_QUEUE_VECTOR: 0x%x\n", value16);	
	
}

int main(int argc, char ** argv)
{
	int i = 0, j = 0;

	sscanf(argv[1], "%x", &baseaddr);
	printf("input baseaddr:0x%x\n", baseaddr);
	
	if (ioperm(BASEPORT, 32, 1)) 
	{
		printf("ioperm"); 
		return;
	}

	hex_dump();

	//¶Áoffset: 0
	ioport_read(VIRTIO_PCI_HOST_FEATURES, &host_features, 4);
	dump_feautre("hostfeature", host_features);

	//¶Áoffset: 4
	ioport_read(VIRTIO_PCI_GUEST_FEATURES, &nego_features, 4);
	dump_feautre("nego-ret", nego_features);

	read_status();

	read_config(nego_features);
  	
	if (ioperm(BASEPORT, 32, 0)) 
	{
		perror("ioperm"); 
		return;
	}

  return;
}
