#include <machine/rtems-bsd-kernel-space.h>
/*
 * Copyright 2015 Andrew Turner.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <rtems/bsd/sys/param.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/callout.h>
#include <sys/condvar.h>
#include <sys/module.h>

#ifndef __rtems__
#include <dev/ofw/ofw_bus_subr.h>
#endif

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>

#include <dev/usb/usb_busdma.h>
#include <dev/usb/usb_process.h>

#include <dev/usb/usb_controller.h>
#include <dev/usb/usb_bus.h>

#include <dev/usb/controller/dwc_otg.h>
#include <dev/usb/controller/dwc_otg_fdt.h>

#ifndef __rtems__
#include <arm/broadcom/bcm2835/bcm2835_mbox_prop.h>
#else
#include <bsp/vc.h>
#endif

static device_probe_t bcm283x_dwc_otg_probe;
static device_attach_t bcm283x_dwc_otg_attach;

static int
bcm283x_dwc_otg_probe(device_t dev)
{

	#ifndef __rtems__
	if (!ofw_bus_status_okay(dev))
		return (ENXIO);

	if (!ofw_bus_is_compatible(dev, "broadcom,bcm2835-usb"))
		return (ENXIO);
	#endif
		
	device_set_desc(dev, "DWC OTG 2.0 integrated USB controller (bcm283x)");

	return (BUS_PROBE_VENDOR);
}

static int
bcm283x_dwc_otg_attach(device_t dev)
{
	#ifndef __rtems__
	bcm2835_mbox_set_power_state(dev, BCM2835_MBOX_POWER_ID_USB_HCD, TRUE);
    #else
    bcm2835_set_power_state_entries entries;
    entries.dev_id = bcm2835_mailbox_power_udid_usb_hcd;
    entries.state = BCM2835_MAILBOX_SET_POWER_STATE_REQ_ON
                    | BCM2835_MAILBOX_SET_POWER_STATE_REQ_WAIT;
    int ret = bcm2835_mailbox_set_power_state(&entries);
    if (ret) return ENXIO;
	#endif
	return (dwc_otg_attach(dev));
}

static device_method_t bcm283x_dwc_otg_methods[] = {
	/* bus interface */
	DEVMETHOD(device_probe, bcm283x_dwc_otg_probe),
	DEVMETHOD(device_attach, bcm283x_dwc_otg_attach),

	DEVMETHOD_END
};

static devclass_t bcm283x_dwc_otg_devclass;

DEFINE_CLASS_1(bcm283x_dwcotg, bcm283x_dwc_otg_driver, bcm283x_dwc_otg_methods,
    sizeof(struct dwc_otg_fdt_softc), dwc_otg_driver);
#ifndef __rtems__
DRIVER_MODULE(bcm283x_dwcotg, simplebus, bcm283x_dwc_otg_driver,
    bcm283x_dwc_otg_devclass, 0, 0);
#else
DRIVER_MODULE(bcm283x_dwcotg, nexus, bcm283x_dwc_otg_driver,
    bcm283x_dwc_otg_devclass, 0, 0);
#endif
MODULE_DEPEND(bcm283x_dwcotg, usb, 1, 1, 1);
