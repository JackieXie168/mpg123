/*
 * MPEG Audio DSP Player Linux Module v0.1
 * copyrights (c) 1996 by Michael Hipp
 * 
 * Read the README file before using the software!!
 */

/*
 * gcc -O2 -Wall -m486 -fomit-frame-pointer -DMODULE -D__KERNEL__ -c dsp.c
 */

#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/malloc.h>
#include <linux/string.h>
#include <asm/system.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <linux/errno.h>

#include "dsp_ld.h"

#define BUFFER_SIZE 16384

#define DSP_MAJOR 62

struct cd {
    unsigned char bufs[2][BUFFER_SIZE];
    int rbuf;
    int wbuf;
    int rpos;
    int wpos;
    int state;
	int wsize,rsize;
    struct wait_queue *waitq;
	int base_addr;
	int dma;
	int irq;
	int interrupt;
};


static int dsp_open_dev(struct inode * inode, struct file * file);
static void dsp_release_dev(struct inode * inode, struct file * file);
static int dsp_read_dev(struct inode * inode, struct file * file,char *,int);
static int dsp_write_dev(struct inode * inode, struct file * file,const char *,int);
static int dsp_reset(struct cd *p);
static int dsp_write(struct cd *d,unsigned char *buf,int count,int source);
static int dsp_probe(struct cd *p);
static void dsp_interrupt(int irq, void *dev_id, struct pt_regs *regs);
static void dsp_init(struct cd *p, unsigned char *,int m);

#define PSS_DATA      0x00
#define PSS_STATUS    0x02
#define PSS_CONTROL   0x02
#define PSS_ID_VERS   0x04

#define PSS_CONFIG 0x10
#define WSS_CONFIG 0x12
#define SB_CONFIG 0x14
#define CD_CONFIG 0x16
#define MIDI_CONFIG 0x18

#define DSP_OPEN		0x1
#define DSP_WRITE_READY	0x2
#define DSP_RELEASE_WAIT 0x4

static struct file_operations dsp_fops = {
        NULL,
        NULL,           /* read */
        dsp_write_dev,	/* write */
        NULL,           /* readdir */
        NULL,           /* select */
        NULL,
        NULL,           /* lp_mmap */
        dsp_open_dev,
        dsp_release_dev
};


int debug = 0;

#define NUM_DEV 1
static struct cd devs[NUM_DEV];

static int minor2dspnum[3] = { -1 , -1 , -1 };

static int dsp_open_dev(struct inode * inode, struct file * file)
{
	unsigned int minor = MINOR(inode->i_rdev);
	int dspnum;
	int i;

	if(debug)
		printk(KERN_DEBUG "dsp_open %d\n",minor);

	if(minor < 0 || minor > 2) {
		return -ENODEV;
	}

	for(i=0;i<3;i++)	/* currently, only one DSP supported */
		if(minor2dspnum[i] >= 0)
			return -EAGAIN;

	minor2dspnum[minor] = 0;

	dspnum = minor2dspnum[minor];
	if(devs[dspnum].state)
		return -EBUSY;

    MOD_INC_USE_COUNT;

	devs[dspnum].state = DSP_OPEN;
	devs[dspnum].rsize = devs[dspnum].wsize = BUFFER_SIZE;
	devs[dspnum].rpos = 0;
	devs[dspnum].wpos = 0;
	devs[dspnum].rbuf = 0;
	devs[dspnum].wbuf = 0;

	dsp_init(&devs[dspnum],dspcode,sizeof(dspcode));

	return 0;
}

static void dsp_release_dev(struct inode * inode, struct file * file)
{
	unsigned int minor = MINOR(inode->i_rdev);
	int dspnum;

	if(debug)
		printk(KERN_DEBUG "dsp_release %d\n",minor);

	if(minor < 0 || minor > 2) {
		return;
	}
	dspnum = minor2dspnum[minor];

	if(dspnum < 0 || !devs[dspnum].state)
		return;

	if(devs[dspnum].wpos > 0 && (devs[dspnum].wpos < devs[dspnum].wsize) )	/* buffer not empty */
	{
		char buf[1] = { 0 };
		devs[dspnum].wsize = devs[dspnum].wpos + 1;
		dsp_write(&devs[dspnum],buf,1,1);
	}

	devs[dspnum].state = DSP_RELEASE_WAIT;

	while(devs[dspnum].wbuf != devs[dspnum].rbuf)
		interruptible_sleep_on(&(devs[dspnum].waitq));

	minor2dspnum[minor] = -1;
	devs[dspnum].state = 0;
#if 0
	dsp_reset(devs[dspnum].dev);
	wake_up_interruptible(&devs[dspnum].waitq);
#endif

	MOD_DEC_USE_COUNT;

}

