

unsigned int Send_ISM_Cmd(unsigned int mode, unsigned int argv1, unsigned int argv2, unsigned int argv3, unsigned int *retval);
#define BUF_SIZE (tp->rx_buf_sz)




#define MAX_ARGV 80
static char*		ArgvArray[MAX_ARGV];
int GetArgc( const char* string )
{
	int			argc;
	char*		p ;

	argc = 0 ;
	p = (char* )string ;
	while( *p )
	{
		if( *p != ' '  &&  *p )
		{
			argc++ ;
			while( *p != ' '  &&  *p ) p++ ;
			continue ;
		}
		p++ ;
	}
	if (argc >= MAX_ARGV) argc = MAX_ARGV - 1;
	return argc ;
}

//---------------------------------------------------------------------------
char** GetArgv(const char* string)
{
	char*			p ;
	int				n;
	
	n = 0 ;
	memset( ArgvArray, 0, MAX_ARGV*sizeof(char *) );
	p = (char* )string ;
	while( *p )
	{
		ArgvArray[n] = p ;
		while( *p != ' '  &&  *p ) p++ ;
		*p++ = '\0';
		while( *p == ' '  &&  *p ) p++ ;
		n++ ;
		if (n == MAX_ARGV) break;
	}
	return (char** )&ArgvArray ;
}
//---------------------------------------------------------------------------
char* StrUpr( char* string ){
	char*		p ;
	const int	det = 'a' - 'A';

	p = string ;
	while( *p ){
		if( *p >= 'a'  &&  *p <= 'z' ){
			*p -= det ;
		}
		p++ ;
	}
	return string ;
}


//----------------------------------------------------------------------------
static int _atoi(const char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	}
	else {
		while (*s != '\0') {
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				printk("error hex format!\n");
				return 0;
			}
			k = 16 * k + v;
			s++;
		}
	}
	return k;
}

#define strtoul(x,y,z) _atoi(x,z)
//----------------------------------------------------------------------------
inline volatile unsigned int SPE_SWAP32(unsigned int data)   //wei add, for  endian swap
{
	unsigned int cmd=data;
	unsigned char *p=&cmd;	
	return ( (p[3]<<0) |  (p[2]<<8) | (p[1]<<16)  | (p[0]<<24) );	
}

//======================================================================
void ddump(unsigned char * pData, int len)
{
	unsigned char *sbuf = pData;	
	int length=len;

	int i=0,j,offset;
	dprintf(" [Addr]   .0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .A .B .C .D .E .F\r\n" );

	while(i< length)
	{		
			
			dprintf("%08X: ", (int)(sbuf+i) );

			if(i+16 < length)
				offset=16;
			else			
				offset=length-i;
			

			for(j=0; j<offset; j++)
				dprintf("%02x ", sbuf[i+j]);	

			for(j=0;j<16-offset;j++)	//a last line
			dprintf("   ");


			dprintf("    ");		//between byte and char
			
			for(j=0;  j<offset; j++)
			{	
				if( ' ' <= sbuf[i+j]  && sbuf[i+j] <= '~')
					dprintf("%c", sbuf[i+j]);
				else
					dprintf(".");
			}
			dprintf("\n\r");
			i+=16;
	}

	//dprintf("\n\r");	
}
//----------------------------------------------------------------

void dwdump(u32 * pData, int count)
{
	u32 *sbuf = pData;	
	int length=count;  //is word unit

	//dprintf("Addr=%x, len=%d", sbuf, length);	
	dprintf(" [Addr]    .0.1.2.3    .4.5.6.7    .8.9.A.B    .C.D.E.F" );
	
	{
		int i;		
		for(i=0;i<length; i++)
		{
			if((i%4)==0)
			{	dprintf("\n\r");
				dprintf("%08X:  ", (int)(sbuf+i) );
			}
			
			dprintf("%08X    ", sbuf[i]);
			//sbuf[i];
			
		}
		dprintf("\n\r");
	}	
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
int CMD_db( int argc, char* argv[] )
{
	
	unsigned int src;
	unsigned int len,i;
/*
	if(argc<1)
	{	dprintf("Wrong argument number!\r\n");
		return;
	}
*/
	if(argv[0])
		src = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	
	if(!argv[1])
		len = 1;
	else
	len= strtoul((const char*)(argv[1]), (char **)NULL, 10);		
	
	ddump(src,len);

}
//--------------------------------------------------------------------------
int CMD_dw( int argc, char* argv[] )
{
	
	unsigned int src;
	unsigned int len,i;
/*
	if(argc<1)
	{	dprintf("Wrong argument number!\r\n");
		return;
	}
*/
	if(argv[0])
		src = strtoul((const char*)(argv[0]), (char **)NULL, 16);
#ifdef CONFIG_R8198EP_DEVICE	
	else
		src = 0x80000000;
	
	if(src<0x80000000)
		src+=0x80000000;
#endif
	
	if(!argv[1])
		len = 1;
	else
	len= strtoul((const char*)(argv[1]), (char **)NULL, 10);		
	
	while ( (src) & 0x03)
		src++;

	for(i=0; i< len ; i+=4,src+=16)
	{	
		printk("%X:	%X	%X	%X	%X\n",
		src, *(unsigned long *)(src), *(unsigned long *)(src+4), 
		*(unsigned long *)(src+8), *(unsigned long *)(src+12));
	}

}
//------------------------------------------------------------------

int CMD_ew( int argc, char* argv[] )
{
	
	unsigned long src;
	unsigned int value,i;
	
	src = strtoul((const char*)(argv[0]), (char **)NULL, 16);		
	while ( (src) & 0x03)
		src++;

	for(i=0;i<argc-1;i++,src+=4)
	{
		value= strtoul((const char*)(argv[i+1]), (char **)NULL, 16);	
		*(volatile unsigned int *)(src) = value;
	}
	
}

//------------------------------------------------------------------
void dump_config(struct pci_dev *pdev, unsigned int offset)
{
	u16 val;
	unsigned int i;

	for(i=0;i<256;i=i+2)
	{	pci_read_config_word(pdev, offset+i, &val);
		if(i%4==0)
			printk("\n[%04x]    ", i);
		printk("%04x ", val);
	}

	printk("\n");
	for(i=0;i<3;i++)
		printk(" %d: [%08x-%08x] flags=%x \n",i,(int)(pdev->resource[i].start), (int)(pdev->resource[i].end), (int)pdev->resource[i].flags); 



}
//------------------------------------------------------------------

void dump_EPreg()
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;

	if(1)
	{//dump all	
	       dprintf("\n"); 
		   
		dprintf("DMA_TXFDP=%08x\n", REG32_R(SPE_DMA_TXFDP));
	 	dprintf("DMA_TXCDO=%x\n", REG32_R(SPE_DMA_TXCDO));  		  
	 	dprintf("DMA_RXFDP=%08x\n", REG32_R(SPE_DMA_RXFDP));                  
	 	dprintf("DMA_RXCDO=%x\n", REG32_R(SPE_DMA_RXCDO));                  
 		dprintf("DMA_TXOKCNT=%08x\n", REG32_R(SPE_DMA_TXOKCNT));                  
 		dprintf("DMA_RXOKCNT=%08x\n", REG32_R(SPE_DMA_RXOKCNT));          
                                            
	 	dprintf("DMA_IOCMD=%x\n", REG32_R(SPE_DMA_IOCMD));                  
	 	dprintf("DMA_IM=%x\n", REG32_R(SPE_DMA_IM ));  	          
	 	dprintf("DMA_IMR=%x\n", REG32_R(SPE_DMA_IMR ));  	          
	 	dprintf("DMA_ISR=%x\n", REG32_R(SPE_DMA_ISR ));                    
#ifdef CONFIG_R8198EP_HOST	
	 	dprintf("DMA_SIZE=%x\n", REG32_R(SPE_DMA_SIZE ));   		
 #endif                                                      
	 	dprintf("ISM_LR=%x\n", REG32_R(SPE_ISM_LR));   	          
 #ifdef CONFIG_R8198EP_HOST	
	 	dprintf("ISM_OR=%x\n", REG32_R(SPE_ISM_OR));   
	 	dprintf("ISM_DR=%x\n", REG32_R(SPE_ISM_DR));   		
#else
	 	dprintf("ISM_BAR=%x\n", REG32_R(SPE_ISM_BAR));  
#endif
#ifdef CONFIG_R8198EP_HOST		                                                        
          	dprintf("NFBI_CMD=%x\n", REG32_R(SPE_NFBI_CMD)); 
	 	dprintf("NFBI_ADDR=%x\n", REG32_R(SPE_NFBI_ADDR)); 
	 	dprintf("NFBI_DR=%x\n", REG32_R(SPE_NFBI_DR)); 
#endif
	 	dprintf("NFBI_SYSSR=%x\n", REG32_R(SPE_NFBI_SYSSR));               
	 	dprintf("NFBI_SYSCR=%x\n", REG32_R(SPE_NFBI_SYSCR));      
#ifdef CONFIG_R8198EP_HOST			 
	 	dprintf("NFBI_DCR=%x\n", REG32_R(SPE_NFBI_DCR));               
	 	dprintf("NFBI_DTR=%x\n", REG32_R(SPE_NFBI_DTR));     
	 	dprintf("NFBI_DDCR=%x\n", REG32_R(SPE_NFBI_DDCR));               
	 	dprintf("NFBI_TRXDLY=%x\n", REG32_R(SPE_NFBI_TRXDLY));     	
#endif
		return;
	}

	
}


