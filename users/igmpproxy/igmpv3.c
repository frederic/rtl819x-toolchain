#include "mclab.h"
#include "timeout.h"
#include "igmpproxy.h"
#include <fcntl.h>
#include <signal.h>
#ifdef CONFIG_IGMPV3_SUPPORT

#if 0
#define IGMPV3LOG	printf
#else
#define IGMPV3LOG(...)	while(0){}
#endif

#define IGMPV3_MAX_SRCNUM	64

__u32 gsrctmp[IGMPV3_MAX_SRCNUM];

int igmpv3_query( struct mcft_entry *entry, int srcnum, __u32 *srcfilter );



struct src_entry *add_to_srclist(struct mcft_entry *mcp, __u32 src)
{
	struct src_entry *p;
	
	if(mcp==NULL) return NULL;

	IGMPV3LOG( "%s> group=%s", __FUNCTION__, inet_ntoa( mcp->grp_addr ) );
	IGMPV3LOG( ", src=%s\n", inet_ntoa( src ) );
	
	p = mcp->srclist;
	while (p) {
		if (p->srcaddr == src)
			return p;
		p = p->next;
	}
	
	p = malloc(sizeof(struct src_entry));
	if (!p) {
		return NULL;
	}
	memset( p, 0, sizeof(struct src_entry) );
	p->srcaddr = src;
	p->timer.lefttime = 0;
	p->timer.retry_left = 0;
	//p->mrt_state = 0;
	p->next = mcp->srclist;
	mcp->srclist = p;
	return p;
}

int del_from_srclist(struct mcft_entry *mcp, __u32 src)
{
	struct src_entry **q, *p;

	if(mcp==NULL) return  -1;
	
	IGMPV3LOG( "%s> group=%s", __FUNCTION__, inet_ntoa( mcp->grp_addr ) );
	IGMPV3LOG( ", src=%s\n", inet_ntoa( src ) );
	
	q = &mcp->srclist;
	p = *q;
	while (p) {
		if(p->srcaddr == src) {
			*q = p->next;
			free(p);
			return 0;
		}
		q = &p->next;
		p = p->next;
	}
	
	return 0;
}

struct src_entry * get_specific_src(struct mcft_entry *mcp, __u32 src)
{
	struct src_entry **q, *p;
	
	if(mcp==NULL) return NULL;
	
	IGMPV3LOG( "%s> group=%s", __FUNCTION__, inet_ntoa( mcp->grp_addr ) );
	IGMPV3LOG( ", src=%s\n", inet_ntoa( src ) );
	
	q = &mcp->srclist;
	p = *q;
	while (p) {
		if(p->srcaddr == src)
		{
			return p;
		}
		q = &p->next;
		p = p->next;
	}
	
	return NULL;
}

int get_srclist_num( struct mcft_entry *mcp )
{
	int ret=0;
	struct src_entry *p;

	if(mcp==NULL) return ret;
	
	p = mcp->srclist;
	while(p)
	{
		ret++;
		p=p->next;
	}

	IGMPV3LOG( "%s> group:%s has %d source(s)\n", __FUNCTION__, inet_ntoa( mcp->grp_addr ), ret );

	return ret;
}

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int igmp_allow_new_source( __u32 group,__u32 src )
{
	struct ip_mreq_source mreq_s;
	struct IfDesc *up_dp = NULL;
	int ret;
	int idx;
	
	IGMPV3LOG( "%s> allow new source the group=%s\n", __FUNCTION__, inet_ntoa( group ) );

	/* join multicast group */
	mreq_s.imr_multiaddr = group;
	mreq_s.imr_sourceaddr =src;
	for(idx=0;idx<igmp_up_if_num;idx++){
		up_dp = getIfByName(igmp_up_if_name[idx]);
		mreq_s.imr_interface = up_dp->InAdr.s_addr;
		ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (void*)&mreq_s, sizeof(mreq_s));
		if(ret)
			printf("setsockopt IP_ADD_SOURCE_MEMBERSHIP %s error!\n", inet_ntoa(mreq_s.imr_multiaddr));
		}
	return ret; 	
}

