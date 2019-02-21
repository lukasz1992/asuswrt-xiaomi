#include "io.h"
extern int GetAddressLptPort(int myPort);

	int LoadIODLL_C(){
		return (LoadIODLL());
	}

	void PortOut_C(short int addr,char data){
	PortOut(addr, data);
	}

	char PortIn_C (short int addr){
	return (PortIn(addr));	
	}

	/*int PortAddress_C (unsigned char LPT_Number){
		return (GetAddressLptPort(LPT_Number));
	}*/

	int IsDriverInstalled_C (){
		return (IsDriverInstalled());
	}
	void UnloadIODLL_C (){

		UnloadIODLL();
	}