//--------------------------------------------------------------------------

static void dump_tx_desc(struct rtl8198_private *tp)
{
	printk("TX: \n");
	if(tp && tp->TxDescArray )	
	dwdump( (u32 *)tp->TxDescArray , NUM_TX_DESC* sizeof(struct TxDesc)/4 );	
}
//--------------------------------------------------------------------------
static void dump_rx_desc(struct rtl8198_private *tp)
{
	printk("RX: \n");	
	if(tp && tp->RxDescArray )
	dwdump( (u32 *)tp->RxDescArray , NUM_RX_DESC* sizeof(struct RxDesc)/4 );
}

//--------------------------------------------------------------------------

//======================================================================


int SlvPCIe_RegRead(int argc, char* argv[])
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;

	int offset;
	
	if(argc<1)
	{//dump all	
		dump_EPreg();
		return;
	}

	int addr,val;
	StrUpr( argv[0] );
	if( ! strcmp( argv[0], "TXFDP" ) )		addr=SPE_DMA_TXFDP;
	else if( ! strcmp( argv[0], "TXCDO" ) )		addr=SPE_DMA_TXCDO;	
	else if( ! strcmp( argv[0], "RXFDP" ) )		addr=SPE_DMA_RXFDP;
	else if( ! strcmp( argv[0], "RXCDO" ) )		addr=SPE_DMA_RXCDO;

	else if( ! strcmp( argv[0], "TXOKCNT" ) )		addr=SPE_DMA_TXOKCNT;
	else if( ! strcmp( argv[0], "RXOKCNT" ) )		addr=SPE_DMA_RXOKCNT;
	
	else if( ! strcmp( argv[0], "IOCMD" ) )		addr=SPE_DMA_IOCMD;
	else if( ! strcmp( argv[0], "IM" ) )		addr=SPE_DMA_IM;
	else if( ! strcmp( argv[0], "IMR" ) )		addr=SPE_DMA_IMR;
	else if( ! strcmp( argv[0], "ISR" ) )		addr=SPE_DMA_ISR;	
#ifdef CONFIG_R8198EP_HOST		
	else if( ! strcmp( argv[0], "SIZE" ) )		addr=SPE_DMA_SIZE;	
#endif	
	else if( ! strcmp( argv[0], "ILR" ) )		addr=SPE_ISM_LR;
#ifdef CONFIG_R8198EP_HOST		
	else if( ! strcmp( argv[0], "IOR" ) )		addr=SPE_ISM_OR;	
	else if( ! strcmp( argv[0], "IDR" ) )		addr=SPE_ISM_DR;		
#else
	else if( ! strcmp( argv[0], "IBAR" ) )		addr=SPE_ISM_BAR;	
#endif
#ifdef CONFIG_R8198EP_HOST	
	else if( ! strcmp( argv[0], "CMD" ) )		addr=SPE_NFBI_CMD;		
	else if( ! strcmp( argv[0], "ADDR" ) )		addr=SPE_NFBI_ADDR;	
	else if( ! strcmp( argv[0], "DR" ) )		addr=SPE_NFBI_DR;		
#endif	
	else if( ! strcmp( argv[0], "SYSCR" ) )		addr=SPE_NFBI_SYSCR;	
	else if( ! strcmp( argv[0], "SYSSR" ) )		addr=SPE_NFBI_SYSSR;	
#ifdef CONFIG_R8198EP_HOST		
	else if( ! strcmp( argv[0], "DCR" ) )		addr=SPE_NFBI_DCR;	
	else if( ! strcmp( argv[0], "DTR" ) )		addr=SPE_NFBI_DTR;		
	else if( ! strcmp( argv[0], "DDCR" ) )		addr=SPE_NFBI_DDCR;	
	else if( ! strcmp( argv[0], "TRXDLY" ) )		addr=SPE_NFBI_TRXDLY;	
#endif	
	//else {	dprintf("Wrong Reg Name \n");	return; }
	else
	{
		//offset= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		//addr=PCIE0_RC_CFG_BASE+offset;
	}

	val=REG32_R(addr);

	//regr iocmd
	if(argc==1)
	{	dprintf("Addr %08x, %s=%08x \n", addr, argv[0],val );	
	}

	//regr iocmd 0001
	else if(argc==2)
	{
		unsigned int mask = _atoi((const char*)(argv[1]), 16);
		dprintf("Addr %s=%x \n", argv[0], val&mask );	
	}
	//regr iocmd : 0x0001
	else if(   argc>=3  &&  *(argv[1])==':' )
	{	
		unsigned int expval = _atoi((const char*)(argv[2]),  16);			
		if(val!=expval)
		{	dprintf("Fail, addr=%08x val=%x, expval=%x \n", addr, val, expval);
			//errcnt++;
		}
		else
			dprintf("Pass \n");

	}
	//regr iocmd 0x0001 : 0x0001
	else if(argc>=3  && *(argv[1]) != '\0')
	{	
		unsigned int mask = _atoi((const char*)(argv[1]) , 16);
		
		if(argc>=3 && *(argv[2]) == ':')
		{
			unsigned int expval = _atoi((const char*)(argv[3]), 16);	
			if( (val&mask) !=expval)
			{	dprintf("Fail, addr=%08x val=%x, expval=%x \n", addr, val, expval);
				//errcnt++;
			}
			else
				dprintf("Pass \n");
		}

	}

}
//----------------------------------------------------------------------------

int SlvPCIe_RegWrite(int argc, char* argv[])
{

	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	int addr,mask,val;

	
	if(argc<2)
	{	 
		dprintf("regw <reg_name> <val> \n");		
		dprintf("regw <reg_name> <mask> <value>\n");			
		dprintf("ex: regw IMR  ffffffff \n");			
		return 0;	
	}

//	int off = strtoul((const char*)(argv[0]), (char **)NULL, 16);


	StrUpr( argv[0] );
	if( ! strcmp( argv[0], "TXFDP" ) )		addr=SPE_DMA_TXFDP;
	else if( ! strcmp( argv[0], "TXCDO" ) )		addr=SPE_DMA_TXCDO;	
	else if( ! strcmp( argv[0], "RXFDP" ) )		addr=SPE_DMA_RXFDP;
	else if( ! strcmp( argv[0], "RXCDO" ) )		addr=SPE_DMA_RXCDO;
	
	else if( ! strcmp( argv[0], "TXOKCNT" ) )	addr=SPE_DMA_TXOKCNT;
	else if( ! strcmp( argv[0], "RXOKCNT" ) )	addr=SPE_DMA_RXOKCNT;
	
	else if( ! strcmp( argv[0], "IOCMD" ) )		addr=SPE_DMA_IOCMD;
	else if( ! strcmp( argv[0], "IM" ) )		addr=SPE_DMA_IM;
	else if( ! strcmp( argv[0], "IMR" ) )		addr=SPE_DMA_IMR;
	else if( ! strcmp( argv[0], "ISR" ) )		addr=SPE_DMA_ISR;	
	
#ifdef CONFIG_R8198EP_HOST	
	else if( ! strcmp( argv[0], "SIZE" ) )		addr=SPE_DMA_SIZE;	
#endif	
	else if( ! strcmp( argv[0], "ILR" ) )		addr=SPE_ISM_LR;	
#ifdef CONFIG_R8198EP_HOST
	else if( ! strcmp( argv[0], "IOR" ) )		addr=SPE_ISM_OR;
	else if( ! strcmp( argv[0], "IDR" ) )		addr=SPE_ISM_DR;	
#else
	else if( ! strcmp( argv[0], "IBAR" ) )		addr=SPE_ISM_BAR;		
#endif
		
#ifdef CONFIG_R8198EP_HOST	
	else if( ! strcmp( argv[0], "CMD" ) )		addr=SPE_NFBI_CMD;		
	else if( ! strcmp( argv[0], "ADDR" ) )		addr=SPE_NFBI_ADDR;	
	else if( ! strcmp( argv[0], "DR" ) )		addr=SPE_NFBI_DR;		
#endif	

	else if( ! strcmp( argv[0], "SYSCR" ) )		addr=SPE_NFBI_SYSCR;	
	else if( ! strcmp( argv[0], "SYSSR" ) )		addr=SPE_NFBI_SYSSR;	
	
#ifdef CONFIG_R8198EP_HOST		
	else if( ! strcmp( argv[0], "DCR" ) )		addr=SPE_NFBI_DCR;	
	else if( ! strcmp( argv[0], "DTR" ) )		addr=SPE_NFBI_DTR;		
	else if( ! strcmp( argv[0], "DDCR" ) )		addr=SPE_NFBI_DDCR;	
	else if( ! strcmp( argv[0], "TRXDLY" ) )		addr=SPE_NFBI_TRXDLY;	
#endif	
	//else dprintf("Wrong Reg Name \n");	
	else
	{
		//int offset= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		//addr=PCIE0_EP_CFG_BASE+offset;		
	}	

	//regw iocmd 0x0001
	if(argc==2)
	{
		val = _atoi((const char*)(argv[1]),  16);
		REG32_W(addr,val);	
	}

	//regw iocmd 0x0001 0x0001
	else if(argc>=3)
	{
		 mask = _atoi((const char*)(argv[1]),  16);
		 val = _atoi((const char*)(argv[2]),  16);
		REG32_W(addr, (REG32_R(addr) & mask) | val );	

	}
		

	
}