int igmp_add_group( __u32 group )
{
	struct ip_mreq mreq;
	struct IfDesc *up_dp ;
	int ret;
    int idx ;
		
	IGMPV3LOG( "%s> join the group=%s\n", __FUNCTION__, inet_ntoa( group ) );

	/* join multicast group */
	mreq.imr_multiaddr.s_addr = group;
	
	for(idx=0;idx<igmp_up_if_num;idx++){
	   up_dp = getIfByName(igmp_up_if_name[idx]);	
	   mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	   ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	   if(ret)
		printf("setsockopt IP_ADD_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
	}	
	return ret;		
}

int igmp_del_group( __u32 group )
{
	struct ip_mreq mreq;
	struct IfDesc *up_dp ;
	int idx ;
        int ret ;
	IGMPV3LOG( "%s> leave the group=%s\n", __FUNCTION__, inet_ntoa( group ) );

	/* drop multicast group */
	mreq.imr_multiaddr.s_addr = group;
	
	for(idx=0;idx<igmp_up_if_num;idx++){
		
		up_dp = getIfByName(igmp_up_if_name[idx]);	
		mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
		ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
		if(ret)
			printf("setsockopt IP_DROP_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
	}	

	return ret;
}
#else
int igmp_allow_new_source( __u32 group,__u32 src )
{
	struct ip_mreq_source mreq_s;
	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
	int ret;
	
	IGMPV3LOG( "%s> allow new source  source=%s\n", __FUNCTION__, inet_ntoa(src) );

	/* join multicast group */
	mreq_s.imr_multiaddr = group;
	mreq_s.imr_interface = up_dp->InAdr.s_addr;
	mreq_s.imr_sourceaddr =src;
	ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (void*)&mreq_s, sizeof(mreq_s));
	if(ret)
		printf("setsockopt IP_ADD_SOURCE_MEMBERSHIP %s error!\n", inet_ntoa(mreq_s.imr_multiaddr));

	return ret; 	
}

int igmp_add_group( __u32 group )
{
	struct ip_mreq mreq;
	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
	int ret;

	IGMPV3LOG( "%s> join the group=%s\n", __FUNCTION__, inet_ntoa( group ) );

	/* join multicast group */
	mreq.imr_multiaddr.s_addr = group;
	mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	if(ret)
		printf("setsockopt IP_ADD_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));

	return ret;		
}

int igmp_del_group( __u32 group )
{
	struct ip_mreq mreq;
	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
	int ret;

	IGMPV3LOG( "%s> leave the group=%s\n", __FUNCTION__, inet_ntoa( group ) );

	/* drop multicast group */
	mreq.imr_multiaddr.s_addr = group;
	mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	//if(ret)
	//	printf("setsockopt IP_DROP_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
	return ret;
}
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int igmp_add_mr( __u32 group, __u32 src, int enable )
{
	struct MRouteDesc	mrd;
        int idx ;
	/* add multicast routing entry */
	//mrd.OriginAdr.s_addr = src;
	mrd.SubsAdr.s_addr = 0;
	mrd.McAdr.s_addr = group;

	
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
	mrd.TtlVc[igmp_down_if_idx] = enable;	

	IGMPV3LOG( "%s> group:%s", __FUNCTION__, inet_ntoa(mrd.McAdr) );
	IGMPV3LOG( ", src:%s, enable:%d\n", inet_ntoa(src), enable );
	for(idx=0;idx<igmp_up_if_num;idx++)
	{    
		mrd.InVif = igmp_up_if_idx[idx];
		if (src)
			mrd.OriginAdr.s_addr = src;
		else
			mrd.OriginAdr.s_addr =igmp_up_if_idx[idx];
		addMRoute(&mrd);
	}
	return (1);
}

int igmp_del_mr( __u32 group, __u32 src )
{
	struct MRouteDesc	mrd;
	int ret=0;
        int idx;
		
	/* delete multicast routing entry */
	//mrd.OriginAdr.s_addr = src;
	mrd.McAdr.s_addr = group;

	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));

	IGMPV3LOG( "%s> group:%s", __FUNCTION__, inet_ntoa(mrd.McAdr) );
	IGMPV3LOG( ", src:%s\n", inet_ntoa(src) );
	for(idx=0;idx<igmp_up_if_num;idx++){
       mrd.InVif = igmp_up_if_idx[idx];
	   if (src)
			mrd.OriginAdr.s_addr = src;
	   else
	   		mrd.OriginAdr.s_addr =igmp_up_if_idx[idx];
	   delMRoute(&mrd);
	}
	return ret;
}
#else
int igmp_add_mr( __u32 group, __u32 src, int enable )
{
	struct MRouteDesc	mrd;

	/* add multicast routing entry */
	if(src){
		mrd.OriginAdr.s_addr = src;
	}
	else{
		mrd.OriginAdr.s_addr =igmp_up_if_idx;
	}
	//mrd.OriginAdr.s_addr = src;
	mrd.SubsAdr.s_addr = 0;
	mrd.McAdr.s_addr = group;

	mrd.InVif = igmp_up_if_idx;
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
	mrd.TtlVc[igmp_down_if_idx] = enable;	

	IGMPV3LOG( "%s> group:%s", __FUNCTION__, inet_ntoa(mrd.McAdr) );
	IGMPV3LOG( ", src:%s, enable:%d\n", inet_ntoa(mrd.OriginAdr), enable );

	return (addMRoute(&mrd));
}

