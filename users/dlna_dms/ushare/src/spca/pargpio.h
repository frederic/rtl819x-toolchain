 
/***************************************************************************#
# pargpio: library to use parpaort device with 2 D/A . converter            #
# can read status pin 13 or pin 15                                          #
#.                                                                          #
# 		Copyright (C) 2005 Michel Xhaard                            #
#                                                                           #
# This program is free software; you can redistribute it and/or modify      #
# it under the terms of the GNU General Public License as published by      #
# the Free Software Foundation; either version 2 of the License, or         #
# (at your option) any later version.                                       #
#                                                                           #
# This program is distributed in the hope that it will be useful,           #
# but WITHOUT ANY WARRANTY; without even the implied warranty of            #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
# GNU General Public License for more details.                              #
#                                                                           #
# You should have received a copy of the GNU General Public License         #
# along with this program; if not, write to the Free Software               #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA #
#                                                                           #
#***************************************************************************/
int openclaimParaport(char *dev);
unsigned char read_portstatus15(int fd); //pin15
int closereleaseParaport( int fd);
unsigned char read_portstatus13(int fd); //pin13
int port_setdata2(int fd,unsigned char val2);
int port_setdata1(int fd,unsigned char bitval);
int port_toggleInitbit (int fd); //pin 16
int port_toggleAutoFeedbit (int fd); // pin14