//---------------------------------------------------------------------------------


int CMD_Dump(int argc, char* argv[])
{
	int i,len;
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned int offset=0;

	
	if(argc<1)
	{	dprintf("dump <ep> <offset>  \n");	
		dprintf("dump <txd/txb/rxd/rxb> <number>  \n");
		dprintf("dump <nfbi> <addr>  \n");
		return;
	}
	StrUpr( argv[0] );

	if(argc>1)
	offset = _atoi((const char*)(argv[1]),  16);	
	

	if( ! strcmp( argv[0], "EP" ) )	
	{
		dprintf("EP Cfg \n");
	}
	//----------------------------------------
	else if( ! strcmp( argv[0], "TXD" ) )	
	{
		dump_tx_desc(tp);
	}
	else if( ! strcmp( argv[0], "RXD" ) )	
	{
		dump_rx_desc(tp);	

	}
	//-------------------------------------------
	else if( ! strcmp( argv[0], "CNT" ) )	
	{
		dprintf("dirty_tx=%d [0x%x] \n", tp->dirty_tx, tp->dirty_tx);		
		dprintf("  cur_tx=%d [0x%x] \n", tp->cur_tx,   tp->cur_tx);	
		dprintf("dirty_rx=%d [0x%x] \n", tp->dirty_rx, tp->dirty_rx);			
		dprintf("  cur_rx=%d [0x%x] \n", tp->cur_rx,   tp->cur_rx);	
	

	}
	//-------------------------------------------
	else if( ! strcmp( argv[0], "TXB" ) )	
	{
		unsigned int num=0;	
		if(argc>= 2)
		 num = _atoi((const char*)(argv[1]),  16);	
#ifdef CONFIG_R8198EP_HOST			
		if(tp->TxDescArray[num].addr)		
		//	ddump(tp->TxDescArray[num].addr, RX_BUF_SIZE/4);	
			ddump((unsigned char *)&tp->tx_skb[num], (int)RX_BUF_SIZE/4);	
#else  //CONFIG_R8198EP_DEVICE
#ifdef DEVICE_USING_FIXBUF
			ddump((unsigned char *)(tp->pTxBuffPtr[num]), (int)RX_BUF_SIZE/4);	
#endif
#endif


		
	}
	else if( ! strcmp( argv[0], "RXB" ) )	
	{
		unsigned int num=0;	
		if(argc>= 2)
		 num = _atoi((const char*)(argv[1]),  16);		
	/*	
		if(tp->RxDescArray[num].addr)		
			ddump(tp->RxDescArray[num].addr, tp->rx_buf_sz/4);	
	*/
#ifdef CONFIG_R8198EP_HOST		
		if(tp->Rx_skbuff[num]->data)			
			ddump( (unsigned char *)(tp->Rx_skbuff[num]->data), (int)(tp->rx_buf_sz/4));	
#else //CONFIG_R8198EP_DEVICE
#ifdef DEVICE_USING_FIXBUF
			ddump( (unsigned char *)(tp->pRxBuffPtr[num]), (int)(tp->rx_buf_sz/4));	
#endif			

#endif
	}	
	//---------------------------------------------
	
	else if( ! strcmp( argv[0], "ISM" ) )	
	{
#ifdef CONFIG_R8198EP_HOST	
		len=REG32_R(SPE_ISM_LR);
		REG32_W(SPE_ISM_OR,offset);

		
		for(i=0; i<len; i=i+4)
		{
			if(i%16==0)
			{
				dprintf("\nOff:[%08x]    ", i);
			}			
			dprintf("%08x    ",REG32_R(SPE_ISM_DR));			
		}		
		dprintf("\n");
#else
		printk("ISM BAR=%x, Len=%x \n",tp->ISM_buff, tp->ISM_len);
		ddump(tp->ISM_buff, tp->ISM_len);
#endif
	}	
	//---------------------------------------------
	else if( ! strcmp( argv[0], "NFBI" ) )	
	{
		REG32_W(SPE_NFBI_ADDR,offset);
		len=0x80;

		
		for(i=0; i<len; i=i+4)
		{
			if(i%16==0)
			{
				dprintf("\nAddr:[%08x]    ", offset+i);
			}			
			dprintf("%08x    ",REG32_R(SPE_NFBI_DR));			
		}		
		dprintf("\n");

	}

}

//----------------------------------------------------------------

//---------------------------------------------------------------------------
int CMD_VarRead(int argc, char* argv[])
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	unsigned int val;
	unsigned int check=0,mask=0xffffffff,expval=0;

	if(argc<1)
	{//dump all	
	       	   
		dprintf("Varr info1, dmsg, ");
		dprintf("\n"); 	
		return;
	}


	StrUpr( argv[0] );
	if( ! strcmp( argv[0], "INFO1" ) )		val=tp->info1;	
	else if( ! strcmp( argv[0], "DMSG" ) )		val=tp->dmsg;		
	else {	dprintf("Wrong Reg Name \n");	return; }
/*	
	else
	{
		return;
	}
*/
	//regr iocmd
	if(argc==1)
	{	
	}
	//regr iocmd 0001
	else if(argc==2)
	{
		mask = _atoi((const char*)(argv[1]),16);	
	}
	//regr iocmd : 0x0001
	else if(   argc>=3  &&  *(argv[1])==':' )
	{	check=1;
		expval = _atoi((const char*)(argv[2]), 16);	
	}
	//regr iocmd 0x0001 : 0x0001
	else if(argc>=3  && *(argv[1]) != '\0')
	{	
		 mask = _atoi((const char*)(argv[1]), 16);
		
		if(argc>=3 && *(argv[2]) == ':')
		{	check=1;
			expval = _atoi((const char*)(argv[3]), 16);	
		}
	}


	//verify
	if(!check)
	{
		dprintf(" %s=%08x \n",  argv[0],val&mask );			
	}
	else
	{
		if( (val&mask) !=expval)
		{	dprintf("Fail, %s val=%x, expval=%x \n", argv[0], val, expval);
			//at_errcnt++;
		}
		else
			dprintf("Pass \n");
	}	


}
//----------------------------------------------------------------------------
int CMD_VarWrite( int argc, char* argv[] )
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	if(argc<2)
	{	 
		dprintf("varw <name> <val> \n");		
		dprintf("varw <name> <mask> <value>\n");			
		dprintf("ex: varw IMR  ffffffff \n");			
		return;	
	}

//	int off = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	unsigned int mask=0,val;
	unsigned int *addr;

	StrUpr( argv[0] );
	if( ! strcmp( argv[0], "INFO1" ) )		addr=&(tp->info1);
	else if( ! strcmp( argv[0], "DMSG" ) )		addr=&(tp->dmsg);
	//else if( ! strcmp( argv[0], "RXFDP" ) )		addr=SPE_DMA_RXFDP;
	
	//else dprintf("Wrong Reg Name \n");	
	else
	{
	
	}	

	//regw iocmd 0x0001
	if(argc==2)
	{	mask=0;
		val = _atoi((const char*)(argv[1]),  16);	
	}

	//regw iocmd 0x0001 0x0001
	else if(argc>=3)
	{	mask = _atoi((const char*)(argv[1]), 16);
		val = _atoi((const char*)(argv[2]), 16);
	}
		
	*(addr)=(*(addr) & mask) | val;
}


//----------------------------------------------------------------