int igmp_del_mr( __u32 group, __u32 src )
{
	struct MRouteDesc	mrd;
	int ret=0;

	/* delete multicast routing entry */
	if(src){
	
		mrd.OriginAdr.s_addr = src;
	}
	else{
		mrd.OriginAdr.s_addr =igmp_up_if_idx;
		
	}
	//mrd.OriginAdr.s_addr = src;
	mrd.McAdr.s_addr = group;
	mrd.InVif = igmp_up_if_idx;
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));

	IGMPV3LOG( "%s> group:%s", __FUNCTION__, inet_ntoa(mrd.McAdr) );
	IGMPV3LOG( ", src:%s\n", inet_ntoa(mrd.OriginAdr) );

	delMRoute(&mrd);	
	return ret;
}
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int igmp_set_srcfilter( struct mcft_entry *p )
{
	struct ip_msfilter *imsfp;
	int	size,i;
	struct IfDesc *up_dp ;
	__u32 group;
	struct src_entry *s;
	int idx;
	if(p==NULL)	
		return -1;

	  
    //        up_dp = getIfByName(igmp_up_if_name[idx]);
			
	   /*use the "send_buf buffer*/
	   imsfp = (struct ip_msfilter *)send_buf;
	   imsfp->imsf_multiaddr=p->grp_addr;
//	   imsfp->imsf_interface=up_dp->InAdr.s_addr;
	   imsfp->imsf_fmode=p->filter_mode;
	   imsfp->imsf_numsrc=0;
	   IGMPV3LOG( "%s> maddr:%s", __FUNCTION__, inet_ntoa(imsfp->imsf_multiaddr) );
	   IGMPV3LOG( ", if:%s, fmode:%d\n", inet_ntoa(imsfp->imsf_interface), imsfp->imsf_fmode  );

	   i=0;
	   s=p->srclist;
	     while(s)
	   {
		IGMPV3LOG( "%s>try to match=> fmode:%d, timer:%d, slist:%s\n", __FUNCTION__, p->filter_mode, s->timer.lefttime, inet_ntoa(s->srcaddr) );
		if( ((p->filter_mode==MCAST_INCLUDE) && (s->timer.lefttime>0)) ||
		    ((p->filter_mode==MCAST_EXCLUDE) && (s->timer.lefttime==0)) )
		{
			imsfp->imsf_slist[i] = s->srcaddr;
			IGMPV3LOG( "%s> slist:%s\n", __FUNCTION__, inet_ntoa(imsfp->imsf_slist[i]) );
			i++;
		}
		s=s->next;
	   }
	   imsfp->imsf_numsrc=i;
	   size = IP_MSFILTER_SIZE( i );
	   IGMPV3LOG( "%s> numsrc:%d, size:%d\n", __FUNCTION__, imsfp->imsf_numsrc, size );

	   
	    for(idx=0;idx<igmp_up_if_num;idx++)
          {
                  up_dp = getIfByName(igmp_up_if_name[idx]);
                  imsfp->imsf_interface=up_dp->InAdr.s_addr;
		 if (setsockopt(up_dp->sock, IPPROTO_IP, IP_MSFILTER, imsfp,size) < 0 )
	        {
		perror("setsockopt IP_MSFILTER"); 
        	return -1;
	       }		  
	  }  
	return 0;
}
#else
int igmp_set_srcfilter( struct mcft_entry *p )
{
	struct ip_msfilter *imsfp;
	int	size,i;
	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
	__u32 group;
	struct src_entry *s;

	if(p==NULL)	return -1;

	/*use the "send_buf buffer*/
	imsfp = (struct ip_msfilter *)send_buf;
	imsfp->imsf_multiaddr=p->grp_addr;
	imsfp->imsf_interface=up_dp->InAdr.s_addr;
	imsfp->imsf_fmode=p->filter_mode;
	imsfp->imsf_numsrc=0;
	IGMPV3LOG( "%s> maddr:%s", __FUNCTION__, inet_ntoa(imsfp->imsf_multiaddr) );
	IGMPV3LOG( ", if:%s, fmode:%d\n", inet_ntoa(imsfp->imsf_interface), imsfp->imsf_fmode  );

	i=0;
	s=p->srclist;
	while(s)
	{
		IGMPV3LOG( "%s>try to match=> fmode:%d, timer:%d, slist:%s\n", __FUNCTION__, p->filter_mode, s->timer.lefttime, inet_ntoa(s->srcaddr) );
		if( ((p->filter_mode==MCAST_INCLUDE) && (s->timer.lefttime>0)) ||
		    ((p->filter_mode==MCAST_EXCLUDE) && (s->timer.lefttime==0)) )
		{
			imsfp->imsf_slist[i] = s->srcaddr;
			IGMPV3LOG( "%s> slist:%s\n", __FUNCTION__, inet_ntoa(imsfp->imsf_slist[i]) );
			i++;
		}
		s=s->next;
	}
	imsfp->imsf_numsrc=i;
	size = IP_MSFILTER_SIZE( i );
	IGMPV3LOG( "%s> numsrc:%d, size:%d\n", __FUNCTION__, imsfp->imsf_numsrc, size );

	if (setsockopt(up_dp->sock, IPPROTO_IP, IP_MSFILTER, imsfp,size) < 0 )
	{
		//perror("setsockopt IP_MSFILTER"); 
        	return -1;
	}

	return 0;
}
#endif
int check_src_set( __u32 src, struct src_entry *srclist )
{
	struct src_entry *p;
	for( p=srclist; p!=NULL; p=p->next )
	{
		if ( src == p->srcaddr )
			return 1;
	}
	return 0;
}

int check_src( __u32 src, __u32 *sources, int numsrc )
{
	int i;
	for (i=0;i< numsrc; i++)
	{
		if( src == sources[i] )
			return 1;
	}
	return 0;
}

void handle_igmpv3_isex( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	// Mason Yu Test
	//printf("handle_igmpv3_isex\n");
		
	if(!IN_MULTICAST(group))
		return;
	/* check if it's protocol reserved group */
	if((group&0xFFFFFF00)==0xE0000000){
		//printf("[%s]:It's protocol | reserved group!\n",__FUNCTION__);
		return;
	}
	
	if(!chk_mcft(group)) 
	{
		//return;
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( group );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set,*s_next;
				
				// Mason Yu Test
				//printf("handle_igmpv3_isex: MCAST_INCLUDE\n");
	
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif	

				//IN(A), IS_EX(B) => EX(A*B, B-A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{	
					//(B-A)=0 
					if (check_src_set(grec_src[i],old_set)==0)
					{
						s = add_to_srclist( mymcp, grec_src[i] );
						if(s)
						{
							s->timer.lefttime = 0;
							s->timer.retry_left = 0;
							igmp_add_mr( mymcp->grp_addr, s->srcaddr, 0 );
						}
					}
				}
				
				s = old_set;
				while(s)
				{
					s_next=s->next;
					//Delete (A-B)
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						igmp_del_mr( mymcp->grp_addr, s->srcaddr );
						del_from_srclist( mymcp, s->srcaddr );
					}					
					s = s_next;
				}

				//Group Timer=GMI
				mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left = MEMBER_QUERY_COUNT;
				
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set,*s_next;
				
				// Mason Yu Test
				//printf("handle_igmpv3_isex: MCAST_EXCLUDE\n");
#ifdef KEEP_GROUP_MEMBER
				add_user(mymcp, src);
#endif

				//EX(X,Y), IS_EX(A) => EX(A-Y, Y*A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					// (A-X-Y)=GMI
					if( check_src_set( grec_src[i],old_set )==0 )
					{
						s=add_to_srclist( mymcp, grec_src[i] );
						if(s)
						{
							s->timer.lefttime = MEMBER_QUERY_INTERVAL;
							s->timer.retry_left = MEMBER_QUERY_COUNT;
							igmp_add_mr( mymcp->grp_addr, s->srcaddr, 1 );
						}
					}
				}
				
				s = old_set;
				while(s)
				{
					s_next =s->next;
					
					//Delete (X-A), Delete(Y-A)
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						igmp_del_mr( mymcp->grp_addr, s->srcaddr );
						del_from_srclist( mymcp, s->srcaddr );
					}
					s = s_next;
				}
								
				//Group Timer=GMI
				mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left = MEMBER_QUERY_COUNT;
				//printf("grp_addr:%s.mymcp->timer.lefttime=%d,mymcp->timer.retry_left=%d,[%s]:[%d].\n",inet_ntoa(group),mymcp->timer.lefttime,mymcp->timer.retry_left,__FUNCTION__,__LINE__);
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}