static int dsp_write_dev(struct inode * inode, struct file * file,const char *buf , int count)
{
	unsigned int minor = MINOR(inode->i_rdev);
	struct cd *d;
	int r,dspnum;

	if(minor < 0 || minor > 2)
		return -ENODEV;

	dspnum = minor2dspnum[minor];

    if(dspnum < 0 || !devs[dspnum].state)
        return -ENXIO;

    if ((r = verify_area(VERIFY_READ, (void *) buf, count)))
       return r;

	d = &devs[dspnum];

	return dsp_write(d,(unsigned char *) buf,count,0);
}


static int dsp_write(struct cd *d,unsigned char *buf,int count,int source)
{
	int cnt1 = count;
	int pos = 0;

	while(cnt1) {
		if(d->wpos < d->wsize) {
			int len = d->wsize - d->wpos;
			if(len > cnt1)
				len = cnt1;
			if(debug)
				printk("got %d bytes\n",len);
			if(source)
	        	memcpy(d->bufs[d->wbuf]+d->wpos,buf+pos,len);
			else
	        	memcpy_fromfs(d->bufs[d->wbuf]+d->wpos,buf+pos,len);
			pos += len;
			d->wpos += len;	
			cnt1 -= len;
		}
		if(d->wpos >= d->wsize) {
			if(d->wbuf == d->rbuf) {
				long flags;
				d->wbuf++;
				d->wbuf &= 0x1;
				d->wpos = 0;
				d->rpos = 0;
				d->rsize = d->wsize;
				d->wsize = BUFFER_SIZE;
				save_flags(flags);
				cli();
				if( (inw(d->base_addr+PSS_STATUS) & 0x8000) ) {
					unsigned short w;
					w = d->bufs[d->rbuf][d->rpos]<<8;
					w |= d->bufs[d->rbuf][d->rpos+1];
					if(debug)
						printk("initial write %02x\n",w);
#if 0
					outw(0x8000,d->base_addr+PSS_CONTROL);
#endif
					outw(w,d->base_addr+PSS_DATA);
					d->rpos+=2;
				}
				restore_flags(flags);
			}
			else if(cnt1 > 0){
		        interruptible_sleep_on(&(d->waitq));
				if(debug)
					printk("waked up %d %d\n",d->state,d->wpos);
		        if(!d->state || d->wpos >= d->wsize)
		            return 0;
			}
		}
	}

	return count;
}


static int dsp_read_dev(struct inode * inode, struct file * file, char *buf , int count)
{
	unsigned int minor = MINOR(inode->i_rdev);
	int dspnum;

    if(minor < 0 || minor > 2) {
        return -ENODEV;
    }
	dspnum = minor2dspnum[minor];
    if(dspnum < 0 || !devs[dspnum].state)
		return -ENXIO;

	return 0;
}

int dsp_probe(struct cd *p)
{
	static int irqtab[] = { -1,-1,-1,0x1,-1,0x2,-1,0x3,-1,0x4,0x5,0x6,0x7 ,-1,-1,-1 };
	static int dmatab[] = { 0x1,0x2,-1,0x3,-1,0x5,0x6,0x7 };
	unsigned short w;
	int dspnum = 0;

	p = &devs[dspnum];

	outw(0,p->base_addr + PSS_CONFIG);
	outw(0,p->base_addr + WSS_CONFIG);
	outw(0,p->base_addr + SB_CONFIG);
	outw(0,p->base_addr + CD_CONFIG);
	outw(0,p->base_addr + MIDI_CONFIG);
	
	/* Grab the region so that no one else tries to probe our ioports. */

	if(p->dma <= 7 && dmatab[p->dma] < 0) {
		printk("illegal DMA %d\n",p->dma);
		return -EAGAIN;
	}
	if(p->irq < 0 || p->irq > 15 || irqtab[p->irq] < 0) {
		printk("illegal IRQ %d\n",p->irq);
		return -EAGAIN;
	}

	if(check_region(p->base_addr, 0x20 )) {
		printk("dsp: region %#x-%#x in use.\n",p->base_addr,p->base_addr+0x20);
#if 0
		return -EAGAIN;
#endif
	}
	request_region(p->base_addr, 0x20 ,"dsp");
	if (request_irq(p->irq, &dsp_interrupt, 0, "dsp", NULL)) {
		printk("Can't get IRQ %d\n",p->irq);
		release_region(p->base_addr,0x20);
		return -EAGAIN;
	}	
	if(p->dma <= 7) {
		if(request_dma(p->dma,"dsp")) {
			printk("Can'T get DMA %d\n",p->dma);
			free_irq(p->irq,NULL);
			release_region(p->base_addr,0x20);
			return -EAGAIN;
		}
		w = (irqtab[p->irq]<<3) | dmatab[p->dma];
	}
	else {
		w = (irqtab[p->irq]<<3);
	}

	irq2dev_map[p->irq] = p;

	outw(w,p->base_addr+PSS_CONFIG);

	dsp_init(p,dspcode,sizeof(dspcode));

	return 0;
}