int CMD_DMATx(int argc, char* argv[])
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	int Len=tp->rx_buf_sz;
	
	unsigned int i;	
	unsigned int len=8; 
	unsigned int txinfo1=0, txinfo2=0, txinfo3=0;	
	int offset=0;
	int fs,ls;

	if( argc < 1 ) 
	{
		dprintf("tx <mode>  <len> <info1>	<info2>	<info3>	<offset> \n");
		dprintf("tx 1t 128\n");			
		return 0;
	}


	StrUpr( argv[0] );	
	 if( ! strcmp( argv[0], "1T" ) )
	 {
		fs=1; ls=1;
	 }
	 else  if( ! strcmp( argv[0], "MTF" ) )
	 {
		fs=1; ls=0;
	 }
	 else  if( ! strcmp( argv[0], "MTM" ) )
	 {
		fs=0; ls=0;
	 }
	 else  if( ! strcmp( argv[0], "MTL" ) )
	 {
		fs=0; ls=1;
	 }
 	else
 	{
 		fs=1; ls=1;	
 	}

	if(argc>=2)
	len= strtoul((const char*)(argv[1]), (char **)NULL, 16);

	if(argc>=3)
	txinfo1= strtoul((const char*)(argv[2]), (char **)NULL, 16);

	if(argc>=4)
	txinfo2= strtoul((const char*)(argv[3]), (char **)NULL, 16);

	if(argc>=5)
	txinfo3= strtoul((const char*)(argv[4]), (char **)NULL, 16);	

	if(argc>=6)
	offset= strtoul((const char*)(argv[5]), (char **)NULL, 16);	


	//action:
	//DMA_Transmit(DMA_buff, len,fs,ls, txinfo1, txinfo2, txinfo3, offset);
	rtl8198_hw_tx(tp->DMA_buff, len,fs,ls, txinfo1, txinfo2, txinfo3, offset);
#if 0	
	struct sk_buff *skb;
	skb= dev_alloc_skb(len);
	if (!skb) 
	{
		printk(" tx: fail, no mem \n");		
		return NULL;
	}
	skb_put(skb, tp->DMA_len);
	memcpy(skb->data, tp->DMA_buff, tp->DMA_len);
	skb->dev = tp->dev;	
	skb->len=len; //tp->DMA_len;   //using key in len
	
	tp->txoffset=offset;
	tp->info1=txinfo1;
	tp->info2=txinfo2;
	tp->info3=txinfo3;
	
	if(rtl8198_start_xmit(skb, reNet)!=NETDEV_TX_OK)
	{
		printk("start xmit faile \n");
	};
	tp->txoffset=0;	
	tp->info1=0;
	tp->info2=0;
	tp->info3=0;	
	//dev_kfree_skb(skb);  //release in tx_interrupt
#endif
	
}; 

//----------------------------------------------------------------

int CMD_DMARx(int argc, char* argv[])
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	unsigned char *pbuff;
	int rxlen;
	int rxoffset=0;
	unsigned int rxinfo1, rxinfo2,rxinfo3,drop,offset;



	if(argc<=0)
	{
		//if(DMA_Receive(&pbuff, &rxlen,&rxinfo1, &rxinfo2, &rxinfo3, &drop)==-1)
		//if(rtl8198_rx_interrupt(tp->dev, tp, tp->mmio_addr, ~(u32)0)==0)		
		if(rtl8198_hw_rx(&pbuff, &rxlen,&rxinfo1, &rxinfo2, &rxinfo3, &offset, &drop)==-1)
		{
			dprintf("no rx pkt, at index=%x\n", tp->cur_rx);
			return -1;
		};	
	
		return 0;
	}


	//if(DMA_Receive(&pbuff, &rxlen,&rxinfo1, &rxinfo2, &rxinfo3, &drop)==-1)
	//if(rtl8198_rx_interrupt(tp->dev, tp, tp->mmio_addr, ~(u32)0)==0)
	if(rtl8198_hw_rx(&pbuff, &rxlen,&rxinfo1, &rxinfo2, &rxinfo3, &offset, &drop)==-1)	
	{
		dprintf("no rx pkt, at index=%x\n", tp->cur_rx);
		return -1;
	};	

	unsigned int txlen=0; 
	unsigned int txinfo1=0, txinfo2=0, txinfo3=0;	
	int txoffset=0;

	rxoffset=(int)pbuff&0x03;

	if(argc>=1)
		StrUpr( argv[0] );	


	if(argc>=2)
	txlen= strtoul((const char*)(argv[1]), (char **)NULL, 16);

	if(argc>=3)
	txinfo1= strtoul((const char*)(argv[2]), (char **)NULL, 16);

	if(argc>=4)
	txinfo2= strtoul((const char*)(argv[3]), (char **)NULL, 16);

	if(argc>=5)
	txinfo3= strtoul((const char*)(argv[4]), (char **)NULL, 16);	

	if(argc>=6)
	txoffset= strtoul((const char*)(argv[5]), (char **)NULL, 16);	
	
	 if( ! strcmp( argv[0], "CHK" ) )
	 {

	 	if(txlen!=rxlen)		{	dprintf("fail, txlen=%x rxlen=%x \n", txlen, rxlen);at_errcnt++; return -1; }
		if(txoffset!=rxoffset)	{	dprintf("fail, txoffset=%x rxoffset=%x \n", txoffset, rxoffset);at_errcnt++; return -1; }

		if(txinfo1!=rxinfo1)	{	dprintf("fail, txinfo1=%x rxinfo1=%x \n", txinfo1, rxinfo1); at_errcnt++;return -1; }
		if(txinfo2!=rxinfo2)	{	dprintf("fail, txinfo2=%x rxinfo2=%x \n", txinfo1, rxinfo1); at_errcnt++;return -1; }
		if(txinfo3!=rxinfo3)	{	dprintf("fail, txinfo3=%x rxinfo3=%x \n", txinfo1, rxinfo1); at_errcnt++;return -1; }

		int i;
		for(i=0; i<txlen; i++)
		{
			if(tp->DMA_buff[i]!=pbuff[i]) 		
			{	dprintf("fail,  txdata[%x]=%x rxdata[%x]=%x \n", i, tp->DMA_buff[i],  i,pbuff[i]); at_errcnt++;return -1; 
			}
		
		}

		dprintf("Pass\n");
		
	 }
	
	
}; 

//-----------------------------------------------------------------
u8 random(u8 x )
{
	srandom32(x);
	return random32();
}

unsigned int pktgen(unsigned int mode, unsigned char *pbuff, unsigned int pklen ,unsigned char initval)
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	unsigned int len;
	unsigned int val;
		int i;	
		

	
	//cal len
	if(mode&PKTGEN_LenFix)
	{	
		len=pklen;
		if(pklen==0)			
			len=(int)random(BUF_SIZE+1);
		
	}
	else if(mode&PKTGEN_LenRand)
	{	len=(int)random(BUF_SIZE+1);
	}
	else if(mode&PKTGEN_LenRand4B)
	{	
		len=(int)random(pklen+1)&~(3);
	}


	//protect
	if(len==0) 
	{	len++;
		if(mode&PKTGEN_LenRand4B)
			len=4;
	}
	if(len>BUF_SIZE) len=BUF_SIZE;

	//fill content
	if(mode&PKTGEN_ValInc)
	{	for(i=0;i<len;i++)
		{	pbuff[i]=(unsigned char)(initval+i)&0xff;
		}
	}
	else if(mode&PKTGEN_ValRand)
	{	for(i=0;i<len;i++)
		{	pbuff[i]=(unsigned char)random(256)&0xff;
		}
	}
	else if(mode&PKTGEN_ValFix)
	{	for(i=0;i<len;i++)
		{	pbuff[i]=(unsigned char)(initval)&0xff;
		}
	}	
	return len;
}

//----------------------------------------------------------------
int CMD_PktGen(int argc, char* argv[])
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	int i;
	if( argc < 4 ) 
	{
		dprintf("pktgen <mode> <len> <mode> <initval>\n");
		dprintf("pktgen fix 128 fix 55\n");	
		dprintf("pktgen fix 128 inc 55\n");	
		dprintf("pktgen rand[4B] 0 rand 0 \n");			
		return 0;
	}

	unsigned int len=4,val=0;	
	unsigned int mode=0;

	
	StrUpr( argv[0] );		
	 if( ! strcmp( argv[0], "FIX" ) )
	 {	mode=PKTGEN_LenFix;
	 }	 
	 else if( ! strcmp( argv[0], "RAND" ) )
	 {	mode=PKTGEN_LenRand;	
	 }	 
	 else if( ! strcmp( argv[0], "RAND4B" ) )
	 {	mode=PKTGEN_LenRand4B;	
	 }	
	 len = strtoul((const char*)(argv[1]), (char **)NULL, 16);			 
	//
	StrUpr( argv[2] );		
	 if( ! strcmp( argv[2], "FIX" ) )
	 {	mode|=PKTGEN_ValFix;
	 }	 
	 else if( ! strcmp( argv[2], "INC" ) )
	 {	mode|=PKTGEN_ValInc;	
	 }	 
	 else if( ! strcmp( argv[2], "RAND" ) )
	 {	mode|=PKTGEN_ValRand;	
	 }		 
	val = strtoul((const char*)(argv[3]), (char **)NULL, 16);	
	
	tp->DMA_len=pktgen(mode, tp->DMA_buff,len, val);
	dprintf("Gen addr=%x, len=0x%x bytes \n", tp->DMA_buff, tp->DMA_len);		