void handle_igmpv3_isin( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	if(!IN_MULTICAST(group))
		return;
	/* check if it's protocol reserved group */
	if((group&0xFFFFFF00)==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( group );
		
		
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;

				//printf("handle_igmpv3_isin: MCAST_INCLUDE\n");	
				
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif	

				//IN(A), IN(B) => IN(A+B)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					
					if( check_src_set(grec_src[i], old_set )==0 )
						igmp_add_mr( mymcp->grp_addr, grec_src[i], 1 );
					// (B)= GMI
					s = add_to_srclist( mymcp, grec_src[i] );
					if (s)
					{
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
					}
				}

				//set the new state
				mymcp->filter_mode = MCAST_INCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				//printf("handle_igmpv3_isin: MCAST_EXCLUDE\n");
				//EX(X,Y), IS_IN(A) => EX(X+A, Y-A)
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif	

				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (A)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						igmp_add_mr( mymcp->grp_addr, s->srcaddr, 1 );
					}
				}
				
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}

void handle_igmpv3_toin( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;	
	
	if(!IN_MULTICAST(group))
		return;
	/* check if it's protocol reserved group */
	if((group&0xFFFFFF00)==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( group );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;

				// Mason Yu Test
				//printf("handle_igmpv3_toin: MCAST_INCLUDE\n");
#ifdef KEEP_GROUP_MEMBER
				if ( srcnum != 0 ){

					add_user(mymcp, src);
				}
#endif

				//IN(A), TO_IN(B) => IN(A+B)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					
					if( check_src_set( grec_src[i], old_set )==0 )
						igmp_add_mr( mymcp->grp_addr, grec_src[i], 1 );
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (B)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
					}				
					
				}
				
				//send Q(G,A-B)
				i=0;
				s = old_set;
				while(s)
				{
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						gsrctmp[i]=s->srcaddr;
						
						/*lower A-B timer to LMQT*/
						s->timer.lefttime=LAST_MEMBER_QUERY_INTERVAL;
						s->timer.retry_left=LAST_MEMBER_QUERY_COUNT;
						
						i++;
						if(i==IGMPV3_MAX_SRCNUM) break;
					}					
					s = s->next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
				//set the new state
				mymcp->filter_mode = MCAST_INCLUDE;
				igmp_set_srcfilter( mymcp );	
				#ifdef KEEP_GROUP_MEMBER
				if ( srcnum == 0 ) {
					int count;
					count = del_user(mymcp, src);
					if (count == 0) {// no member, drop it!
						del_mr(mymcp->grp_addr);				
						del_mcft(mymcp->grp_addr);
					}
				}
				#endif
			}
			
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;

				// Mason Yu Test
				//printf("handle_igmpv3_toin: MCAST_EXCLUDE and srcnum=%d\n", srcnum);
#ifdef KEEP_GROUP_MEMBER
				if ( srcnum != 0 ){
					add_user(mymcp, src);
				}
#endif

				//EX(X,Y), TO_IN(A) => EX(X+A, Y-A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (A)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						igmp_add_mr( mymcp->grp_addr, s->srcaddr, 1 );
					}
				}	

				//send Q(G,X-A)
				i=0;
				s = old_set;
				while(s)
				{
					if( s->timer.lefttime>0 )
					{
						if( check_src( s->srcaddr, grec_src, srcnum )==0 )
						{
							gsrctmp[i]=s->srcaddr;

							/*lower X-A timer to LMQT*/
							s->timer.lefttime=LAST_MEMBER_QUERY_INTERVAL;
							s->timer.retry_left=LAST_MEMBER_QUERY_COUNT;
							
							i++;
							if(i==IGMPV3_MAX_SRCNUM) break;
						}
					}					
					s = s->next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
				/* lower group filter timer to LMQT*/
				mymcp->timer.lefttime=LAST_MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left=LAST_MEMBER_QUERY_COUNT;
				
				//send Q(G)
				if( mymcp->igmp_ver==IGMP_VER_3 )
					igmpv3_query( mymcp, 0, NULL );
				else
					igmp_query(ALL_SYSTEMS, mymcp->grp_addr, LAST_MEMBER_QUERY_INTERVAL);

				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
				
#ifdef KEEP_GROUP_MEMBER				
				if ( srcnum == 0 )
				{
					int count;
					count = del_user(mymcp, src);
					if (count == 0) {// no member, drop it!
						del_mr(mymcp->grp_addr);    			
						del_mcft(mymcp->grp_addr);
					}
				}
#endif
			}
			break;
		default:
			break;
		}
	}
}


