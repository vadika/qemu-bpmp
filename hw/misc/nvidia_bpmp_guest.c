#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/nvidia_bpmp_guest.h"

#define TYPE_NVIDIA_BPMP_GUEST "nvidia_bpmp_guest"
typedef struct NvidiaBpmpGuestState NvidiaBpmpGuestState;
DECLARE_INSTANCE_CHECKER(NvidiaBpmpGuestState, NVIDIA_BPMP_GUEST, TYPE_NVIDIA_BPMP_GUEST)

#define WRITE_ID 	0x7F
#define MEM_SIZE 0x100
#define HOST_DEVICE_PATH "/dev/bpmp-host"
#define MESSAGE_SIZE 0x0C   

// qemu_log_mask(LOG_UNIMP, "%s: \n", __func__ );

struct NvidiaBpmpGuestState {
	SysBusDevice parent_obj;
	MemoryRegion iomem;
	int host_device_fd;
	uint64_t mem[MEM_SIZE];
};



// Device memory map: 
// Base address in virt machine  memory is 
//S
// 0x090c0000 +
//     0x000 \ Request buffer
//     0x07F /
//
// Data should be aligned to 64bit paragraph. 
// Protocol is:
// 1) write data buffer to 0x000-0x00b (48*bytes, 12*64bit words)
// 2) start operation by writing anything to address 0x07F
// 3) read response from 0x000-0x00b
//


static uint64_t nvidia_bpmp_guest_read(void *opaque, hwaddr addr, unsigned int size)
{
	NvidiaBpmpGuestState *s = opaque;

	if (addr >= MEM_SIZE)
		return 0xDEADBEEF;
	
	return s->mem[addr];
}



static void nvidia_bpmp_guest_write(void *opaque, hwaddr addr, uint64_t data, unsigned int size)
{
	NvidiaBpmpGuestState *s = opaque;
	int ret;

	if (addr >= MEM_SIZE)
		return;

	switch (addr) {
	case WRITE_ID:
    	ret = write(s->host_device_fd, s->mem, MESSAGE_SIZE*sizeof(uint64_t) ); // Send the data to the host module
    	if (ret < 0)
    	{
        	qemu_log_mask(LOG_UNIMP, "%s: Failed to write the host device..\n", __func__ );
        	return;
    	}
		break;

	default:
		s->mem[addr]=data;
	}

	return;
}


static const MemoryRegionOps nvidia_bpmp_guest_ops = {
	.read = nvidia_bpmp_guest_read,
	.write = nvidia_bpmp_guest_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

static void nvidia_bpmp_guest_instance_init(Object *obj)
{
	NvidiaBpmpGuestState *s = NVIDIA_BPMP_GUEST(obj);

	/* allocate memory map region */
	memory_region_init_io(&s->iomem, obj, &nvidia_bpmp_guest_ops, s, TYPE_NVIDIA_BPMP_GUEST, 0x100);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    s->host_device_fd = open(HOST_DEVICE_PATH, O_RDWR); // Open the device with read/write access
    
	if (s->host_device_fd < 0)
    {
        qemu_log_mask(LOG_UNIMP, "%s: Failed to open the host device..\n", __func__ );
        return;
    }}

/* create a new type to define the info related to our device */
static const TypeInfo nvidia_bpmp_guest_info = {
	.name = TYPE_NVIDIA_BPMP_GUEST,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(NvidiaBpmpGuestState),
	.instance_init = nvidia_bpmp_guest_instance_init,
};

static void nvidia_bpmp_guest_register_types(void)
{
    type_register_static(&nvidia_bpmp_guest_info);
}

type_init(nvidia_bpmp_guest_register_types)

/*
 * Create the Nvidia BPMP guest device.
 */
DeviceState *nvidia_bpmp_guest_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_NVIDIA_BPMP_GUEST);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
}
