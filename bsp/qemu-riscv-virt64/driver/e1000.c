/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-25     bigmagic     first version
 */
#include <stdarg.h>
#include <e1000.h>
#include <rtthread.h>
#include <netif/ethernetif.h>
#include <lwipopts.h>

static volatile rt_uint32_t *regs;

#define TX_RING_SIZE 16
#define RX_RING_SIZE 128
#define RX_PKT_BUFF_SIZE 2048

static struct tx_desc tx_ring[TX_RING_SIZE] __attribute__((aligned(16)));
static rt_uint8_t tx_buffs[TX_RING_SIZE][RX_PKT_BUFF_SIZE];

#define RX_RING_SIZE 128
#define RX_PKT_BUFF_SIZE 2048
#define TX_BUFF_SIZE 2048
static rt_uint8_t rx_buffs[RX_RING_SIZE][RX_PKT_BUFF_SIZE];
static struct rx_desc rx_ring[RX_RING_SIZE] __attribute__((aligned(16)));

static rt_uint8_t lwip_tx_buf[TX_BUFF_SIZE];
static rt_uint8_t raw_recv_buff[RX_PKT_BUFF_SIZE];

struct eth_device_e1000
{
    /* inherit from Ethernet device */
    struct eth_device parent;
    /* interface address info. */
    rt_uint8_t enetaddr[8];         /* MAC address  */

    rt_uint32_t iobase;
    rt_uint32_t irqno;
};
static struct eth_device_e1000 _emac;

void e1000_init(rt_uint32_t *xregs)
{
    int i;
    regs = xregs;
     // Reset the device
    regs[E1000_IMS] = 0; // disable interrupts
    regs[E1000_CTL] |= E1000_CTL_RST;
    regs[E1000_IMS] = 0; // redisable interrupts

    // [E1000 14.5] Transmit initialization
    for (i = 0; i < TX_RING_SIZE; i++) 
    {
        tx_ring[i].status = E1000_TXD_STAT_DD;
        tx_ring[i].addr = (rt_uint64_t)tx_buffs[i];
    }
    regs[E1000_TDBAL] = (rt_uint64_t) tx_ring;
    regs[E1000_TDBAH] = 0;
    regs[E1000_TDLEN] = sizeof(tx_ring);
    regs[E1000_TDH] = regs[E1000_TDT] = 0;

    // [E1000 14.4] Receive initialization
    for (i = 0; i < RX_RING_SIZE; i++)
    {
        rx_ring[i].addr = (rt_uint64_t)rx_buffs[i];
    }
    regs[E1000_RDBAL] = (rt_uint64_t) rx_ring;
    regs[E1000_RDH] = 0;
    regs[E1000_RDT] = RX_RING_SIZE - 1;
    regs[E1000_RDLEN] = sizeof(rx_ring);

    // filter by qemu's MAC address, 52:54:00:12:34:56
    regs[E1000_RA] = 0x12005452;
    regs[E1000_RA+1] = 0x5634 | (1<<31);
    // multicast table
    for (int i = 0; i < 4096/32; i++)
    regs[E1000_MTA + i] = 0;
    // transmitter control bits.
    regs[E1000_TCTL] = E1000_TCTL_EN |                   // enable
                       E1000_TCTL_PSP |                  // pad short packets
                       (0x10 << E1000_TCTL_CT_SHIFT) |   // collision stuff
                       (0x40 << E1000_TCTL_COLD_SHIFT);
                       regs[E1000_TIPG] = 10 | (8<<10) | (6<<20); // inter-pkt gap

    // receiver control bits.
    regs[E1000_RCTL] = E1000_RCTL_EN | // enable receiver
                       E1000_RCTL_BAM |                 // enable broadcast
                       E1000_RCTL_SZ_2048 |             // 2048-byte rx buffers
                       E1000_RCTL_SECRC;                // strip CRC
  
    // ask e1000 for receive interrupts.
    regs[E1000_RDTR] = 0; // interrupt after every received packet (no timer)
    regs[E1000_RADV] = 0; // interrupt after every packet (no timer)
    regs[E1000_IMS] = (1 << 7); // RXDW -- Receiver Descriptor Write Back
}

int e1000_transmit(char * buf,int len)
{
    rt_uint32_t tail = regs[E1000_TDT];
    if (tx_ring[tail].cmd & E1000_TXD_CMD_RS)
    {
        if (!(tx_ring[tail].status & E1000_TXD_STAT_DD))
        {
            return -1;
        }
    }
    rt_memcpy(tx_buffs[tail], buf, len);
    tx_ring[tail].length =len;
    tx_ring[tail].cmd |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    tx_ring[tail].status &= ~E1000_TXD_STAT_DD;
    regs[E1000_TDT] = (tail + 1) % TX_RING_SIZE;
    return 0;
}