void handle_igmpv3_toex( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	if(!IN_MULTICAST(group))
		return;
	/* check if it's protocol reserved group */
	if((group&0xFFFFFF00)==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( group );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set,*s_next;
				
				//printf("handle_igmpv3_toex: MCAST_INCLUDE\n");
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif
				//IN(A), TO_EX(B) => EX(A*B, B-A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					// (B-A)=0
					if( check_src_set(grec_src[i], old_set )==0 )
					{
						s = add_to_srclist( mymcp, grec_src[i] );
						if(s){
							
							s->timer.lefttime = 0;
							s->timer.retry_left = 0;
							igmp_add_mr( mymcp->grp_addr, s->srcaddr, 0 );
						}
					}
				}

				i=0;
				s = old_set;
				while(s)
				{
					s_next=s->next;
					
					//Delete (A-B)
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						igmp_del_mr( mymcp->grp_addr, s->srcaddr );
						del_from_srclist( mymcp, s->srcaddr );
					}		
					else
					{
						/*lower A*B timer to LMQT*/
						gsrctmp[i]=s->srcaddr;
						s->timer.lefttime=LAST_MEMBER_QUERY_INTERVAL;
						s->timer.retry_left=LAST_MEMBER_QUERY_COUNT;
						
						i++;
						if(i==IGMPV3_MAX_SRCNUM) 
							break;
					}
					s = s_next;
				}
				//send Q(G,A*B)
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
				
				//Group Timer=GMI
				mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left = MEMBER_QUERY_COUNT;
				
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set,*s_next;
				
				//printf("handle_igmpv3_toex: MCAST_EXCLUDE\n");
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif	
				//EX(X,Y), TO_EX(A) => EX(A-Y, Y*A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
									
					if( check_src_set( grec_src[i], old_set )==0 )
					{
						// (A-X-Y)=Group Timer
						s = add_to_srclist( mymcp, grec_src[i] );
						if(s){
							
							s->timer.lefttime = mymcp->timer.lefttime;
							s->timer.retry_left = MEMBER_QUERY_COUNT;
							igmp_add_mr( mymcp->grp_addr, s->srcaddr, 1 );
						}
					}
				}
				
				s = old_set;
				while(s)
				{
					s_next=s->next;
					
					//Delete (X-A), Delete(Y-A)
					if( check_src( s->srcaddr, grec_src, srcnum )==0 )
					{
						igmp_del_mr( mymcp->grp_addr, s->srcaddr );
						del_from_srclist( mymcp, s->srcaddr );
					}
					s = s_next;
				}

				//send Q(G,A-Y)
				i=0;
				s = mymcp->srclist;
				while(s)
				{
					s_next=s->next;				
					if( s->timer.lefttime > 0 )//A-Y
					{
						gsrctmp[i]=s->srcaddr;
						/*lower A-Y timer to LMQT*/
						s->timer.lefttime=LAST_MEMBER_QUERY_INTERVAL;
						s->timer.retry_left=LAST_MEMBER_QUERY_COUNT;
							
						i++;
						if(i==IGMPV3_MAX_SRCNUM) break;
					}
					s=s_next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
				//Group Timer=GMI
				mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
				mymcp->timer.retry_left = MEMBER_QUERY_COUNT;

				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}


void handle_igmpv3_allow( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	if(!IN_MULTICAST(group))
		return;
	/* check if it's protocol reserved group */
	if((group&0xFFFFFF00)==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		//igmp_add_group( group );
		
		int i=0;
		for(i=0;i<srcnum;i++)
		{
			igmp_allow_new_source( group,grec_src[i] );
		}
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				//printf("handle_igmpv3_allow: MCAST_INCLUDE\n");		
				
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif
				//IN(A), ALLOW(B) => IN(A+B)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					if( check_src_set( grec_src[i], old_set )==0 )
						igmp_add_mr( mymcp->grp_addr, grec_src[i], 1 );	
					
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (B)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						
					}
				}

				//set the new state
				mymcp->filter_mode = MCAST_INCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				
				//printf("handle_igmpv3_allow: MCAST_EXCLUDE\n");
	
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif
				//EX(X,Y), ALLOW(A) => EX(X+A, Y-A)
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					s = add_to_srclist( mymcp, grec_src[i] );
					if(s)
					{	// (A)= GMI
						s->timer.lefttime = MEMBER_QUERY_INTERVAL;
						s->timer.retry_left = MEMBER_QUERY_COUNT;
						igmp_add_mr( mymcp->grp_addr, s->srcaddr, 1 );
					}
				}
				
				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}

void handle_igmpv3_block( __u32 group, __u32 src, int srcnum, __u32 *grec_src )
{
	struct mcft_entry *mymcp;
	
	if(!IN_MULTICAST(group))
		return;
	/* check if it's protocol reserved group */
	if((group&0xFFFFFF00)==0xE0000000)
		return;

	if(!chk_mcft(group)) 
	{
		mymcp = add_mcft(group, src);
		if(!mymcp) return;		
		mymcp->igmp_ver = IGMP_VER_3;
		igmp_add_group( group );
	}
		
	mymcp = get_mcft(group);
	if(mymcp)
	{
		switch( mymcp->filter_mode )
		{
		case MCAST_INCLUDE:
			{
				int i;
				struct src_entry *s, *old_set;
				struct	src_entry *s_next;
				// Mason Yu Test
				//printf("handle_igmpv3_block: MCAST_INCLUDE\n");
#ifdef KEEP_GROUP_MEMBER				
				add_user(mymcp, src);
#endif

				/*IN(A),BLOCK(B) => IN(A)  send Q(G,A*B)*/			
				
				i=0;
				s = mymcp->srclist;
				while(s)
				{
					s_next=s->next;				
					if( check_src( s->srcaddr, grec_src, srcnum )==1 )
					{
						gsrctmp[i]=s->srcaddr;
						
						/*lower A*B timer to LMQT*/
						s->timer.lefttime=LAST_MEMBER_QUERY_INTERVAL;
						s->timer.retry_left=LAST_MEMBER_QUERY_COUNT;
						
						i++;
						if(i==IGMPV3_MAX_SRCNUM) break;
					}
					
					s=s_next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );
				
			
			}
			break;
		case MCAST_EXCLUDE:
			{
				int i;
				struct src_entry *s, *old_set,*s_next;
				
				//printf("handle_igmpv3_block: MCAST_EXCLUDE\n");

				//EX(X,Y), BLOCK(A) => EX( X+(A-Y), Y )
				old_set = mymcp->srclist;
				for(i=0;i<srcnum;i++)
				{
					// (A-X-Y)=Group Timer,and lower to LMQT later
					if( check_src_set( grec_src[i], old_set )==0 )
					{
						s = add_to_srclist( mymcp, grec_src[i] );
						if(s)
						{
							s->timer.lefttime = mymcp->timer.lefttime;
							s->timer.retry_left = MEMBER_QUERY_COUNT;
							
						  	igmp_add_mr( mymcp->grp_addr, s->srcaddr, 1 );
							
						}
					}
				}
				
				//send Q(G,A-Y)
				i=0;
				s = mymcp->srclist;
				while(s)
				{
					s_next=s->next;
					if( check_src( s->srcaddr, grec_src, srcnum )==1 )//
					{
						if( s->timer.lefttime > 0 )
						{
							gsrctmp[i]=s->srcaddr;
							
							/*lower A-Y timer to LMQT*/
							s->timer.lefttime=LAST_MEMBER_QUERY_INTERVAL;
							s->timer.retry_left=LAST_MEMBER_QUERY_COUNT;
							
							i++;
							if(i==IGMPV3_MAX_SRCNUM) break;
						}
					}
					s=s_next;
				}
				if(i>0) igmpv3_query( mymcp, i, gsrctmp );

				//set the new state
				mymcp->filter_mode = MCAST_EXCLUDE;
				igmp_set_srcfilter( mymcp );
			}
			break;
		default:
			break;
		}
	}
}

