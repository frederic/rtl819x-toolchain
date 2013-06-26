/*
 * Copyright (C) 2006 Free Software Initiative of Japan
 *
 * Author: NIIBE Yutaka  <gniibe at fsij.org>
 *
 * This file can be distributed under the terms and conditions of the
 * GNU General Public License version 2 (or later).
 *
 */

#include <usb.h>
#include <stdio.h>

#define USB_RT_HUB			(USB_TYPE_CLASS | USB_RECIP_DEVICE)
#define USB_RT_PORT			(USB_TYPE_CLASS | USB_RECIP_OTHER)
#define USB_PORT_FEAT_POWER		8
#define USB_PORT_FEAT_INDICATOR         22
#define USB_DIR_IN			0x80		/* to host */

#define COMMAND_SET_LED   0
#define COMMAND_SET_POWER 1
#define HUB_LED_GREEN 2

static void
usage (const char *progname)
{
  fprintf (stderr, "Usage: %s [-b BUSNUM] [-d DEVNUM] [-P PORT] [{-l [VALUE]}|{-p [VALUE]}]\n", progname);
}

/*
 * HUB-CTRL  -  program to control port power/led of USB hub
 *
 *   # hub-ctrl -b 1 -d 2 -P 1 -p            // Power off at port 1
 *   # hub-ctrl -b 1 -d 2 -P 1 -p 1          // Power on at port 1
 *   # hub-ctrl -b 1 -d 2 -P 2 -l            // LED on at port 1
 *
 * Requirement: USB hub which implements port power control / indicator control
 *      Work fine:
 *              Elecom's U2H-G4S: www.elecom.co.jp (indicator depends on power)
 *		Sanwa Supply's USB-HUB14GPH: www.sanwa.co.jp (indicators don't)
 *		Targus, Inc.'s PAUH212: www.targus.com (indicators don't)
 *
 */
int
main (int argc, const char *argv[])
{
  int busnum = 0, devnum = 0;
  int cmd = COMMAND_SET_POWER;
  int port = 1;
  int value = 0;
  struct usb_bus *busses;
  struct usb_bus *bus;
  int i;

  for (i = 1; i < argc; i++)
    if (argv[i][0] == '-')
      switch (argv[i][1])
	{
	case 'b':
	  if (++i >= argc)
	    {
	      usage (argv[0]);
	      exit (1);
	    }
	  busnum = atoi (argv[i]);
	  break;

	case 'd':
	  if (++i >= argc)
	    {
	      usage (argv[0]);
	      exit (1);
	    }
	  devnum = atoi (argv[i]);
	  break;

	case 'P':
	  if (++i >= argc)
	    {
	      usage (argv[0]);
	      exit (1);
	    }
	  port = atoi (argv[i]);
	  break;

	case 'l':
	  cmd = COMMAND_SET_LED;
	  if (++i < argc)
	    value = atoi (argv[i]);
	  else
	    value = HUB_LED_GREEN;
	  break;

	case 'p':
	  cmd = COMMAND_SET_POWER;
	  if (++i < argc)
	    value = atoi (argv[i]);
	  else
	    value= 0;
	  break;

	default:
	  usage (argv[0]);
	  exit (1);
	}

  usb_init();
  usb_find_busses();
  usb_find_devices();

  busses = usb_get_busses();
  if (busses == NULL)
    {
      perror ("failed to access USB");
      exit (1);
    }

  bus = busses->next;
  for (bus = busses; bus; bus = bus->next)
    {
      struct usb_device *dev;
      usb_dev_handle *uh;
      int result = 0;
      int request, feature, index;

      if (atoi (bus->dirname) != busnum)
	continue;

      for (dev = bus->devices; dev; dev = dev->next)
	{
	  if (dev->devnum != devnum)
	    continue;

	  if (dev->descriptor.bDeviceClass != USB_CLASS_HUB)
	    {
	      fprintf (stderr, "Device is not hub.\n");
	      exit (1);
	    }

	  uh = usb_open (dev);

	  if (uh != NULL)
	    {
	      unsigned char buf[1024];
	      int len;

	      if ((len = usb_control_msg (uh, 
					  USB_DIR_IN | USB_RT_HUB,
					  USB_REQ_GET_DESCRIPTOR,
					  USB_DT_HUB << 8, 0, 
					  buf, 1024, 1000)) > 0)
		{
		  /* Should check wHubCharacteristic for power control */
		  ;
		}
	      else
		perror ("usb_get_descriptor");

	      if (cmd == COMMAND_SET_POWER)
		if (value)
		  {
		    request = USB_REQ_SET_FEATURE;
		    feature = USB_PORT_FEAT_POWER;
		    index = port;
		  }
		else
		  {
		    request = USB_REQ_CLEAR_FEATURE;
		    feature = USB_PORT_FEAT_POWER;
		    index = port;
		  }
	      else
		{
		  request = USB_REQ_SET_FEATURE;
		  feature = USB_PORT_FEAT_INDICATOR;
		  index = (value << 8) | port;
		}

	      if (usb_control_msg (uh, USB_RT_PORT, request, feature, index,
				   NULL, 0, 1000) < 0)
		{
		  perror ("failed to control.\n");
		  result = 1;
		}
	    }
	  else
	    {
	      perror ("failed to open");
	      result = 1;
	    }

	  usb_release_interface(uh, 0);
	  usb_close (uh);
	  exit (result);
	}
    }

  fprintf (stderr, "Device not found.\n");
  exit (1);
}
