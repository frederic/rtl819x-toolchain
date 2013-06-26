1. How to enable Boa web server
1) $ make config
2) The configuration menu is shown as follows.
   Select "Config kernel" and "Config users", and then Exit the menuconfig.

                   --- select components               
                       Selected Target (rtl8196c)  --->
                       Selected Kernel (linux-2.6.30)  --->  
                       Selected Busybox (busybox-1.13)  ---> 
                       Selected toolchain (rsdk-1.3.6-4181-EB-2.6.30-0.9.30)  --->
                   --- rtl8196c         
                       Selected Target of SDK (11nRouter_GW)  --->                
                       Selected Board Configuration (SPI flash + Squashfs)  --->  
                       IC Test Configuration  --->     
                   --- config components               
                   [*] Config kernel    
                   [*] Config users     
                   [ ] Config busybox   
                   [ ] Load default settings           
                   [ ] Save default settings           
                   ---                  
                       Load an Alternate Configuration File  
                       Save an Alternate Configuration File           

3) If you turn on APMIB_SHARED feature (APMIB_SHARED = 1, users/boa/Makefile),
   you have to do the step. Otherwise, skip this step.
   In kernel config, enter "General setup" menu to enable "System V IPC" option.

 	[*] Prompt for development and/or incomplete code/drivers
	()  Local version - append to kernel release             
	[ ] Automatically append version information to the version string
	[ ] Support for paging of anonymous memory (swap)        
	[*] System V IPC                                         
	[ ] POSIX Message Queues                                 
	[ ] BSD Process Accounting                  

4) In users config, enable "boa" application.

		 --- Applications                  
                   [*] auth                          
                   [*] brctl                         
                   [*] busybox                       
                   [ ] login on console              
                   [ ] Enhanced Ctorrent             
                   [ ] dcts                          
                   [ ] dhcpv6                        
                   [ ] discover                      
                   [*] dnrd                          
                   [ ] dnsmasq                       
                   [ ] dosfsck                       
                   [ ] gdbserver                     
                   [*] boa                           
                   [ ] gproxy


2. Directory structure
boa
|-- apmib            <----- apmib library 
|-- defconfig        <----- default apmib configuration 
|-- html             <----- HTML pages and graphics files 
|-- src              <----- source code of Boa web server 
|-- system           <----- source code for system and network init
|-- tools            <----- tools on x86
`-- utils            <----- flash utility

