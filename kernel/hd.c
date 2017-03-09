#include "const.h"
#include "type.h"
#include "protect.h"
#include "string.h"
#include "tty.h"
#include "console.h"
#include "hd.h"
#include "fs.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "rbtree.h"
#include "memory.h"

#define DRV_OF_DEV(dev) (dev <= MAX_PRIM ? dev / NR_PRIM_PER_DRIVER : (dev - MINOR_HD1A) / NR_SUB_PER_DRIVER)

PRIVATE	u8		hd_status;
PRIVATE	u8		hdbuf[SECTOR_SIZE * 2];
PRIVATE	hd_info	hd_infos[1];

PRIVATE int waitfor(int mask, int val, int timeout)
{
	int t = get_ticks();

	while(((get_ticks( ) - t) * 1000/HZ) < timeout)
	{
		if((in_byte(REG_STATUS) & mask) == val)
		{
			return 1;
		}
	}
	return 0;
}

PRIVATE void interrupt_wait()
{
	MESSAGE msg;
	send_recv(RECEIVE, INTERRUPT, &msg);
}

PRIVATE void hd_cmd_out(hd_cmd* cmd)
{
	if(!waitfor(STATUS_BSY, 0, HD_TIMEOUT))
	{
		panic("hd error.");
	}

	out_byte(REG_DEV_CTRL, 0);
	out_byte(REG_FEATURES, cmd->features);
	out_byte(REG_NSECTOR, cmd->count);
	out_byte(REG_LBA_LOW, cmd->lba_low);
	out_byte(REG_LBA_MID, cmd->lba_mid);
	out_byte(REG_LBA_HIGH, cmd->lba_high);
	out_byte(REG_DEVICE, cmd->device);

	out_byte(REG_CMD, cmd->command);
}

PRIVATE void get_part_table(int driver, int sect_id, part_entry *entry)
{
	hd_cmd cmd;
	cmd.features = 0;
	cmd.count = 1;
	cmd.lba_low = sect_id & 0xFF;
	cmd.lba_mid = (sect_id >> 8) & 0xFF;
	cmd.lba_high = (sect_id >> 16) & 0xFF;
	cmd.device = MAKE_DEVICE_REG(1, driver, (sect_id >> 24) & 0xF);

	cmd.command = ATA_READ;
	hd_cmd_out(&cmd);
	interrupt_wait();

	port_read(REG_DATA, hdbuf, SECTOR_SIZE);
	memcpy(entry, hdbuf + PART_TABLE_OFFSET, sizeof(part_entry) * NR_PART_PER_DRIVER);
}

PRIVATE void partition(int device, int style)
{
	int i;
	int driver = DRV_OF_DEV(device);
	hd_info *hdi = &hd_infos[driver];

	part_entry part_table[NR_SUB_PER_DRIVER];

	if(style == P_PRIMARY)
	{
		get_part_table(driver, driver, part_table);

		int nr_prim_parts = 0;
		for(i = 0; i < NR_PART_PER_DRIVER; ++i)
		{
			if(part_table[i].sys_id == NO_PART)
			{
				continue;
			}

			++nr_prim_parts;
			int dev_id = i + 1;
			hdi->primary[dev_id].base = part_table[i].start_sect;
			hdi->primary[dev_id].size = part_table[i].nr_sects;

			if(part_table[i].sys_id == EXT_PART)
			{
				partition(device + dev_id, P_EXTENDED);
			}
		}
		assert(nr_prim_parts != 0);
	}  
	else if (style == P_EXTENDED)
	{
		int j = device % NR_PRIM_PER_DRIVER;
		int ext_start_sect = hdi->primary[j].base;
		int s = ext_start_sect;
		int nr_1st_sub = (j - 1) * NR_SUB_PER_PART;

		//printk("nr_1st_sub: %d\n", nr_1st_sub);

		for(i = 0; i < NR_SUB_PER_PART; ++i)
		{
			int dev_id = nr_1st_sub + i;

			get_part_table(driver, s, part_table);

			hdi->logical[dev_id].base = s + part_table[0].start_sect;
			hdi->logical[dev_id].size = part_table[0].nr_sects;			

			s = ext_start_sect + part_table[1].start_sect;

			if(part_table[1].sys_id == NO_PART)
				break;
		}
	}
	else
	{
		assert(0);
	}
}