int igmpv3_query( struct mcft_entry *entry, int srcnum, __u32 *srcfilter )
{
    struct igmpv3_query	*igmpv3;
    struct sockaddr_in	sdst;
    struct IfDesc 	*dp = getIfByName(igmp_down_if_name);
    __u32	grp=0;
    int		i,totalsize=0;
	
	if(dp==NULL)
		return;
    if(entry) grp=entry->grp_addr;
    	
    igmpv3            = (struct igmpv3_query *)send_buf;
    igmpv3->type      = 0x11;
    igmpv3->code      = LAST_MEMBER_QUERY_INTERVAL;
    igmpv3->csum      = 0;
    igmpv3->group     = grp;
    igmpv3->resv      = 0;
    igmpv3->suppress  = 1;
    igmpv3->qrv       = 2;
    igmpv3->qqic      = MEMBER_QUERY_INTERVAL;
    igmpv3->nsrcs     = srcnum;
    IGMPV3LOG( "%s> send to group:%s, src:", __FUNCTION__, inet_ntoa( grp ) );
    for(i=0;i<srcnum;i++)
    {
    	igmpv3->srcs[i] = srcfilter[i];
    	IGMPV3LOG( "(%s)", inet_ntoa( igmpv3->srcs[i] ) );
    }
    totalsize	      = sizeof(struct igmpv3_query)+igmpv3->nsrcs*sizeof(__u32);
    igmpv3->csum      = in_cksum((u_short *)igmpv3, totalsize );
    IGMPV3LOG( "\n" );


    bzero(&sdst, sizeof(struct sockaddr_in));
    sdst.sin_family = AF_INET;
    if(grp)
    	sdst.sin_addr.s_addr = grp; 
    else 
    	sdst.sin_addr.s_addr = ALL_SYSTEMS;

    if (sendto(dp->sock, igmpv3, totalsize, 0, (struct sockaddr *)&sdst, sizeof(sdst)) < 0)
    {
	printf("igmpv3_query> sendto error, from %s ", inet_ntoa(dp->InAdr.s_addr));
	printf("to %s\n", inet_ntoa(sdst.sin_addr.s_addr));
    }

    return 0;
}



