#ifndef RTK_HW_QOS_H
#define RTK_HW_QOS_H


struct qos_cmd_info_s{
	int action;
	union{
		struct{
			unsigned char port[8];
			unsigned int portmask;
		} port_based_priority, queue_num;
		struct{
			unsigned char vlan[8];
			unsigned int vlanmask;
		}vlan_based_priority;
		struct{
			unsigned char dscp[64];
			unsigned int dscpmask1;
			unsigned int dscpmask2;
		}dscp_based_priority;
		struct{
			unsigned char queue[8][6];
			unsigned int portmask;
			unsigned int queuemask;
		}queue_type;
		struct{
			unsigned char priority[8];
			unsigned int prioritymask;
		}sys_priority;
		struct{
			unsigned char decision[5];
		}pri_decision;
		struct{
			unsigned char remark[8][8];
			unsigned int portmask;
			unsigned int prioritymask;
		}vlan_remark, dscp_remark;
		struct{
			unsigned short apr[8][6];
			unsigned char burst[8][6];
			unsigned char ppr[8][6];
			unsigned int portmask;
			unsigned int queuemask;
		}queue_rate;
	} qos_data;
};

#define PORT_BASED_PRIORITY_ASSIGN 1
#define VLAN_BASED_PRIORITY_ASSIGN 2
#define DSCP_BASED_PRIORITY_ASSIGN 3
#define QUEUE_NUMBER 4
#define QUEUE_TYPE_STRICT 5
#define QUEUE_TYPE_WEIGHTED 6
#define PRIORITY_TO_QID 7
#define PRIORITY_DECISION 8
#define VLAN_REMARK 9
#define DSCP_REMARK 10
#define PORT_BASED_PRIORITY_SHOW 11
#define VLAN_BASED_PRIORITY_SHOW 12
#define DSCP_BASED_PRIORITY_SHOW 13
#define QUEUE_NUMBER_SHOW 14
#define QUEUE_TYPE_STRICT_SHOW 15
#define QUEUE_TYPE_WEIGHTED_SHOW 16
#define PRIORITY_TO_QID_SHOW 17
#define PRIORITY_DECISION_SHOW 18
#define VLAN_REMARK_SHOW 19
#define DSCP_REMARK_SHOW 20
#define QUEUE_RATE 21
#define QUEUE_RATE_SHOW 22
#define FLOW_CONTROL_ENABLE 23
#define FLOW_CONTROL_DISABLE 24
#define FLOW_CONTROL_CONFIGURATION_SHOW 25


int rtk_hw_qos_parse(int _num, char* _param[]);


#endif