static void dsp_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
	struct cd *p = (struct cd *)(irq2dev_map[irq]);
	int stat;
	int fail = 32;
	int mask = 0xf000;

	if (p == NULL) {
		printk(KERN_WARNING "%s: irq %d for unknown device.\n", "dsp" , irq);
		return;
	}

	if(set_bit(0,&p->interrupt))
		return;

	stat=inw(p->base_addr+PSS_STATUS);
    if(stat & 0x3000) {
       outw(0x0,p->base_addr+0x4);
    }

    while( (stat & mask) && fail ) {
		fail--;
    	if(stat & 0x4000)
	    {
		  int k = inw(p->base_addr+PSS_DATA);
          printk("dsp->%04x ",k);
        }

        if(stat & 0x8000)
        {
		  if(p->state && p->wbuf != p->rbuf) {
              unsigned short w;
              w = p->bufs[p->rbuf][p->rpos]<<8;
              w |= p->bufs[p->rbuf][p->rpos+1];
	          outw(w,p->base_addr+PSS_DATA);
	          p->rpos+=2;
			  if(p->rpos >= p->rsize) {
				p->rbuf++;
				p->rbuf &= 0x1;
				p->rpos = 0;
				if(p->wpos >= p->wsize) {
					p->rsize = p->wsize;
					p->wbuf++;
					p->wbuf &= 0x1;
					p->wpos = 0;
					p->wsize = BUFFER_SIZE;
				}
				wake_up_interruptible(&(p->waitq));
			  }
		  }
		  else {
#if 0
			mask = 0x7000;
#endif
		  }
        }
		stat=inw(p->base_addr+PSS_STATUS);
	}

	p->interrupt = 0;
	return;
}

static int dsp_reset(struct cd *p)
{
  int i;
  outw (0x2000,p->base_addr + PSS_CONTROL);

  for (i = 0; i < 32768 ; i++)
    inw (p->base_addr + PSS_CONTROL);

  outw (0x0000,p->base_addr + PSS_CONTROL);

  return 1;
}

static void dsp_init(struct cd *p,unsigned char *block,int len )
{
   int i,j;
	unsigned char *pnt1 = block;

   dsp_reset(p);
   outw(*block++,p->base_addr+PSS_DATA);
   dsp_reset(p);

   for(i=0;;i++)
   {
     for(j=0;j<100000;j++)
       if(inw(p->base_addr+PSS_STATUS) & 0x800)
         break;
     if(j==100000)
     {
       printk("dsp TIMEOUT.\n");
       break;
     }
     if(i == len-1)
       break;
     outw(*block++,p->base_addr+PSS_DATA);
   }
   printk("downloaded code: %d\n",block-pnt1);

#if 0
   outw(0x4000,p->base_addr+PSS_CONTROL);
#endif

   outw(0,p->base_addr+PSS_DATA);

}

#ifdef MODULE

static int io = 0x220;
static int irq = 10;
static int dma = 255;

int init_module(void)
{
	int result,i;

	/* Copy the parameters from insmod into the device structure. */
	devs[0].base_addr = io;
	devs[0].irq       = irq;
	devs[0].dma       = dma;

    for(i=0;i<NUM_DEV;i++)
    {
        devs[i].state = 0;
        devs[i].waitq = NULL;
    }

	if (register_chrdev(DSP_MAJOR, "dsp", &dsp_fops)) {
		printk(KERN_WARNING "dsp: Could not register control devices\n");
		return -EIO;
	}

	result = dsp_probe(&devs[0]);
	if(result) {
		printk(KERN_ERR "dsp: Can't init board!\n");
		unregister_chrdev(DSP_MAJOR,"dsp");
		return result;
	}

	return result;
}

void
cleanup_module(void)
{
	dsp_reset(&devs[0]);

	free_irq(devs[0].irq, NULL);
	if(devs[0].dma <= 7)
		free_dma(devs[0].dma);
	irq2dev_map[devs[0].irq] = 0;

	unregister_chrdev(DSP_MAJOR,"dsp");

	if(devs[0].base_addr)
		release_region(devs[0].base_addr, 0x20);
}

#endif /* MODULE */