/*
	for(i=0;i<argc-1;i++,src+=4)
	{
		value= strtoul((const char*)(argv[i+1]), (char **)NULL, 16);	
		*(volatile unsigned int *)(src) = value;
	}
*/		
}; 


//----------------------------------------------------------------
void DMA_Restart(struct net_device *dev)		//wei add
{
			//rtl8198_close(dev);
			//rtl8198_open(dev);			
			rtl8198_init_ring(dev);
			rtl8198_hw_start(dev);

}




//===================================================
#ifdef CONFIG_R8198EP_HOST


inline void NFBI_WRITE_MEM32(unsigned int addr,unsigned int data)
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	REG32_W(SPE_NFBI_ADDR,addr);
	REG32_W(SPE_NFBI_DR,data);
}


inline unsigned int NFBI_READ_MEM32(unsigned int addr)
{	
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	REG32_W(SPE_NFBI_ADDR,addr);
	unsigned int val=	REG32_R(SPE_NFBI_DR);
	return val;
}

//--------------------------------------------------------------
void DDR_calibration()
{ 
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;

	
	#define DDR_DBG 0
	#define IMEM_DDR_CALI_LIMITS 1


	int i,j,k;
   
        int L0 = 0, R0 = 33, L1 = 0, R1 = 33;
 
	// int  DRAM_ADR = 0xa0400000;//0xa0400000 is virtual
        int  DRAM_ADR = 0x400000;//Physical address 
	int  DRAM_VAL = 0x5A5AA5A5;
 
       // int  DDCR_ADR = 0xB8001050;//Virtual
        int  DDCR_ADR = SPE_NFBI_DDCR;//Physical    
        int  DDCR_VAL = 0x80000000; //Digital
        //int  DDCR_VAL = 0x0; //Analog (JSW: N/A for 8198 FPGA)   
  
 
        NFBI_WRITE_MEM32(DRAM_ADR, DRAM_VAL);   
        //while( (NFBI_READ_MEM32(DDCR_ADR)& 0x40000000) != 0x40000000);
       
 
  for(k=1;k<=IMEM_DDR_CALI_LIMITS;k++)  //Calibration times
    //while(1)
    {
  
         // Calibrate for DQS0
         for (i = 1; i <= 31; i++)
         {           
       	REG32_W(DDCR_ADR,  (DDCR_VAL & 0x80000000) | ((i-1) << 25));
		
 		#if DDR_DBG
              __delay(10000);
             //dprintf("DQS0(i=%d),(DDCR=0x%x) \n", i,EP_REG32_READ(DDCR_ADR));
	 	#endif
		
            if (L0 == 0)
            {              
          		if ((NFBI_READ_MEM32(DRAM_ADR) & 0x00FF00FF) == 0x005A00A5)
			{
				L0 = i;
			}
            }
            else
            {       
		#if DDR_DBG
         	dprintf("  DRAM(0x%x)=%x\n", DRAM_ADR,NFBI_READ_MEM32(DRAM_ADR));
      		#endif
			
               if ((NFBI_READ_MEM32(DRAM_ADR) & 0x00FF00FF) != 0x005A00A5)
               {
                  //dprintf("\n\n\nError!DQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_ADR));
			#if DDR_DBG
			dprintf("  DRAM(0x%x)=%x\n\n\n", DRAM_ADR,NFBI_READ_MEM32(DRAM_ADR));
			#endif
                  R0 = i - 1;
                  //R0 = i - 3;  //JSW
                  break;
               }
            } //end else

		//dprintf("i=%d L0=%d R0=%d \n",i,L0,R0);
         }
        
	DDCR_VAL = (DDCR_VAL & 0xC0000000) | (((L0 + R0) >> 1) << 25); // ASIC
	dprintf("DDCR_VAL =%x \n", DDCR_VAL );
	REG32_W(DDCR_ADR,  DDCR_VAL);
 
 	//----------------------------------------------------------------
         // Calibrate for DQS1
         for (i = 1; i <= 31; i++)
         {
        #if DDR_DBG
               __delay(10000);
              dprintf("DQS1(i=%d),(DDCR=0x%x) \n", i,EP_REG32_READ(DDCR_ADR));
 #endif
        
             
   REG32_W(DDCR_ADR,  (DDCR_VAL & 0xFE000000) | ((i-1) << 20));
 
            if (L1 == 0)
            {
               if ((NFBI_READ_MEM32(DRAM_ADR) & 0xFF00FF00) == 0x5A00A500)
               {
                  L1 = i;
               }
            }
            else
            {
			#if DDR_DBG
			dprintf("  DRAM(0x%x)=%x \n", DRAM_ADR,NFBI_READ_MEM32(DRAM_ADR));
			#endif
               if ((NFBI_READ_MEM32(DRAM_ADR) & 0xFF00FF00) != 0x5A00A500)
               {
                 //dprintf("\n\n\nError!DQS1(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
           // dprintf("DRAM(0x%x)=%x\n\n\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
                  R1 = i - 1;
      //R1 = i - 3;
                  break;
               }
            }
         }
 
         DDCR_VAL = (DDCR_VAL & 0xFE000000) | (((L1 + R1) >> 1) << 20); // ASIC
         dprintf("\nDDCR_VAL =%x\n", DDCR_VAL );
          REG32_W(DDCR_ADR,  DDCR_VAL);
 
        /* wait a little bit time, necessary */       
       __delay(100);
  #if 1  
		dprintf("R0:%d L0:%d C0:%d\n", R0, L0, (L0 + R0) >> 1);
		dprintf("R1:%d L1:%d C1:%d\n", R1, L1, (L1 + R1) >> 1);
                 
              dprintf("=>After IMEM Cali,DDCR(%d)=0x%x \n",k ,REG32_R(DDCR_ADR));
		dprintf("=================================\n");
 #endif
  
 
     }//end of while(1) 
 
 
 
}

//--------------------------------------------------------------
void SettingDRAMConfig()
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	int cmd=REG32_R(SPE_NFBI_CMD);
	REG32_W(SPE_NFBI_CMD, cmd | NFBI_CMD_SWAP );	

	int dram_type=0; //0: SDR, 1:DDR, 
	int dram_trxdly;
	
	if(cmd& (1<<3))		dram_type=1;
	else dram_type=0;	
	
	if(dram_type==0)		dram_trxdly=DRAM_TRXDLY_VAL_SDR;
	else if(dram_type==1)		dram_trxdly=DRAM_TRXDLY_VAL_DDR1;
	else if(dram_type==2)		dram_trxdly=DRAM_TRXDLY_VAL_DDR2;				

	dprintf("DRAM type=%x, Set T/RX dly=%x \n",dram_type, dram_trxdly);
	REG32_W(SPE_NFBI_TRXDLY, dram_trxdly);
	if( REG32_R(SPE_NFBI_TRXDLY) != dram_trxdly)
	{
		//at_errcnt++;
		dprintf("write fail! NFBI_TRXDLY=%x \n", REG32_R(SPE_NFBI_TRXDLY));
	}


	dprintf("Set DRAM cfg\r\n");
	REG32_W(SPE_NFBI_DTR, DRAM_TIMING_VAL);
	if(REG32_R(SPE_NFBI_DTR)!= DRAM_TIMING_VAL) 
	{	//at_errcnt++;
		dprintf("write fail! NFBI_DTR=%x \n", REG32_R(SPE_NFBI_DTR));	
	}
	
	REG32_W(SPE_NFBI_DCR, DRAM_CONFIG_VAL);
	if(REG32_R(SPE_NFBI_DCR)!= DRAM_CONFIG_VAL) 
	{	//at_errcnt++;
		dprintf("writing fail! NFBI_DCR=%x  \n", REG32_R(SPE_NFBI_DCR));
	}


	if( dram_type == 1)
	{
		dprintf("Do DDR calibration \n");
		DDR_calibration();  //just for DDR
	}

}