int e1000_recv(char *buf)
{
    int len=0;
    rt_uint32_t tail = regs[E1000_RDT];
    rt_uint32_t next = (tail + 1) % RX_RING_SIZE;
    if (!(rx_ring[next].status & E1000_RXD_STAT_DD))
    {
        return 0;
    }
    len = rx_ring[next].length;
    rt_memcpy(buf,rx_buffs[next],len);
    regs[E1000_RDT] = next;
    rx_ring[next].status &= ~E1000_RXD_STAT_DD;
    return len;
}

static rt_err_t e1000_bus_init(rt_device_t dev)
{
    rt_uint16_t bus = 0;
    rt_uint16_t func = 0;
    rt_uint16_t offset = 0;
    rt_uint16_t pci_dev = 0;
    rt_uint32_t pci_off = 0;
    rt_uint32_t pci_id = 0;
    rt_uint32_t old = 0;
    volatile rt_uint32_t *base;

    // look at each possible PCI device on bus 0.
    for(pci_dev = 0; pci_dev < 32; pci_dev++)
    {
        pci_off = (bus << 16) | (pci_dev << 11) | (func << 8) | (offset);
        base = (rt_uint32_t *)(VIRT_PCIE_ECAM_ADDR + pci_off);
        pci_id = base[0];
    
        // 100e:8086 is an e1000
        if(pci_id == PCIE_E1000_ADDR)
        {
            // command and status register.
            // bit 0 : I/O access enable
            // bit 1 : memory access enable
            // bit 2 : enable mastering
            base[1] = 7;
            for(int i = 0; i < 6; i++)
            {
                old = base[4+i];
                // writing all 1's to the BAR causes it to be
                // replaced with its size.
                base[4+i] = 0xffffffff;
                base[4+i] = old;
            }

            // tell the e1000 to reveal its registers at
            // physical address 0x40000000.
            base[4+0] = E1000_REGS_ADDR;
            e1000_init((rt_uint32_t*)E1000_REGS_ADDR);
        }
    }
    return RT_EOK;
}

rt_err_t rt_eth_tx(rt_device_t device, struct pbuf *p)
{
    if(p->tot_len > 0)
    {
        /* copy pbuf to a whole ETH frame */
        pbuf_copy_partial(p, lwip_tx_buf, p->tot_len, 0);
        if(e1000_transmit(lwip_tx_buf, p->tot_len) == 0)
        {
            return RT_EOK;
        }
        else
        {
            return -RT_EIO;
        }
    }
    return -RT_EIO;
}

struct pbuf *rt_eth_rx(rt_device_t device)
{
    int recv_len = 0;
    struct pbuf *pbuf = RT_NULL;
    recv_len = e1000_recv(raw_recv_buff);
    if(recv_len > 0)
    {   
        pbuf = pbuf_alloc(PBUF_LINK, recv_len, PBUF_RAM);
        rt_memcpy(pbuf->payload, raw_recv_buff, recv_len);
    }
    return pbuf;
}

static rt_err_t e1000_eth_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
    case NIOCTL_GADDR:
        if (args)
            rt_memcpy(args, _emac.enetaddr, 6);
        else
            return -RT_ERROR;
        break;
    default:
        break;
    }
    return RT_EOK;
}

static void rt_hw_eth_isr(int irqno, void *param)
{
    regs[E1000_ICR];
    eth_device_ready(&_emac.parent);
}

static int e1000_hw_init(void)
{
    //filter by qemu's MAC address, 52:54:00:12:34:56
    _emac.enetaddr[0] = 0x52;
    _emac.enetaddr[1] = 0x54;
    _emac.enetaddr[2] = 0x00;
    _emac.enetaddr[3] = 0x12;
    _emac.enetaddr[4] = 0x34;
    _emac.enetaddr[5] = 0x56;

    _emac.parent.parent.init       = e1000_bus_init;
    _emac.parent.parent.open       = RT_NULL;
    _emac.parent.parent.close      = RT_NULL;
    _emac.parent.parent.read       = RT_NULL;
    _emac.parent.parent.write      = RT_NULL;
    _emac.parent.parent.control    = e1000_eth_control;

    _emac.parent.parent.user_data  = RT_NULL;
    _emac.parent.eth_rx            = rt_eth_rx;
    _emac.parent.eth_tx            = rt_eth_tx;

    /* register ETH device */
    eth_device_init(&(_emac.parent), "e0");
    eth_device_linkchange(&_emac.parent, RT_TRUE);

    rt_hw_interrupt_install(E1000_IRQ, rt_hw_eth_isr, NULL, "eth");
    rt_hw_interrupt_umask(E1000_IRQ);

    return 0;
}
INIT_COMPONENT_EXPORT(e1000_hw_init);