PRIVATE void print_identify_info(u16* hdinfo)
{
	int i, k;
	char s[64];

	struct iden_info_ascii
	{
		int idx;
		int len;
		char *desc;
	}iinfo[] = {{10, 20, "[HD] SN"}, {27, 40, "[HD] Model"}};

	for(k = 0; k < sizeof(iinfo) / sizeof(iinfo[0]); ++k)
	{
		char *p = (char*)&hdinfo[iinfo[k].idx];
		for(i = 0; i < iinfo[k].len/2; ++i)
		{
			s[i*2+1] = *p++;
			s[i*2] = *p++;
		}
		s[i*2] = 0;
		printk("%s: %s\n", iinfo[k].desc, s);
	}

	int capabilities = hdinfo[49];
	printk("[HD] LBA supported: %s\n",
	       (capabilities & 0x0200) ? "Yes" : "No");

	int cmd_set_supported = hdinfo[83];
	printk("[HD] LBA48 supported: %s\n",
	       (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	printk("[HD] HD size: %dMB\n", sectors * 512 / 1000000);
}

PRIVATE void hd_identify(int driver)
{
	hd_cmd cmd;
	cmd.device = MAKE_DEVICE_REG(0, driver, 0);
	cmd.command = ATA_IDENTIFY;
	hd_cmd_out(&cmd);
	interrupt_wait();
	port_read(REG_DATA, hdbuf, SECTOR_SIZE);

	print_identify_info((u16*)hdbuf);

	u16* hdinfo = (u16*)hdbuf;

	hd_infos[driver].primary[0].base = 0;
	hd_infos[driver].primary[0].size = ((int)hdinfo[61] << 16) + hdinfo[60];
}

PRIVATE void print_hdinfo(hd_info* hdi)
{
	int i;
	for(i = 0; i < NR_PART_PER_DRIVER + 1; ++i)
	{
		printk("[HD] %sPART_%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
			i == 0 ? "  " : "    ",
			i,
			hdi->primary[i].base,
			hdi->primary[i].base,
			hdi->primary[i].size,
			hdi->primary[i].size);
	}
	for(i = 0; i < NR_SUB_PER_DRIVER; ++i)
	{
		if(hdi->logical[i].size == 0)
		{
			continue;
		}
		printk("[HD] \t\t%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
			i,
			hdi->logical[i].base,
			hdi->logical[i].base,
			hdi->logical[i].size,
			hdi->logical[i].size);
	}
}

PRIVATE void hd_init()
{
	int i;

	printk("[HD] Hard Disk Information:\n");

	u8* nr_drivers = (u8*)(0x475); 
	printk("[HD] Number of Drivers:%d.\n", *nr_drivers);
	assert(*nr_drivers);

	put_irq_handler(AT_WINI_IRQ, hd_handler);
	enable_irq(CASCADE_IRQ);
	enable_irq(AT_WINI_IRQ);

	for(i = 0; i < (sizeof(hd_infos) / sizeof(hd_infos[0])); ++i)
	{
		memset(&hd_infos[i], 0, sizeof(hd_infos[0]));
	}
	hd_infos[0].open_cnt = 0;
}

PRIVATE void hd_open(int device)
{
	int driver = DRV_OF_DEV(device);
	assert(driver == 0);

	hd_identify(driver);
	
	if(hd_infos[driver].open_cnt++ == 0)
	{
		partition(driver * (NR_PART_PER_DRIVER + 1), P_PRIMARY);
		print_hdinfo(&hd_infos[driver]);
	}
}

PRIVATE void hd_close(int device)
{
	int driver = DRV_OF_DEV(device);
	assert(driver == 0);

	--hd_infos[driver].open_cnt;
}

PRIVATE void hd_rdwt(MESSAGE *msg)
{
	int driver = DRV_OF_DEV(msg->DEVICE);

	u64 pos = msg->POSITION;
	assert((pos >> SECTOR_SIZE_SHIFT) < (1 << 31));
	assert((pos & 0x1FF) == 0);

	u32 sect_id = (u32)(pos >> SECTOR_SIZE_SHIFT);
	int logicalIdx = (msg->DEVICE - MINOR_HD1A) % NR_SUB_PER_DRIVER;
	sect_id += msg->DEVICE < MAX_PRIM ? 
		hd_infos[driver].primary[msg->DEVICE].base : 
		hd_infos[driver].logical[logicalIdx].base;

	hd_cmd cmd;
	cmd.features= 0;
	cmd.count	= (msg->CNT + SECTOR_SIZE - 1) / SECTOR_SIZE;
	cmd.lba_low	= sect_id & 0xFF;
	cmd.lba_mid	= (sect_id >> 8) & 0xFF;
	cmd.lba_high= (sect_id >> 16) & 0xFF;
	cmd.device	= MAKE_DEVICE_REG(1, driver, (sect_id >> 24) & 0xF);
	cmd.command	= (msg->type == DEV_READ) ? ATA_READ : ATA_WRITE;
	hd_cmd_out(&cmd);

	//printk("%d 0x%xla: " , msg->PROC_ID, msg->BUF);

	int bytes_left = msg->CNT;
	void* la = (void*)va2la(msg->PROC_ID, msg->BUF);

	//printk("0x%x\n", la);

	while(bytes_left)
	{
		int bytes = min(SECTOR_SIZE, bytes_left);
		if(msg->type == DEV_READ)
		{
			interrupt_wait();
			port_read(REG_DATA, hdbuf, SECTOR_SIZE);
			memcpy(la, (void*)va2la(TASK_HD, hdbuf), bytes);
		}
		else
		{
			if(!waitfor(STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT))
			{
				panic("HD Writing Error.");
			}
			port_write(REG_DATA, la, bytes);
			interrupt_wait();
		}
		bytes_left -= SECTOR_SIZE;
		la += SECTOR_SIZE;
	}

}

PRIVATE void hd_ioctl(MESSAGE* msg)
{
	int device = msg->DEVICE;
	int driver = DRV_OF_DEV(device);

	hd_info* hdi = &hd_infos[driver];

	if(msg->REQUEST == DIOCTL_GET_GEO)
	{
		void* dst = va2la(msg->PROC_ID, msg->BUF);
		void* src = va2la(TASK_HD,
					device < MAX_PRIM?
					&hdi->primary[device]:
					&hdi->logical[(device - MINOR_HD1A) % NR_SUB_PER_DRIVER]);
		memcpy(dst, src, sizeof(part_info));
	}
	else
	{
		assert(0);
	}
}

PUBLIC void task_hd()
{
	MESSAGE msg;

	hd_init();

	while(1)
	{
		send_recv(RECEIVE, ANY, &msg);

		int src = msg.source;

		switch(msg.type)
		{
		case DEV_OPEN:
			hd_open(msg.DEVICE);
			break;
		case DEV_CLOSE:
			hd_close(msg.DEVICE);
			break;
		case DEV_READ:
		case DEV_WRITE:
			hd_rdwt(&msg);
			break;
		case DEV_IOCTL:
			hd_ioctl(&msg);
			break;
		default:
			dump_msg("HD Driver::unknown msg: ",&msg);
			spin("File System (invalid msg.type)");
			break;
		}

		send_recv(SEND, src, &msg);
	}
}

PUBLIC void hd_handler(int irq)
{
	hd_status = in_byte(REG_STATUS);

	inform_int(TASK_HD);
}