//----------------------------------------------------------------------------------------
void SettingJumpCode(unsigned int saddr, unsigned int daddr)
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;



	int i,off=0;
	unsigned int dramcode[]={
							//DTR
							0x3c08ffff, 	// lui	t0,0xffff		//0
							0x350805c0, 	// ori	t0,t0,0x5c0		//1
							0x3c09b800, 	// lui	t1,0xb800
							0x35291008, 	// ori	t1,t1,0x1008
							0xad280000, 	// sw	t0,0(t1)
							//DCR
							0x3c085448, 	// lui	t0,0x5448		//5
							0x35080000, 	// ori	t0,t0,0x0000		//6				
							0x3c09b800, 	// lui	t1,0xb800
							0x35291004, 	// ori	t1,t1,0x1004
							0xad280000, 	// sw	t0,0(t1)

							//TRX	
							0x3c0800ff, 	// lui	t0,0x00ff			//10
							0x3508ffd6, 	// ori	t0,t0,0xffd6		//11
							0x3c09b800, 	// lui	t1,0xb800
							0x35290010, 	// ori	t1,t1,0x0010
							0xad280000, 	// sw	t0,0(t1)
							};

	
	unsigned int jmpcode[]={
							0x3c1aa070, 	//		lui	k0,0x8123 
							0x375a0000, 	// 		ori	k0,k0,0x4567  
							0x03400008, 	//		jr	k0 
							0x0	,		// 		nop
							0x5555aaaa,	// 		magic numbeer
							};

	unsigned int dbgcode[]={
							0x3c080000, 	// lui	t0,0x0000			//0
							0x35080001, 	// ori	t0,t0,0x0001		//1				
							0x3c09b8b4, 	// lui	t1,0xb8b4
							0x3529108C, 	// ori	t1,t1,0x108C           //b8b4108c=SYSSR
							0xad280000, 	// sw	t0,0(t1)
							0x0			// 		nop
							};	

		//setting remote DCR and DTR register
		dramcode[0]=(dramcode[0] &0xffff0000) | DRAM_TIMING_VALH;
		dramcode[1]=(dramcode[1] &0xffff0000) | DRAM_TIMING_VALL;
		dramcode[5]=(dramcode[5] &0xffff0000) | DRAM_CONFIG_VALH;
		dramcode[6]=(dramcode[6] &0xffff0000) | DRAM_CONFIG_VALL;		
		//dramcode[10]=(dramcode[10] &0xffff0000) | DRAM_TRXDLY_VALH;
		//dramcode[11]=(dramcode[11] &0xffff0000) | DRAM_TRXDLY_VALL;			

		jmpcode[0]=(jmpcode[0] &0xffff0000) | (( (daddr|0x80000000) & 0xffff0000)>>16);
		jmpcode[1]=(jmpcode[1] &0xffff0000) | ( (daddr|0x80000000) & 0x0000ffff);			



	//--------------------------------------------	

		dprintf("Set Reset Jump Code,  from %08x jump to %08x\n", saddr, daddr);
		saddr=saddr&0x0fffffff;

		for(i=0;i<6;i++)
		NFBI_WRITE_MEM32(saddr+(off++)*4, dbgcode[i] );			
		
	//	for(i=0;i<15;i++)
	//	NFBI_WRITE_MEM32(saddr+(off++)*4, dramcode[i] );			
		for(i=0;i<5;i++)
		NFBI_WRITE_MEM32(saddr+(off++)*4, jmpcode[i] );	
		
}

//-------------------------------------------------------------------------
int CMD_SetJumpCode(int argc, char* argv[])
{
	int saddr=0x80700000;
	int daddr=0x80700000;

	if(argc<1)
	{	dprintf("sendj 0  , set configuration\n");
		dprintf("Usage: sendj from_addr to_addr\n");	
		dprintf("Usage: sendj 00008000 80700000\n");				
		return;
	}
	if(argc ==1 )
	{
		SettingDRAMConfig();	
		return;

	}
	if(argc >=2 )
	{
		saddr= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		daddr= strtoul((const char*)(argv[1]), (char **)NULL, 16);
		SettingDRAMConfig();		
		SettingJumpCode(saddr, daddr);		
	}
	
}
//-----------------------------------------------------------------
int CMD_TESTRAM(int argc, char* argv[])
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	unsigned int daddr,dlen;
	unsigned int i,sidx;
	unsigned int t,v,exp;

	if(argc<1)
	{	
		dprintf("Usage: tram <remote_addr> <remote_len>, ps: using pkgen first. \n");			
		return;
	}


	if(argc >=2 )
	{
		daddr= strtoul((const char*)(argv[0]), (char **)NULL, 16);	
		dlen= strtoul((const char*)(argv[1]), (char **)NULL, 16);
	}

	SettingDRAMConfig();
	dprintf("\nWriting ram addr=0x%08x, len=0x%08x \n",daddr, dlen);

	REG32_W(SPE_NFBI_ADDR, daddr) ;
	sidx=0;
	for(i=0;i<dlen;i+=4)
	{	
		sidx=i%(tp->DMA_len);
		REG32_W(SPE_NFBI_DR, *(unsigned int *)(tp->DMA_buff+sidx));
		
		if( !(i&0xffff))	dprintf(".");		
	}

	//------------------------------read bootcode back
	if(1)
	{
		dprintf("\ncode Reading Back\n");
		REG32_W(SPE_NFBI_ADDR,daddr);
	
		for(i=0;i<dlen;i+=4)
		{	
			v=REG32_R(SPE_NFBI_DR);	
			sidx=i%tp->DMA_len;	
			t=*(unsigned int *)(tp->DMA_buff+sidx);
			if(v!=  t)
				dprintf("Error! at addr=%x value=%x Expect val=%x\n", daddr+i, v, t);		
			if( !(i&0xffff))	dprintf(".");	
		
		}	
	}	
	//--------------------------------------	
	return 1;

}
//-----------------------------------------------------------------------------------------


int NFBI_SendCode(unsigned int addr, unsigned char *buff, unsigned int len, unsigned int send,unsigned int verify )
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;

	unsigned int i;
	unsigned int t;
	unsigned int *p;

	int remain;
	unsigned int verify_errcnt=0;

	if(send)
	{
	dprintf("Writing local buff addr=%08x len=%x to remote addr=%08x ...\n",buff,len,addr);
	REG32_W(SPE_NFBI_ADDR,addr); 

	for(i=0;i<len;i+=4)
	{		
		//if( !(i&0xfff))	dprintf(".");	
		//REG32_W(SPE_NFBI_DR, *(unsigned int *)(buff+i)) ; 			//no swap
		REG32_W(SPE_NFBI_DR, SPE_SWAP32(*(unsigned int *)(buff+i)) ) ;    //x86 swap, because we use big-endia image
		
		//dprintf("Write data=%x \n", *(unsigned int *)(buff+i));
	}
	if( (len%4) )
	{
		remain=(len%4);
		dprintf("bootcode size is not 4 factor \n");
#if 0
		switch(remain)
		{
			case 1:	REG32(0x13,(bootstart[i] <<8)| 0 ); //data 
					REG32(0x14, 0 | 0 );
					break;
			case 2:	REG32(0x13,(bootstart[i] <<8)| bootstart[i+1] ); //data
					REG32(0x14, 0 | 0 );
					break;
				
			case 3:	REG32(0x13,(bootstart[i] <<8)| bootstart[i+1] ); //data
					REG32(0x14, bootstart[i+2]<<8 | 0 );
					break;
		}
#endif
	}
	}
	//------------------------------read bootcode back
	if(verify)
	{
		dprintf("code Reading Back...\r\n");
		REG32_W(SPE_NFBI_ADDR, addr); //address H
		
		for(i=0;i<len;i+=4)
		{	
			//t=REG32_R(SPE_NFBI_DR);
			t=SPE_SWAP32(REG32_R(SPE_NFBI_DR));  //x86 swap, becaus we use big-endia image

			p=(unsigned int *)(buff+i) ;
			if( t != *p) 		 
			{	dprintf("Error! at offset=%x value=%04x Expect val=%04x\r\n", i,t, *p );
				verify_errcnt++;				
			}

			if(verify_errcnt>32)
				break;
		}	
	}

	//if(verify_errcnt!=0)
	//	at_errcnt++;
	return 1;
}

//----------------------------------------------------------------------------
int ReadFileinKernel(char *filename, unsigned char *buf, int maxlen)
{

	struct file *filp;
	int r;
	mm_segment_t oldfs;
	oldfs=get_fs();
	set_fs(KERNEL_DS);

	filp=filp_open(filename,O_RDONLY,0);


	if(IS_ERR(filp))
        {        printk("File Open Error:%s\n",filename);        return 0;        }

	if(!filp->f_op)
        {       printk("File Operation Method Error!!\n");        return 0;       }



	//printk("File Offset Position:%xh\n",(unsigned)filp->f_pos);
	
	//r=filp->f_op->write(filp,buf,sizeof(buf),&filp->f_pos);
	//printk("Write :%xh bytes Offset=%xh\n",r,(unsigned)filp->f_pos);

	//
	filp->f_pos=0x0;
	r=filp->f_op->read(filp,buf,maxlen,&filp->f_pos);
	printk("Read %d bytes \n",r);
	int i=0;
	for(i=0; i<48; i++)
	printk("%02x ", (unsigned char)buf[i]);

	

	filp_close(filp,NULL);
	set_fs(oldfs);

	return r;
#if 0

	char path[64];
	char buf[32];
	int range;
	dev_t res;
	char *s;
	int len;
	int fd;
	unsigned int maj, min;

	/* read device number from .../dev */

	sprintf(path, "/sys/block/%s/dev", name);
	fd = sys_open(path, 0, 0);
	if (fd < 0)
		goto fail;
	len = sys_read(fd, buf, 32);
	sys_close(fd);

#endif

}

