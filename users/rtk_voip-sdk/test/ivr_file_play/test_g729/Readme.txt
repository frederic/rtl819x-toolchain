檔案: 
	729_raw: 錄有 G.729 的聲音檔
	Makefile: 用來 make test_g729.c --> test_g729.o, 
	          使用時要把 -I../../../include/ 指向 VoIP-ATA/linux-2.4.18/rtk_voip/include
	test_g729.c: 測試程式
	test_g729.o: 測試程式執行檔

測試程式說明：
測試程式中的 rtk_SetIvrPlayG729() 等同於 
VoIP-ATA/AP/rtk_voip/voip_manager/voip_manager.c 中的 rtk_IvrStartPlayG729()
參數：
	nCount: 準備寫入的 frame 個數, (每個frame  10 bytes )
	pData: data 的位置

回傳值：
	真實寫入的 frame 個數 
	若回傳值不等於 nCount, 必須等一段時間 (建議1秒), 再嚐試將未寫入的部分寫入。
	目前 kernel buffer 長度約為 2.56 秒. 