void handle_group_timer(void)
{
	struct mcft_entry *p,*next;
	
	p = mcpq;
	next = NULL;
	while( p!=NULL )
	{
		next = p->next;
		
		if( p->timer.lefttime )
		{
			p->timer.lefttime--;
			if( (p->timer.lefttime==0) && (p->timer.retry_left!=0) )
			{
				p->timer.retry_left--;
				if( p->timer.retry_left )
				{
					IGMPV3LOG("%s> GROUP TIMEOUT, send Query to group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );

					p->timer.lefttime = LAST_MEMBER_QUERY_INTERVAL;
					//if( p->igmp_ver==IGMP_VER_3 )
					//	igmpv3_query( p, 0, NULL );
					//else
						igmp_query(ALL_SYSTEMS, p->grp_addr, LAST_MEMBER_QUERY_INTERVAL);
				}
				
			}
			
			switch( p->filter_mode )
			{
			case MCAST_INCLUDE:
				break;
			case MCAST_EXCLUDE:
				if( p->timer.lefttime==0 )
				{
					struct src_entry *s, *s_next;
					
					IGMPV3LOG("%s> group:%s is timeout(EXCLUDE mode)\n", __FUNCTION__, inet_ntoa(p->grp_addr) );
					
					s=p->srclist;
					while(s)
					{
						s_next=s->next;
						if( s->timer.lefttime==0 )
						{
							//remove this source
							igmp_del_mr( p->grp_addr, s->srcaddr );
							del_from_srclist( p, s->srcaddr );
						}
						s=s_next;
					}
					
					if( p->srclist )
					{
						IGMPV3LOG("%s> group:%s changes to INCLUDE mode\n", __FUNCTION__, inet_ntoa(p->grp_addr) );

						p->filter_mode=MCAST_INCLUDE;
						igmp_set_srcfilter( p );
					}else{
						IGMPV3LOG("%s> remove group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );

						//delete this group record
						if(p->mrt_state)
						{
							igmp_del_mr( p->grp_addr, 0 );
							p->mrt_state=0;
						}
						igmp_del_group( p->grp_addr );
						del_mcft( p->grp_addr );
					}
				}
				break;
			default:
				break;
			}
			
		}

		p = next;
	}
}

void handle_src_timer(void)
{
	struct mcft_entry *p,*next;
	
	p = mcpq;
	next = NULL;
	while( p!=NULL )
	{
		struct src_entry *s, *src_next;
		int	change_sf=0;
		
		next = p->next;
		s = p->srclist;
		src_next = NULL;
		while( s )
		{
			src_next = s->next;
			
			if( s->timer.lefttime )
			{
				s->timer.lefttime--;
				if( (s->timer.lefttime==0) && (s->timer.retry_left!=0) )
				{
					s->timer.retry_left--;
					if( s->timer.retry_left )
					{
						IGMPV3LOG("%s> SRC TIMEOUT, send Query to group:%s", __FUNCTION__, inet_ntoa(p->grp_addr) );
						IGMPV3LOG(", src:%s\n", __FUNCTION__, inet_ntoa(s->srcaddr) );
						
						s->timer.lefttime = LAST_MEMBER_QUERY_INTERVAL;
						igmpv3_query( p, 1, &s->srcaddr );
					}
				}
				
					
				switch( p->filter_mode )
				{
				case MCAST_INCLUDE:
					if( s->timer.lefttime )
					{
						//forward this src
						if(p->mrt_state)
						{
							IGMPV3LOG("%s> stop all sources for group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );
							igmp_del_mr( p->grp_addr, 0 );
							p->mrt_state=0;
						}
					}else{
						IGMPV3LOG("%s> remove src:%s", __FUNCTION__, inet_ntoa(s->srcaddr) );
						IGMPV3LOG(" from group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );
						
						//==0, stop this src
						igmp_del_mr( p->grp_addr, s->srcaddr );
						del_from_srclist( p, s->srcaddr );
						//NO MORE SOURCE, DELETE GROUP RECORD
						change_sf=1;
					}
					break;
				case MCAST_EXCLUDE:
					if( s->timer.lefttime )
					{
						//forward this src
						if(p->mrt_state)
						{
							IGMPV3LOG("%s> stop all sources for group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );
							igmp_del_mr( p->grp_addr, 0 );
							p->mrt_state=0;
						}
					}else{
						IGMPV3LOG("%s> stop forwarding src:%s", __FUNCTION__, inet_ntoa(s->srcaddr) );
						IGMPV3LOG(" for group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );

						//==0, stop this src, do not remove record
						igmp_add_mr( p->grp_addr, s->srcaddr, 0 );
						change_sf=1;
					}
					break;
				default:
					break;
				}
			}				
			s = src_next;
		}
		
		//set the new state
		if(change_sf)	igmp_set_srcfilter( p );
		
		//EX( {}, X )
		if( (p->filter_mode==MCAST_EXCLUDE) && (p->srclist!=NULL) )
		{
			int allsrcinex=1;
			s = p->srclist;
			while(s)
			{
				if(s->timer.lefttime>0)
				{
					allsrcinex=0;
					break;
				}
				s=s->next;
			}
			if(allsrcinex==1)
			{
				if( p->mrt_state==0 )
				{
					IGMPV3LOG("%s> forward all sources for group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );

					igmp_add_mr( p->grp_addr, 0, 1 );
					p->mrt_state = 1;
				}
			}
		}
		
		//for empty condition
		if( p->srclist==NULL )
		{
			switch( p->filter_mode )
			{
			case MCAST_INCLUDE:
				//not foreward all source
				//delete this group record
				if(p->mrt_state)
				{
					IGMPV3LOG("%s> stop all sources for group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );

					igmp_del_mr( p->grp_addr, 0 );
					p->mrt_state=0;
				}
				IGMPV3LOG("%s> remove group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );
				igmp_del_group( p->grp_addr );
				del_mcft( p->grp_addr );
				break;
			case MCAST_EXCLUDE:
				//forward all source
				if( p->mrt_state==0 )
				{
					IGMPV3LOG("%s> forward all sources for group:%s\n", __FUNCTION__, inet_ntoa(p->grp_addr) );

					igmp_add_mr( p->grp_addr, 0, 1 );
					p->mrt_state = 1;
				}
				break;
			default:
				break;
			}			
		}
		
		p = next;
	}
}



static struct timeval start_time={0,0};
static int init_stat_time=0;
void igmpv3_timer(void)
{
	struct timeval cur_time;
	
	if( init_stat_time==0 )
	{
		gettimeofday( &start_time, NULL );
		init_stat_time = 1;
		return;
	}


	
	gettimeofday( &cur_time, NULL );
	if( (cur_time.tv_sec > (start_time.tv_sec+1)) ||
	    ((cur_time.tv_sec == (start_time.tv_sec+1)) && (cur_time.tv_usec >=start_time.tv_usec)) )
	{
		//suppose 1 second passed
		//printf( "." );fflush(NULL);
				
		handle_group_timer();
		handle_src_timer();		
		
		start_time.tv_sec = cur_time.tv_sec;
		start_time.tv_usec = cur_time.tv_usec;
	}

	return;
}





int igmpv3_accept(int recvlen, struct IfDesc *dp)
{
	register __u32 src, dst, group;
	struct iphdr *ip;
	struct igmphdr *igmp;
	int ipdatalen, iphdrlen;
	struct mcft_entry *mymcp;
	
	if (recvlen < sizeof(struct iphdr)) 
	{
		log(LOG_WARNING, 0, "received packet too short (%u bytes) for IP header", recvlen);
		return 0;
	}
	
	ip  = (struct iphdr *)recv_buf;
	src = ip->saddr;
	dst = ip->daddr;
	
	if(!IN_MULTICAST(dst))	/* It isn't a multicast */
		return -1; 
	if(chk_local(src)) 	/* It's our report looped back */
		return -1;
	if(dst == ALL_PRINTER)	/* It's MS-Windows UPNP all printers notify */
		return -1;
		
	//pkt_debug(recv_buf);
	
	iphdrlen  = ip->ihl << 2;
	ipdatalen = ip->tot_len;
	
	igmp    = (struct igmphdr *)(recv_buf + iphdrlen);
	group   = igmp->group;
	
	/* determine message type */
	IGMPV3LOG("\n%s> receive IGMP type [%x] from %s to ", __FUNCTION__, igmp->type, inet_ntoa(ip->saddr));
	IGMPV3LOG("%s\n", inet_ntoa(ip->daddr));
	switch (igmp->type) {
		case IGMP_HOST_MEMBERSHIP_QUERY:
			/* Linux Kernel will process local member query, it wont reach here */
			break;
	
		case IGMP_HOST_MEMBERSHIP_REPORT:
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
		case IGMPV2_HOST_MEMBERSHIP_REPORT:
#else
		case IGMP_HOST_NEW_MEMBERSHIP_REPORT:
#endif
			{
				IGMPV3LOG("%s> REPORT(V1/V2), group:%s\n", __FUNCTION__, inet_ntoa(group) );
				if(!chk_mcft(group)) 
				{
					mymcp = add_mcft(group, src);
					if(!mymcp) return -1;		
					mymcp->igmp_ver = IGMP_VER_2;
					igmp_add_group( group );

					//Group Timer=GMI
					mymcp->timer.lefttime = MEMBER_QUERY_INTERVAL;
					mymcp->timer.retry_left = MEMBER_QUERY_COUNT;
					
					//set the new state
					mymcp->filter_mode = MCAST_EXCLUDE;
					igmp_set_srcfilter( mymcp );
				}
					
				mymcp = get_mcft(group);
				if(mymcp) mymcp->igmp_ver = IGMP_VER_2;
				
				//Report => IS_EX( {} )	
				handle_igmpv3_isex( group,src, 0, NULL );
			}
			break;
 		case IGMP_HOST_V3_MEMBERSHIP_REPORT:
		     {
			struct igmpv3_report *igmpv3;
			struct igmpv3_grec *igmpv3grec;
			unsigned short rec_id;
			
			IGMPV3LOG("%s> REPORT(V3)\n", __FUNCTION__ );
			igmpv3 = (struct igmpv3_report *)igmp;
			//printf( "recv IGMP_HOST_V3_MEMBERSHIP_REPORT\n" );
			//printf( "igmpv3->type:0x%x\n", igmpv3->type );
			//printf( "igmpv3->ngrec:0x%x\n\n", ntohs(igmpv3->ngrec) );
		
			rec_id=0;
			igmpv3grec =  &igmpv3->grec[0];
			while( rec_id < ntohs(igmpv3->ngrec) )
			{
				int srcnum;
				//printf( "igmpv3grec[%d]->grec_type:0x%x\n", rec_id, igmpv3grec->grec_type );
				//printf( "igmpv3grec[%d]->grec_auxwords:0x%x\n", rec_id, igmpv3grec->grec_auxwords );
				//printf( "igmpv3grec[%d]->grec_nsrcs:0x%x\n", rec_id, ntohs(igmpv3grec->grec_nsrcs) );
				//printf( "igmpv3grec[%d]->grec_mca:%s\n", rec_id, inet_ntoa(igmpv3grec->grec_mca) );
			
				group = igmpv3grec->grec_mca;
				srcnum = ntohs(igmpv3grec->grec_nsrcs);
				
				switch( igmpv3grec->grec_type )
				{
				case IGMPV3_MODE_IS_INCLUDE:
					IGMPV3LOG("%s> IS_IN, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(group), srcnum );
					handle_igmpv3_isin( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_MODE_IS_EXCLUDE:
					IGMPV3LOG("%s> IS_EX, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(group), srcnum );
					handle_igmpv3_isex( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_CHANGE_TO_INCLUDE: 
					IGMPV3LOG("%s> TO_IN, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(group), srcnum );
					handle_igmpv3_toin( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_CHANGE_TO_EXCLUDE: 
					IGMPV3LOG("%s> TO_EX, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(group), srcnum );
					handle_igmpv3_toex( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_ALLOW_NEW_SOURCES:
					IGMPV3LOG("%s> ALLOW, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(group), srcnum );
					handle_igmpv3_allow( group,src, srcnum, igmpv3grec->grec_src );
					break;
				case IGMPV3_BLOCK_OLD_SOURCES:
					IGMPV3LOG("%s> BLOCK, group:%s, srcnum:%d\n", __FUNCTION__, inet_ntoa(group), srcnum );
					handle_igmpv3_block( group,src, srcnum, igmpv3grec->grec_src );
					break;
				default:
					IGMPV3LOG("%s> Unknown Group Record Types [%x]\n", __FUNCTION__, igmpv3grec->grec_type );
					break;
				}
			
				rec_id++;
				//printf( "count next: 0x%x %d %d %d %d\n", igmpv3grec, sizeof( struct igmpv3_grec ), igmpv3grec->grec_auxwords, ntohs(igmpv3grec->grec_nsrcs), sizeof( __u32 ) );
				igmpv3grec = (struct igmpv3_grec *)( (char*)igmpv3grec + sizeof( struct igmpv3_grec ) + (igmpv3grec->grec_auxwords+ntohs(igmpv3grec->grec_nsrcs))*sizeof( __u32 ) );
				//printf( "count result: 0x%x\n", igmpv3grec );
			}
			break;
		     }
		case IGMP_HOST_LEAVE_MESSAGE :
			IGMPV3LOG("%s> LEAVE(V2), group:%s\n", __FUNCTION__, inet_ntoa(group) );
			if(chk_mcft(group))
			{
				//Leave => TO_IN( {} )
				handle_igmpv3_toin( group,src, 0, NULL );
			}
			break;
		default:
			IGMPV3LOG("%s> receive IGMP Unknown type [%x]\n", __FUNCTION__, igmp->type );
			break;
    }
    return 0;
}


#endif /*CONFIG_IGMPV3_SUPPORT*/