//----------------------------------------------------------------------------


int CMD_SendF( int argc, char* argv[] )
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	

	unsigned int i;
	unsigned short t;

	if(argc<1)
	{	dprintf("Usage: sendf <mode> <verify> [filename] [remote buff] [remote entry]\n");
		dprintf("mode: 0:not send, 1:send,  2: using file \n");	
		dprintf("Usage: sendf 1 1\n");	
		dprintf("Usage: sendf 2 1 nfjrom 80500000 80500000\n");						
		return;
	}
	
	unsigned char* local_boot_start; 
	unsigned int local_boot_len;

	unsigned char *local_kern_addr;	
	unsigned int local_kern_len;

	
	int remain;
	unsigned int retry;
	unsigned int time_start,time_end;	


	int verify=0,send=1;
	//dprintf("argc=%d argv[0]=%x\r\n",argc,*(char *)argv[0]);
	if(argc >=2 )
	{
		send= _atoi((const char*)(argv[0]),  16);	
		verify= _atoi((const char*)(argv[1]),  16);
	
	}

	if(send<=1)
	{
		local_boot_start=boot98;
		local_boot_len=sizeof(boot98)/sizeof(char);	
		local_kern_addr=boot98;
		local_kern_len=sizeof(boot98)/sizeof(char);

		//s0	
		//dprintf("1'st file start=0x%x, size=%d \n",bootstart, bootlen);
		dprintf("2'nd file start=0x%x, size=%d bytes\n",local_kern_addr, local_kern_len);			
	}
	//-------------
	
	if(send==2)  //load file
	{
		printk("open filename=%s \n", argv[2]);
	
		local_kern_len=2*1024*1024;  // 2M
		local_kern_addr=kmalloc(local_kern_len, GFP_ATOMIC);	
		if(local_kern_addr<=0)
		{	printk("alloc mem  Fail !\n"); 
                        return 0; 
		}


		int rc=ReadFileinKernel(argv[2],local_kern_addr, local_kern_len);
		if(rc==0)
		{	printk("Load kernel file Fail !\n"); 
                        return 0; 
                };

		local_kern_len=rc;
		if(rc%4)
			local_kern_len+=(4-rc%4);

		send==1;


	}
	unsigned int remote_kern_imgstart=NFBI_KERNADDR;
	unsigned int remote_kern_imgentry=NFBI_KERNADDR;
	
	if(argc >=4 )
		remote_kern_imgstart= strtoul((const char*)(argv[3]), (char **)NULL, 16);	
	if(argc >=5 )		
		remote_kern_imgentry= strtoul((const char*)(argv[4]), (char **)NULL, 16);	


	
	//--------------------------------------s2 read NeedBootCode
#if 0	
	dprintf("Waiting NeedBootCodeBit=1...");
	retry=3;
	while(1)
	{
		if(NFBI_MIIRead(NFBI_ISR) & NFBI_INT_NeedBootCode )
		{
			dprintf("PASS! NeedBootCode=1\r\n");
			break;
		}
		if(--retry ==0)
		{	dprintf("timeout! Read NeedBootCode bit Error!\r\n");	break;	}	
	}

#else
	dprintf("NeedBootCodeBit=%x\n", (REG32_R(SPE_DMA_ISR) & SPE_DMA_ISR_NEEDBTCODE)>>31);
#endif
	//------------------------------------------s3
	SettingDRAMConfig();

	//--------------------------------------s4 write bootcode
#if 0	
	time_start=get_timer_jiffies();		
	NFBI_SendCode(NFBI_BOOTADDR, bootstart, bootlen, send, verify);
	time_end=get_timer_jiffies();	
	dprintf("send 1'st file, spend time=%d sec \n", (time_end-time_start)*10/1000); 	
#else
	SettingJumpCode(NFBI_CPUBOOTADDR,  remote_kern_imgentry);
#endif
	//---------------------------------s5 write kernel	
	
	//time_start=get_timer_jiffies();		
	NFBI_SendCode(remote_kern_imgstart, local_kern_addr, local_kern_len, send, verify);
	//time_end=get_timer_jiffies();	
	//dprintf("send 2'nd file, spend time=%d sec\n", (time_end-time_start)*10/1000); 	
	
	//----------------------------------
	REG32_W(SPE_DMA_ISR, REG32_R(SPE_DMA_ISR)|SPE_DMA_ISR_NEEDBTCODE); //clear needbootcode
	if( (REG32_R(SPE_DMA_ISR) & SPE_DMA_ISR_NEEDBTCODE)  == SPE_DMA_ISR_NEEDBTCODE)
		dprintf("Cannot Clear needbootcode bit \n");

	dprintf("Start to booting \n");	
	REG32_W(SPE_NFBI_CMD, REG32_R(SPE_NFBI_CMD)|1); //start run boot code
	if( (REG32_R(SPE_NFBI_CMD) &1) != 1)
		dprintf("Cannot pull high StartRunBootCode bit \n");


	if(argc>=3)
	{	if(local_kern_addr)
			kfree(local_kern_addr);
	}
}
	

//---------------------------------------------------------------------------
// return x;   0: mean communication fail, 1: communication scuess 
// retval:     command return val;
//----------------------------------------------------------------------------
unsigned int Send_ISM_Cmd(unsigned int mode, unsigned int argv1, unsigned int argv2, unsigned int argv3, unsigned int *retval)
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	int rc=0;
	if(REG32_R(SPE_ISM_LR)<0x10)
	{
		dprintf("Error ISM Length is 0 \n");
		return;
	}
	REG32_W(SPE_ISM_OR,0);
	
	REG32_W(SPE_ISM_DR, ('w'<<24)|('e' <<16) | ('i'<<8) | (mode<<0) );	//fill magic word

	REG32_W(SPE_ISM_DR, argv1);
	REG32_W(SPE_ISM_DR, argv2);
	REG32_W(SPE_ISM_DR, argv3);


	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD) |SPE_DMA_IOCMD_SWINT);  //trigger

	int st,et,wait=3000;
	st=get_jiffies_64();
				
	while(1)
	{

		if(SPE_IntFlag& SPE_DMA_ISR_SWINT)
			break;
		if(REG32_R(SPE_DMA_ISR)&SPE_DMA_ISR_SWINT)
			break;

		et=get_jiffies_64();
		
			if((et-st)  > wait)
			{	
				dprintf("ISM timeout!\n");	
				return rc;
			}

	}
	
	//read back
	mdelay(1000);
	REG32_W(SPE_ISM_OR, 0);
	int mag=REG32_R(SPE_ISM_DR);
	if(mag== ( ('w'<<24)|('e' <<16) | ('i'<<8) | (mode<<0) ))
	{	dprintf("Fail! ISM MAG1=%x \n",mag);
		return rc;
	};	
	
	if(mag!= (('w'<<24)|('e' <<16) | ('o'<<8) | (mode<<0) ))
	{	dprintf("Fail ISM MAG2=%x \n",mag);
		return rc;
	};	
	
		
	rc=1;
	unsigned int a1=0,a2=0,a3=0;
	a1=REG32_R(SPE_ISM_DR);
	a2=REG32_R(SPE_ISM_DR);
	a3=REG32_R(SPE_ISM_DR);
	

	switch(mode)
	{
		case SPE_ISM_MAG_REGR:		*retval=a3; break;
		case SPE_ISM_MAG_REGW:     *retval=1; break;
		case SPE_ISM_MAG_DMARST: *retval=1; break;		

	}
	return rc;


}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int CMD_IsmRead(int argc, char* argv[])
{
	unsigned int rc,val,retval;
	unsigned int check=0,mask=0xffffffff,expval=0;

	if(argc<1)
	{//dump all	
	       dprintf("\n"); 		   
		dprintf("ismr reg mask : expval \n" );
		return 0;
	}

	unsigned int reg= strtoul((const char*)(argv[0]), (char **)NULL, 16);


	
	rc=Send_ISM_Cmd(SPE_ISM_MAG_REGR,reg,0,0, &retval);
	if(rc==0)
	{
		printk("Fail ISM commnuication !\n");
		return 0;
	}
	val=retval;
	//regr iocmd
	if(argc==1)
	{	
	}
	//regr iocmd 0001
	else if(argc==2)
	{
		mask = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	}
	//regr iocmd : 0x0001
	else if(   argc>=3  &&  *(argv[1])==':' )
	{	check=1;
		expval = strtoul((const char*)(argv[2]), (char **)NULL, 16);	
	}
	//regr iocmd 0x0001 : 0x0001
	else if(argc>=3  && *(argv[1]) != '\0')
	{	
		 mask = strtoul((const char*)(argv[1]), (char **)NULL, 16);
		
		if(argc>=3 && *(argv[2]) == ':')
		{	check=1;
			expval = strtoul((const char*)(argv[3]), (char **)NULL, 16);	
		}
	}


	//verify
	if(!check)
	{
		dprintf(" %s=%08x \n",  argv[0],val&mask );			
	}
	else
	{
		if( (val&mask) !=expval)
		{	dprintf("Fail, %s val=%x, expval=%x \n", argv[0], val, expval);
			at_errcnt++;
		}
		else
			dprintf("Pass \n");
	}	
	return 1;

}
//----------------------------------------------------------------------------
int CMD_IsmWrite(int argc, char* argv[])
{
	if(argc<2)
	{	 
		dprintf("ismw <reg> <val> \n");		
		dprintf("ismw <reg> <mask> <value>\n");			
		dprintf("ex: ismw b8000030  ffffffff \n");			
		return;	
	}

//	int off = strtoul((const char*)(argv[0]), (char **)NULL, 16);
	unsigned int mask=0,val,retval;
	unsigned int *addr;

	unsigned int reg= strtoul((const char*)(argv[0]), (char **)NULL, 16);
	
	//regw iocmd 0x0001
	if(argc==2)
	{	mask=0;
		val = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	}

	//regw iocmd 0x0001 0x0001
	else if(argc>=3)
	{	mask = strtoul((const char*)(argv[1]), (char **)NULL, 16);
		val = strtoul((const char*)(argv[2]), (char **)NULL, 16);
	}
		
	if(Send_ISM_Cmd(SPE_ISM_MAG_REGW,reg,mask,val,&retval)==0)
	{	dprintf("Write Fail, maybe ISM have error! \n");
	}
	
}



//----------------------------------------------------------------
void CMD_Drst(int argc, char* argv[])
{
	struct net_device *dev=reNet;
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	
	struct pci_dev *mydev=tp->pci_dev;
	int mybus_num=mydev->bus->number;
//	printk("my bus name=%s no=%x \n", mydev->bus->name,  mybus_num);
/*
	struct pci_bus *mondev=mydev->bus->parent;	
	printk("parent bus=%x \n", mondev->number );	
	printk("parent pri_bus=%x, sec_bus=%x, sub_bus=%x \n", mondev->primary, mondev->secondary, mondev->subordinate);	
	printk("parent bus name=%s  \n", mondev->name);	
*/
/*
	struct pci_dev *mondev=pci_get_device(0x8086, 0x27d0,  NULL);
	printk("Parent name=%x \n", mondev->resource[0].name);
	printk("Parent bar0=%x \n", mondev->resource[0].start);
	printk("Parent bar1=%x \n", mondev->resource[1].start);
*/	
/*
	struct pci_dev *parent_dev;	
	parent_dev=pci_find_bus(0,3);
	printk("parent bus Vender=%x \n", parent_dev->vendor);
	printk("parent bus Device=%x \n", parent_dev->device);	
	printk("parent bus bar=%x \n", parent_dev->resource[0]);
*/
	if( argc < 1 ) 
	{
		dprintf("drst nfbi: nfbi system rst\n");
		dprintf("drst dma: dma rst\n");		
		return ;
	}

	StrUpr( argv[0] );		
	
	 if( ! strcmp( argv[0], "NFBI" ) )
	 {
		REG32_W(SPE_NFBI_CMD, REG32_R(SPE_NFBI_CMD) | NFBI_CMD_SYSRST);
		printk("Do NFBI system rst! \n");
	 }
	 else if( ! strcmp( argv[0], "DMA" ) )
	 {	
	 	printk("Do DMA rst! \n");

	 	unsigned int rc,retval;
		rc=Send_ISM_Cmd(SPE_ISM_MAG_DMARST, 0, 0, 0, &retval);
		if(rc==0)
		{
			printk("ISM fail \n");
			return 0;
		};
		if(retval==1)
		{
			mdelay(1000);		
			DMA_Restart(dev);
		}
	 }
	 
}
//------------------------------------------------------------------------------

#endif  //end host
//---------------------------------------------------------------------------

//----------------------------------------------------------------
int CMD_ENIRQ(int argc, char* argv[])
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	struct net_device *dev= tp->dev;
	
	if( argc < 1 ) 
	{
		dprintf("enirq <0: disable, 1:enable> \n");
		return;
	}
	unsigned int en = strtoul((const char*)(argv[0]), (char **)NULL, 16);	
	
	if(en)
	{
		int retval = request_irq(dev->irq, rtl8198_interrupt, (tp->features & RTL_FEATURE_MSI) ? 0 : SA_SHIRQ, dev->name, dev);
		if (retval < 0)
		{
			printk("request irq fail \n");
			return 0;
		}
	}
	else
	{
		free_irq(dev->irq, dev);
	}
}

//-----------------------------------------------------------------
//----------------------------------------------------------------
int CMD_DMALoop(int argc, char* argv[])
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	int i;
	unsigned int   st=0,et=0,timeout=0;
	if( argc < 1 ) 
	{
		dprintf("dmaloop <mode> <pknum> <pklen> <timeout> <para1>\n");
		dprintf("dmaloop txseq 8 0\n");		
		dprintf("dmaloop loop 128 128 10\n");
		dprintf("dmaloop ism 1 128 10\n");		
		return 0;
	}

	StrUpr( argv[0] );	
	unsigned int pknum,txpklen,wait=3;
	unsigned int para1=0;
	
	if(argc>=2)
		pknum = strtoul((const char*)(argv[1]), (char **)NULL, 16);	
	if(argc>=3)	
		txpklen = strtoul((const char*)(argv[2]), (char **)NULL, 16);	
	if(argc>=4)	
		wait = strtoul((const char*)(argv[3]), (char **)NULL, 16);		
	if(argc>=5)	
		para1 = strtoul((const char*)(argv[4]), (char **)NULL, 16);		
	
	unsigned int rxpklen=0;

	//------------------------------------------
/*	
	DMATEST_TxPktMaxNum=pknum;	
	DMATEST_TxPktLen=txpklen;		

	DMATEST_TxPktNum=0;	
*/	
	//-------------------------------------------
	if( ! strcmp( argv[0], "LOOP" ) )
	{
		tp->DMA_mode=DMAMODE_HOSTTX_LOOP;
		unsigned char *pbuff;
		int failcnt=0;
		st=get_jiffies_64();

		int end=0;
		int timeout=0;
		int txpkcnt=0;
		while(!end)
		{	
			printk("pktgen \n");
			if(txpklen==0)
				txpklen=pktgen(PLTGEN_AllRand, tp->DMA_buff,0,0);
			else
				//pktgen(PKTGEN_LenFix|PKTGEN_ValRand, DMA_buff,txpklen,0);
				pktgen(PKTGEN_LenFix|PKTGEN_ValFix, tp->DMA_buff,txpklen,txpkcnt+1);		
			printk("tx \n");
			while(rtl8198_hw_tx(tp->DMA_buff, txpklen,1,1,    DMAMODE_HOSTTX_LOOP,0,0   ,0)==-1)
			{	
				et=get_jiffies_64();
				if((et-st)>wait*100)
				{	timeout=1;
					dprintf("Wait tx timeout!, pktnum=%d\n", txpkcnt);
					if(txpkcnt==0)
						at_errcnt++;					
					break;
				}
			}	

			printk("rx \n");			
			while(rtl8198_hw_rx(&pbuff, &rxpklen, 0,0,0, 0,0)==-1)			
			{
				et=get_jiffies_64();
				if((et-st)> wait*100)
				{	timeout=1;
					dprintf("Wait rx timeout!, pktnum=%d\n", txpkcnt);
					if(txpkcnt==0)
						at_errcnt++;
					break;
				}					
			}	


			et=get_jiffies_64();
			if((et-st)>wait*100)
			{	timeout=1;
				dprintf("Test time finish! \n");
				break;
			}

				
			if(timeout==1)			
				break;
			printk("compare \n");			
			//compare
			if(txpklen == rxpklen)
			{				
				//if(memcmp(DMA_buff, pbuff, txpklen)!=0)
				for(i=0; i<txpklen; i++)
					if(tp->DMA_buff[i]!=pbuff[i])				
					{	dprintf("Fail at txpkcnt=%x, txbuf[%x]=%x, rxbuf[%x]=%x \n",txpkcnt, i,tp->DMA_buff[i], i,pbuff[i]  );
						at_errcnt++;
						failcnt++;
					}
			}
			else
			{
				dprintf("Fail at txpkcnt=%x, txpklen=%x, rxpklen=%x \n",txpkcnt,txpklen,rxpklen  );
				at_errcnt++;
				failcnt++;
			}


			//check if stop dma test
			txpkcnt++;
			if(pknum!=0)
			{	
				if(txpkcnt >= pknum)
				{	end=1;					
				}
			}
			
				
		}

		tp->DMA_mode=DMAMODE_DEFAULT;	
		dprintf("Tx scuess PktCnt=%x \n", txpkcnt);	
		dprintf("Tx spend time=%d sec \n", (et-st)/100);			
		dprintf("Loop FailCnt=%d \n", failcnt);
		

	} 

}

